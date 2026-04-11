#!/bin/sh
# Smoke test for the dtcpStackTestServer/dtcpStackTestClient pair.
# Both binaries enter `while (true) sleep(3600)` after their setup phase,
# so we can only verify that:
#   1) the server brings up its UDP transport,
#   2) the client opens N transports and issues connection requests, and
#   3) neither process crashes during a brief observation window.
# We then SIGTERM both, falling back to SIGKILL after a 5s grace.
set -u

BIN_DIR="${ALPINE_BIN_DIR:-}"
if [ -z "${BIN_DIR}" ] || [ ! -x "${BIN_DIR}/dtcpStackTestServer" ]; then
    SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
    candidate=$(CDPATH= cd -- "${SCRIPT_DIR}/../.." && pwd)/build-baseline/bin
    if [ -x "${candidate}/dtcpStackTestServer" ]; then
        BIN_DIR="${candidate}"
    fi
fi

if [ ! -x "${BIN_DIR}/dtcpStackTestServer" ] || [ ! -x "${BIN_DIR}/dtcpStackTestClient" ]; then
    echo "run_pair.sh: cannot locate dtcpStackTest binaries (looked in ${BIN_DIR})" >&2
    exit 2
fi

SVR_PORT=$((38500 + ($$ % 1000)))
CLI_START_PORT=$((SVR_PORT + 100))
HOST=127.0.0.1

WORK_DIR=$(mktemp -d "${TMPDIR:-/tmp}/alpine-dtcp.XXXXXX") || exit 2
cd "${WORK_DIR}" || exit 2

SERVER_PID=""
CLIENT_PID=""

kill_pid() {
    pid=$1
    if [ -n "${pid}" ] && kill -0 "${pid}" 2>/dev/null; then
        kill "${pid}" 2>/dev/null
        i=0
        while [ "${i}" -lt 50 ] && kill -0 "${pid}" 2>/dev/null; do
            i=$((i + 1))
            sleep 0.1 2>/dev/null || sleep 1
        done
        kill -9 "${pid}" 2>/dev/null
    fi
}

cleanup() {
    kill_pid "${CLIENT_PID}"
    kill_pid "${SERVER_PID}"
    rm -rf "${WORK_DIR}"
}
trap cleanup EXIT INT TERM

echo "run_pair.sh: cwd=$(pwd) HOST=${HOST} SVR_PORT=${SVR_PORT}" >&2
"${BIN_DIR}/dtcpStackTestServer" \
    --logFile server.log --logLevel Debug \
    --ipaddress "${HOST}" --port "${SVR_PORT}" >server.out 2>&1 &
SERVER_PID=$!
echo "run_pair.sh: server pid=${SERVER_PID}" >&2

# Single 2s sleep, then check the server is alive.
sleep 2
if ! kill -0 "${SERVER_PID}" 2>/dev/null; then
    echo "run_pair.sh: dtcpStackTestServer died during startup" >&2
    echo "--- server.out (last 40 lines) ---" >&2
    tail -40 server.out >&2 2>/dev/null
    echo "--- server.log (last 40 lines) ---" >&2
    tail -40 server.log >&2 2>/dev/null
    exit 3
fi

# Client positional args: <logFile> <debugLevel> <svrIp> <svrPort> <myIp> <startPort> <numCreate>
"${BIN_DIR}/dtcpStackTestClient" \
    client.log 2 \
    "${HOST}" "${SVR_PORT}" "${HOST}" "${CLI_START_PORT}" 2 \
    --logFile client.log --logLevel Error >client.out 2>&1 &
CLIENT_PID=$!

# Observation window: both must remain alive for ~2s after the client
# finishes its setup phase.
i=0
while [ "${i}" -lt 20 ]; do
    if ! kill -0 "${CLIENT_PID}" 2>/dev/null; then
        echo "run_pair.sh: dtcpStackTestClient died during startup" >&2
        cat client.out >&2 2>/dev/null
        exit 4
    fi
    if ! kill -0 "${SERVER_PID}" 2>/dev/null; then
        echo "run_pair.sh: dtcpStackTestServer died after client started" >&2
        cat server.out >&2 2>/dev/null
        exit 5
    fi
    i=$((i + 1))
    sleep 0.1 2>/dev/null || sleep 1
done

exit 0
