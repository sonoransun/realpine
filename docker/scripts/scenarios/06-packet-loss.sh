#!/bin/bash
# 06-packet-loss.sh — 30% packet loss, verify resilience with retries

SCRIPT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
source "${SCRIPT_DIR}/lib/cluster.sh"
source "${SCRIPT_DIR}/lib/assertions.sh"
source "${SCRIPT_DIR}/lib/faults.sh"

discover_nodes

begin_test "Packet loss resilience (30% loss on node 0)"

# Inject 30% packet loss on node 0
inject_loss 0 30

# Try to reach node 0 with retries — should succeed within 5 attempts
reached=false
for attempt in $(seq 1 5); do
    response=$(node_rest 0 "/status" --connect-timeout 3 --max-time 5 2>/dev/null)
    if [[ -n "$response" ]]; then
        status=$(echo "$response" | jq -r '.status' 2>/dev/null)
        if [[ "$status" == "running" ]]; then
            reached=true
            pass "Node 0 reachable on attempt $attempt despite 30% loss"
            break
        fi
    fi
    sleep 1
done

if [[ "$reached" != "true" ]]; then
    fail "Node 0 unreachable after 5 attempts with 30% loss"
fi

# Verify other nodes are unaffected
for i in $(seq 1 $((NODE_COUNT - 1))); do
    assert_http_ok "http://${NODE_IPS[$i]}:8080/status" \
        "Node $i unaffected by node 0 packet loss"
done

# Clean up
clear_faults 0

test_summary
