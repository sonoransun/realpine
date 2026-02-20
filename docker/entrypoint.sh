#!/bin/bash
set -e

# Auto-detect container IP address
export IP_ADDRESS="${IP_ADDRESS:-$(hostname -I | awk '{print $1}')}"

# Set defaults for required and optional env vars
export PORT="${PORT:-9000}"
export REST_PORT="${REST_PORT:-8080}"
export REST_BIND_ADDRESS="${REST_BIND_ADDRESS:-0}"
export BEACON_PORT="${BEACON_PORT:-8089}"
export BEACON_ENABLED="${BEACON_ENABLED:-true}"
export BROADCAST_ENABLED="${BROADCAST_ENABLED:-true}"
export BROADCAST_PORT="${BROADCAST_PORT:-8090}"
export TOR_ENABLED="${TOR_ENABLED:-false}"
export DLNA_ENABLED="${DLNA_ENABLED:-false}"
export LOG_LEVEL="${LOG_LEVEL:-Info}"

echo "========================================"
echo "  Alpine P2P Node"
echo "========================================"
echo "  IP_ADDRESS:      ${IP_ADDRESS}"
echo "  PORT:            ${PORT}"
echo "  REST_PORT:       ${REST_PORT}"
echo "  BEACON_PORT:     ${BEACON_PORT}"
echo "  BROADCAST_PORT:  ${BROADCAST_PORT}"
echo "  BEACON_ENABLED:  ${BEACON_ENABLED}"
echo "  TOR_ENABLED:     ${TOR_ENABLED}"
echo "  DLNA_ENABLED:    ${DLNA_ENABLED}"
echo "  LOG_LEVEL:       ${LOG_LEVEL}"
echo "========================================"

exec AlpineRestBridge --logLevel "${LOG_LEVEL}" "$@"
