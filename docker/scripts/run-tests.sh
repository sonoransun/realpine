#!/bin/bash
# run-tests.sh — Main test orchestrator
#
# Usage:
#   docker/scripts/run-tests.sh              # 3 nodes, all tests
#   NODES=7 docker/scripts/run-tests.sh      # 7 nodes, all tests
#   docker/scripts/run-tests.sh 01 03        # run only scenarios 01 and 03

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
export COMPOSE_FILE="${PROJECT_ROOT}/docker/docker-compose.yml"
export NODES="${NODES:-3}"

# Source library functions
source "${SCRIPT_DIR}/lib/cluster.sh"
source "${SCRIPT_DIR}/lib/assertions.sh"
source "${SCRIPT_DIR}/lib/faults.sh"
source "${SCRIPT_DIR}/lib/wsl.sh"

# Cleanup on exit
cleanup() {
    echo ""
    echo "Tearing down cluster..."
    docker compose -f "${COMPOSE_FILE}" down --volumes --remove-orphans 2>/dev/null || true
}
trap cleanup EXIT

# Check prerequisites
for cmd in docker jq curl; do
    if ! command -v "$cmd" &>/dev/null; then
        echo "ERROR: '$cmd' is required but not found"
        exit 1
    fi
done

# Detect WSL2 and apply workarounds
detect_wsl
apply_wsl_workarounds

echo "========================================"
echo "  Alpine Docker Test Suite"
echo "  Nodes: $NODES"
if [[ "$WSL_DETECTED" == "true" ]]; then
echo "  Platform: WSL2"
fi
echo "========================================"

# Build the image
echo ""
echo "--- Building Docker image ---"
docker compose -f "${COMPOSE_FILE}" build

# Start the cluster
echo ""
echo "--- Starting ${NODES}-node cluster ---"
NODES="${NODES}" docker compose -f "${COMPOSE_FILE}" up -d --scale "alpine-node=${NODES}"

# Discover nodes
echo ""
echo "--- Discovering nodes ---"
discover_nodes "$NODES"

# Wait for health
echo ""
echo "--- Waiting for cluster health ---"
wait_for_cluster "$NODES" "${HEALTH_TIMEOUT:-60}"

# Collect scenarios to run
SCENARIO_DIR="${SCRIPT_DIR}/scenarios"
SCENARIOS=()

if [[ $# -gt 0 ]]; then
    # Run specific scenarios by number prefix
    for num in "$@"; do
        match=$(ls "${SCENARIO_DIR}"/${num}-*.sh 2>/dev/null | head -1)
        if [[ -n "$match" ]]; then
            SCENARIOS+=("$match")
        else
            echo "WARNING: No scenario matching '${num}' found"
        fi
    done
else
    # Run all scenarios in order
    for f in "${SCENARIO_DIR}"/*.sh; do
        [[ -f "$f" ]] && SCENARIOS+=("$f")
    done
fi

# Run scenarios
TOTAL=0
PASSED=0
FAILED=0
FAILED_NAMES=()

for scenario in "${SCENARIOS[@]}"; do
    name=$(basename "$scenario" .sh)
    ((TOTAL++))

    echo ""
    echo "========================================"
    echo "  Scenario: $name"
    echo "========================================"

    # Reset per-scenario counters
    PASS_COUNT=0
    FAIL_COUNT=0

    if bash "$scenario"; then
        if [[ "$FAIL_COUNT" -eq 0 ]]; then
            echo "  >>> SCENARIO PASSED: $name"
            ((PASSED++))
        else
            echo "  >>> SCENARIO FAILED: $name ($FAIL_COUNT assertion failures)"
            ((FAILED++))
            FAILED_NAMES+=("$name")
        fi
    else
        echo "  >>> SCENARIO ERROR: $name (script exited with non-zero)"
        ((FAILED++))
        FAILED_NAMES+=("$name")
    fi
done

# Final summary
echo ""
echo "========================================"
echo "  TEST SUITE SUMMARY"
echo "  Total:  $TOTAL"
echo "  Passed: $PASSED"
echo "  Failed: $FAILED"
if [[ ${#FAILED_NAMES[@]} -gt 0 ]]; then
    echo "  Failed scenarios:"
    for name in "${FAILED_NAMES[@]}"; do
        echo "    - $name"
    done
fi
echo "========================================"

[[ "$FAILED" -eq 0 ]]
