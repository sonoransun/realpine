#!/bin/bash
# 01-health-check.sh — Verify all N nodes respond to GET /status

SCRIPT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
source "${SCRIPT_DIR}/lib/cluster.sh"
source "${SCRIPT_DIR}/lib/assertions.sh"

discover_nodes

begin_test "All nodes respond to GET /status"

for i in $(seq 0 $((NODE_COUNT - 1))); do
    response=$(node_rest "$i" "/status" 2>/dev/null)

    assert_json_field "$response" "status" "running" \
        "Node $i status == running"

    assert_json_field "$response" "version" "devel-00019" \
        "Node $i version == devel-00019"
done

test_summary
