#!/bin/bash
# 09-scale-up.sh — Add 2 nodes dynamically, verify mesh expansion

SCRIPT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
source "${SCRIPT_DIR}/lib/cluster.sh"
source "${SCRIPT_DIR}/lib/assertions.sh"

discover_nodes
original_count=$NODE_COUNT
new_count=$((original_count + 2))

begin_test "Scale up from $original_count to $new_count nodes"

echo "  Scaling from $original_count to $new_count nodes..."
NODES="$new_count" docker compose -f "${COMPOSE_FILE}" up -d \
    --scale "alpine-node=${new_count}" --no-recreate 2>/dev/null

# Wait for new containers to appear
sleep 5

# Re-discover nodes
discover_nodes "$new_count"
assert_eq "$new_count" "$NODE_COUNT" \
    "Discovered $new_count nodes after scale-up"

# Wait for all nodes to be healthy
wait_for_cluster "$new_count" 30

# Verify all nodes respond
for i in $(seq 0 $((NODE_COUNT - 1))); do
    assert_http_ok "http://${NODE_IPS[$i]}:8080/status" \
        "Node $i healthy after scale-up"
done

# Allow time for peer discovery
sleep 8

# Check peer counts
expected_peers=$((new_count - 1))
for i in $(seq 0 $((NODE_COUNT - 1))); do
    response=$(node_rest "$i" "/peers" 2>/dev/null)
    if [[ -n "$response" ]]; then
        peer_count=$(echo "$response" | jq '.peers | length' 2>/dev/null)
        if [[ -n "$peer_count" && "$peer_count" != "null" ]]; then
            echo "  Node $i sees $peer_count peers"
        fi
    fi
done

# At minimum, each node should see at least 1 peer
for i in $(seq 0 $((NODE_COUNT - 1))); do
    response=$(node_rest "$i" "/peers" 2>/dev/null)
    if [[ -n "$response" ]]; then
        peer_count=$(echo "$response" | jq '.peers | length' 2>/dev/null)
        assert_ge "${peer_count:-0}" 1 \
            "Node $i sees >= 1 peer after scale-up (actual: ${peer_count:-0})"
    else
        fail "Node $i /peers not responding"
    fi
done

test_summary
