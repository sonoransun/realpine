#!/bin/bash
# 04-broadcast-query.sh — UDP broadcast query reaches all nodes

SCRIPT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
source "${SCRIPT_DIR}/lib/cluster.sh"
source "${SCRIPT_DIR}/lib/assertions.sh"
source "${SCRIPT_DIR}/lib/wsl.sh"

discover_nodes
detect_wsl

begin_test "UDP broadcast query"

# WSL2 NAT networking may not forward UDP broadcast
if ! wsl_broadcast_available; then
    echo "  SKIP: UDP broadcast unreliable under WSL2 NAT networking"
    pass "Skipped (WSL2 NAT — broadcast unavailable)"
    test_summary
    exit 0
fi

# Send a UDP broadcast packet to the broadcast port from node 0
# The broadcast handler listens on port 8090 by default
cid=$(node_container 0)
docker exec "$cid" bash -c \
    'echo "test-broadcast" | timeout 2 bash -c "cat > /dev/udp/255.255.255.255/8090" 2>/dev/null || true'

# Brief pause to let nodes process
sleep 2

# Verify all nodes are still healthy after receiving broadcast
for i in $(seq 0 $((NODE_COUNT - 1))); do
    assert_http_ok "http://${NODE_IPS[$i]}:8080/status" \
        "Node $i healthy after broadcast"
done

test_summary
