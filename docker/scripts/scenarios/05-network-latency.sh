#!/bin/bash
# 05-network-latency.sh — Inject 200ms latency, verify degraded but operational

SCRIPT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
source "${SCRIPT_DIR}/lib/cluster.sh"
source "${SCRIPT_DIR}/lib/assertions.sh"
source "${SCRIPT_DIR}/lib/faults.sh"

discover_nodes

begin_test "Network latency injection (200ms + 50ms jitter)"

# Inject latency on node 0
inject_latency 0 200 50

# Measure response time from the affected node
start_time=$(date +%s%N)
response=$(node_rest 0 "/status" --connect-timeout 10 --max-time 10 2>/dev/null)
end_time=$(date +%s%N)

elapsed_ms=$(( (end_time - start_time) / 1000000 ))
echo "  Response from node 0 took ${elapsed_ms}ms"

assert_ge "$elapsed_ms" 150 \
    "Response time >= 150ms with 200ms injected latency (actual: ${elapsed_ms}ms)"

assert_json_field "$response" "status" "running" \
    "Node 0 still running under latency"

# Verify other nodes are unaffected
for i in $(seq 1 $((NODE_COUNT - 1))); do
    start_time=$(date +%s%N)
    node_rest "$i" "/status" >/dev/null 2>&1
    end_time=$(date +%s%N)
    other_ms=$(( (end_time - start_time) / 1000000 ))

    if [[ "$other_ms" -lt 150 ]]; then
        pass "Node $i unaffected (${other_ms}ms)"
    else
        fail "Node $i unexpectedly slow (${other_ms}ms)"
    fi
done

# Clean up
clear_faults 0

test_summary
