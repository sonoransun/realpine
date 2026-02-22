#!/bin/bash
#######
##
##  Master test runner for Alpine 802.11 integration tests.
##
##  Requires:
##    - Linux with mac80211_hwsim kernel module available
##    - Root privileges (for raw sockets, module loading, namespaces)
##    - iw, ip utilities
##    - Built test binaries in the build directory
##
##  Usage:
##    sudo ./run_wifi_tests.sh [build_dir]
##
#######

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="${1:-$(cd "$SCRIPT_DIR/../../build" && pwd)}"
BIN_DIR="$BUILD_DIR/bin"
LOG_DIR="/tmp/alpine_wifi_tests"

PASS=0
FAIL=0
SKIP=0

# Colors for terminal output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
NC='\033[0m'  # No Color

log_pass() { echo -e "${GREEN}[PASS]${NC} $1"; PASS=$((PASS + 1)); }
log_fail() { echo -e "${RED}[FAIL]${NC} $1"; FAIL=$((FAIL + 1)); }
log_skip() { echo -e "${YELLOW}[SKIP]${NC} $1"; SKIP=$((SKIP + 1)); }
log_info() { echo -e "       $1"; }

# -------------------------------------------------------------------
# Preflight checks
# -------------------------------------------------------------------
if [ "$(id -u)" -ne 0 ]; then
    echo "ERROR: Must run as root." >&2
    exit 1
fi

if [ "$(uname -s)" != "Linux" ]; then
    echo "ERROR: These tests require Linux with mac80211_hwsim." >&2
    exit 1
fi

if ! modinfo mac80211_hwsim >/dev/null 2>&1; then
    echo "ERROR: mac80211_hwsim kernel module not available." >&2
    echo "       Install with: sudo apt install linux-modules-extra-\$(uname -r)" >&2
    exit 1
fi

mkdir -p "$LOG_DIR"

# -------------------------------------------------------------------
# Load environment helpers and set up hwsim
# -------------------------------------------------------------------
source "$SCRIPT_DIR/hwsim_env.sh"

cleanup() {
    hwsim_teardown 2>/dev/null || true
}
trap cleanup EXIT

hwsim_setup

echo ""
echo "============================================="
echo "  Alpine 802.11 Integration Tests"
echo "============================================="
echo ""

# -------------------------------------------------------------------
# Test 1: Raw 802.11 frame exchange via RawWifiConnection
# -------------------------------------------------------------------
TEST_NAME="RawWifiConnection frame exchange"
TEST_BIN="$BIN_DIR/wifiRawConnTest"
TEST_LOG="$LOG_DIR/raw_conn_test.log"

if [ -x "$TEST_BIN" ]; then
    log_info "Running: $TEST_NAME"
    log_info "  Interfaces: $HWSIM_MON0 (sender) -> $HWSIM_MON1 (receiver)"

    if "$TEST_BIN" "$HWSIM_MON0" "$HWSIM_MON1" > "$TEST_LOG" 2>&1; then
        log_pass "$TEST_NAME"
    else
        log_fail "$TEST_NAME (see $TEST_LOG)"
    fi
else
    log_skip "$TEST_NAME (binary not built)"
fi

# -------------------------------------------------------------------
# Test 2: Broadcast UDP send/receive via BroadcastUdpConnection
# -------------------------------------------------------------------
TEST_NAME="BroadcastUdpConnection send/receive"
TEST_BIN="$BIN_DIR/wifiBroadcastConnTest"
TEST_LOG="$LOG_DIR/broadcast_conn_test.log"

if [ -x "$TEST_BIN" ]; then
    log_info "Running: $TEST_NAME"
    log_info "  NS0: $HWSIM_NS0 ($HWSIM_IP0) -> NS1: $HWSIM_NS1 ($HWSIM_IP1)"

    # Start receiver in NS1 in background
    ip netns exec "$HWSIM_NS1" "$TEST_BIN" recv "$HWSIM_IP1" 44100 > "$LOG_DIR/bcast_recv.log" 2>&1 &
    RECV_PID=$!
    sleep 1  # let receiver bind

    # Run sender in NS0
    SEND_LOG="$LOG_DIR/bcast_send.log"
    if ip netns exec "$HWSIM_NS0" "$TEST_BIN" send "$HWSIM_IP0" 44100 "$HWSIM_BCAST" > "$SEND_LOG" 2>&1; then
        # Wait for receiver to finish (with timeout)
        local_timeout=5
        while kill -0 "$RECV_PID" 2>/dev/null && [ "$local_timeout" -gt 0 ]; do
            sleep 1
            local_timeout=$((local_timeout - 1))
        done
        kill "$RECV_PID" 2>/dev/null || true
        wait "$RECV_PID" 2>/dev/null || true

        # Check if receiver got data
        if grep -q "BROADCAST_RECV_OK" "$LOG_DIR/bcast_recv.log"; then
            log_pass "$TEST_NAME"
        else
            log_fail "$TEST_NAME (receiver did not get data, see $LOG_DIR/bcast_recv.log)"
        fi
    else
        kill "$RECV_PID" 2>/dev/null || true
        wait "$RECV_PID" 2>/dev/null || true
        log_fail "$TEST_NAME (sender failed, see $SEND_LOG)"
    fi
else
    log_skip "$TEST_NAME (binary not built)"
fi

# -------------------------------------------------------------------
# Test 3: WiFi beacon injection and discovery
# -------------------------------------------------------------------
TEST_NAME="WifiBeaconInjector peer discovery"
TEST_BIN="$BIN_DIR/wifiBeaconDiscoveryTest"
TEST_LOG="$LOG_DIR/beacon_discovery_test.log"

if [ -x "$TEST_BIN" ]; then
    log_info "Running: $TEST_NAME"
    log_info "  Interfaces: $HWSIM_MON0 (injector) <-> $HWSIM_MON1 (listener)"

    if "$TEST_BIN" "$HWSIM_MON0" "$HWSIM_MON1" > "$TEST_LOG" 2>&1; then
        log_pass "$TEST_NAME"
    else
        log_fail "$TEST_NAME (see $TEST_LOG)"
    fi
else
    log_skip "$TEST_NAME (binary not built)"
fi

# -------------------------------------------------------------------
# Results
# -------------------------------------------------------------------
echo ""
echo "============================================="
echo "  Results: ${GREEN}$PASS passed${NC}, ${RED}$FAIL failed${NC}, ${YELLOW}$SKIP skipped${NC}"
echo "  Logs in: $LOG_DIR"
echo "============================================="
echo ""

[ "$FAIL" -eq 0 ]
