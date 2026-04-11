#!/bin/sh
# Smoke test for the tcpServer/tcpClient pair.  Both binaries enter an
# unbounded event loop after a successful bind/connect, so we treat the
# pair as healthy if:
#   1) the server is still alive after a brief startup grace period, and
#   2) the client is still alive after connecting and issuing its transports.
# We then SIGTERM both, fall back to SIGKILL after 5s.
set -u

BIN_DIR="${ALPINE_BIN_DIR:-}"
if [ -z "${BIN_DIR}" ] || [ ! -x "${BIN_DIR}/tcpServer" ]; then
    SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
    candidate=$(CDPATH= cd -- "${SCRIPT_DIR}/../.." && pwd)/build-baseline/bin
    if [ -x "${candidate}/tcpServer" ]; then
        BIN_DIR="${candidate}"
    fi
fi

if [ ! -x "${BIN_DIR}/tcpServer" ] || [ ! -x "${BIN_DIR}/tcpClient" ]; then
    echo "run_pair.sh: cannot locate tcpServer/tcpClient (looked in ${BIN_DIR})" >&2
    exit 2
fi

PORT=$((38500 + ($$ % 1000)))
HOST=127.0.0.1

WORK_DIR=$(mktemp -d "${TMPDIR:-/tmp}/alpine-tcpconn.XXXXXX") || exit 2
cd "${WORK_DIR}" || exit 2

# Both binaries call Configuration::initialize("server.cfg"/"client.cfg")
# and treat a missing config file as a fatal error.  Drop empty stubs so
# the config loader succeeds and falls back to command-line args.
: > server.cfg
: > client.cfg

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

echo "run_pair.sh: cwd=$(pwd) HOST=${HOST} PORT=${PORT}" >&2
echo "run_pair.sh: launching ${BIN_DIR}/tcpServer" >&2
"${BIN_DIR}/tcpServer" \
    --logFile server.log --logLevel Debug \
    --ipAddress "${HOST}" --port "${PORT}" >server.out 2>&1 &
SERVER_PID=$!

# Wait up to 2s for the server to bind.
i=0
while [ "${i}" -lt 20 ]; do
    if ! kill -0 "${SERVER_PID}" 2>/dev/null; then
        echo "run_pair.sh: tcpServer died during startup" >&2
        echo "--- server.out ---" >&2
        cat server.out >&2 2>/dev/null
        echo "--- server.log ---" >&2
        cat server.log >&2 2>/dev/null
        exit 3
    fi
    i=$((i + 1))
    sleep 0.1 2>/dev/null || sleep 1
done

"${BIN_DIR}/tcpClient" \
    --logFile client.log --logLevel Error \
    --ipAddress "${HOST}" --port "${PORT}" \
    --transports 2 --method Async >client.out 2>&1 &
CLIENT_PID=$!

# Give the client 2s to connect.  If either side is dead by then, fail.
i=0
while [ "${i}" -lt 20 ]; do
    if ! kill -0 "${CLIENT_PID}" 2>/dev/null; then
        echo "run_pair.sh: tcpClient died during startup" >&2
        cat client.out >&2 2>/dev/null
        exit 4
    fi
    if ! kill -0 "${SERVER_PID}" 2>/dev/null; then
        echo "run_pair.sh: tcpServer died after client started" >&2
        cat server.out >&2 2>/dev/null
        exit 5
    fi
    i=$((i + 1))
    sleep 0.1 2>/dev/null || sleep 1
done

# Both sides are healthy after 4s — declare success and tear down.
exit 0
