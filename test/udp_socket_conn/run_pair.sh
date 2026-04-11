#!/bin/sh
# Wrapper that exercises the udpSockServer/udpSockClient pair on loopback.
# The server listens on a port chosen from the current PID; the client sends
# a few packets and a final zero-length packet, which is the documented
# back-door shutdown signal that lets the server exit cleanly.
#
# Exits with the client's exit code (or non-zero if anything went wrong).
set -u

BIN_DIR="${ALPINE_BIN_DIR:-${CTEST_RESOURCE_DIR:-}}"
if [ -z "${BIN_DIR}" ] || [ ! -x "${BIN_DIR}/udpSockServer" ]; then
    SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
    BIN_DIR="${SCRIPT_DIR}/../../bin"
fi
if [ ! -x "${BIN_DIR}/udpSockServer" ]; then
    # Fall back to a search relative to the script: build-baseline/bin
    candidate=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)/build-baseline/bin
    if [ -x "${candidate}/udpSockServer" ]; then
        BIN_DIR="${candidate}"
    fi
fi

if [ ! -x "${BIN_DIR}/udpSockServer" ] || [ ! -x "${BIN_DIR}/udpSockClient" ]; then
    echo "run_pair.sh: cannot locate udpSockServer/udpSockClient (looked in ${BIN_DIR})" >&2
    exit 2
fi

# Stable but per-run port; range 38500..39499.
PORT=$((38500 + ($$ % 1000)))
HOST=127.0.0.1

# A scratch dir for the .log files the binaries hard-code as Server.log /
# Client.log in the current working directory.
WORK_DIR=$(mktemp -d "${TMPDIR:-/tmp}/alpine-udpsock.XXXXXX") || exit 2
cd "${WORK_DIR}" || exit 2

cleanup() {
    if [ -n "${SERVER_PID:-}" ] && kill -0 "${SERVER_PID}" 2>/dev/null; then
        kill "${SERVER_PID}" 2>/dev/null
        # 5s grace, then SIGKILL.
        i=0
        while [ "${i}" -lt 50 ] && kill -0 "${SERVER_PID}" 2>/dev/null; do
            i=$((i + 1))
            sleep 0.1 2>/dev/null || sleep 1
        done
        kill -9 "${SERVER_PID}" 2>/dev/null
    fi
    rm -rf "${WORK_DIR}"
}
trap cleanup EXIT INT TERM

"${BIN_DIR}/udpSockServer" 2 "${HOST}" "${PORT}" >server.out 2>&1 &
SERVER_PID=$!

# Give the server a moment to bind.
sleep 1

if ! kill -0 "${SERVER_PID}" 2>/dev/null; then
    echo "run_pair.sh: server failed to start" >&2
    cat server.out >&2 2>/dev/null
    exit 3
fi

# Client: <debugLevel> <localIp> <localPort> <destIp> <destPort> <packetSize> <packetCount>
"${BIN_DIR}/udpSockClient" 2 "${HOST}" 0 "${HOST}" "${PORT}" 64 8 >client.out 2>&1
CLIENT_RC=$?

# The client sends a final 0-byte packet which causes the server's recv loop
# to exit.  Wait briefly for the server to drain and exit on its own.
i=0
while [ "${i}" -lt 30 ] && kill -0 "${SERVER_PID}" 2>/dev/null; do
    i=$((i + 1))
    sleep 0.1 2>/dev/null || sleep 1
done

exit "${CLIENT_RC}"
