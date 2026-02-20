#!/bin/bash
# 02-peer-discovery.sh — Verify nodes discover each other via UDP beacon

SCRIPT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
source "${SCRIPT_DIR}/lib/cluster.sh"
source "${SCRIPT_DIR}/lib/assertions.sh"

discover_nodes

begin_test "Peer discovery via UDP beacon"

# Allow time for beacon discovery
DISCOVERY_WAIT="${DISCOVERY_WAIT:-8}"
echo "  Waiting ${DISCOVERY_WAIT}s for beacon discovery..."
sleep "$DISCOVERY_WAIT"

expected_peers=$((NODE_COUNT - 1))

for i in $(seq 0 $((NODE_COUNT - 1))); do
    response=$(node_rest "$i" "/peers" 2>/dev/null)

    if [[ -z "$response" ]]; then
        fail "Node $i: no response from /peers"
        continue
    fi

    peer_count=$(echo "$response" | jq '.peers | length' 2>/dev/null)
    if [[ -z "$peer_count" || "$peer_count" == "null" ]]; then
        fail "Node $i: invalid peers response"
        continue
    fi

    assert_ge "$peer_count" "$expected_peers" \
        "Node $i sees >= $expected_peers peers (actual: $peer_count)"
done

test_summary
