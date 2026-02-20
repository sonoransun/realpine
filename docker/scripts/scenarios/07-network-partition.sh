#!/bin/bash
# 07-network-partition.sh — Full partition → isolation → heal → recovery

SCRIPT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
source "${SCRIPT_DIR}/lib/cluster.sh"
source "${SCRIPT_DIR}/lib/assertions.sh"
source "${SCRIPT_DIR}/lib/faults.sh"

discover_nodes

begin_test "Network partition and recovery"

# Partition node 0
partition_node 0

# Verify node 0 is unreachable from host
sleep 2
unreachable_response=$(curl -sf --connect-timeout 3 --max-time 5 \
    "http://${NODE_IPS[0]}:8080/status" 2>/dev/null || true)

if [[ -z "$unreachable_response" ]]; then
    pass "Partitioned node 0 is unreachable (expected)"
else
    fail "Partitioned node 0 is still reachable (unexpected)"
fi

# Verify remaining nodes are healthy
for i in $(seq 1 $((NODE_COUNT - 1))); do
    assert_http_ok "http://${NODE_IPS[$i]}:8080/status" \
        "Node $i healthy during partition"
done

# Heal the partition
echo "  Healing partition on node 0..."
clear_faults 0

# Wait for recovery
RECOVERY_TIMEOUT=20
echo "  Waiting up to ${RECOVERY_TIMEOUT}s for recovery..."

recovered=false
for attempt in $(seq 1 $((RECOVERY_TIMEOUT / 2))); do
    response=$(node_rest 0 "/status" --connect-timeout 2 --max-time 3 2>/dev/null)
    if [[ -n "$response" ]]; then
        status=$(echo "$response" | jq -r '.status' 2>/dev/null)
        if [[ "$status" == "running" ]]; then
            recovered=true
            pass "Node 0 recovered after partition heal (attempt $attempt)"
            break
        fi
    fi
    sleep 2
done

if [[ "$recovered" != "true" ]]; then
    fail "Node 0 did not recover within ${RECOVERY_TIMEOUT}s"
fi

# Allow time for re-discovery
sleep 5

# Check if node 0 can re-discover peers
peers_response=$(node_rest 0 "/peers" --connect-timeout 3 2>/dev/null)
if [[ -n "$peers_response" ]]; then
    peer_count=$(echo "$peers_response" | jq '.peers | length' 2>/dev/null)
    if [[ -n "$peer_count" && "$peer_count" -gt 0 ]]; then
        pass "Node 0 re-discovered $peer_count peers after partition heal"
    else
        fail "Node 0 has no peers after partition heal"
    fi
else
    fail "Node 0 /peers endpoint not responding after heal"
fi

test_summary
