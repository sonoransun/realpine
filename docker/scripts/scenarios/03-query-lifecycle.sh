#!/bin/bash
# 03-query-lifecycle.sh — POST query → GET status → GET results → DELETE

SCRIPT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
source "${SCRIPT_DIR}/lib/cluster.sh"
source "${SCRIPT_DIR}/lib/assertions.sh"

discover_nodes

begin_test "Query lifecycle on node 0"

# POST /query — start a query
start_response=$(curl -sf -X POST \
    -H "Content-Type: application/json" \
    -d '{"queryString":"test-docker-query"}' \
    "http://${NODE_IPS[0]}:8080/query" 2>/dev/null)

query_id=$(echo "$start_response" | jq -r '.queryId' 2>/dev/null)

if [[ -z "$query_id" || "$query_id" == "null" ]]; then
    fail "POST /query returned no queryId (response: $start_response)"
    test_summary
    exit 0
fi

pass "POST /query returned queryId=$query_id"

# GET /query/:id — check status
sleep 1
status_response=$(node_rest 0 "/query/${query_id}" 2>/dev/null)

assert_json_field "$status_response" "queryId" "$query_id" \
    "GET /query/$query_id returns correct queryId"

in_progress=$(echo "$status_response" | jq -r '.inProgress' 2>/dev/null)
# inProgress can be true or false, just check it exists
if [[ "$in_progress" == "true" || "$in_progress" == "false" ]]; then
    pass "GET /query/$query_id has inProgress field"
else
    fail "GET /query/$query_id missing inProgress field"
fi

# GET /query/:id/results — get results
results_response=$(node_rest 0 "/query/${query_id}/results" 2>/dev/null)

results_qid=$(echo "$results_response" | jq -r '.queryId' 2>/dev/null)
assert_eq "$query_id" "$results_qid" \
    "GET /query/$query_id/results returns correct queryId"

# DELETE /query/:id — cancel
cancel_response=$(curl -sf -X DELETE \
    "http://${NODE_IPS[0]}:8080/query/${query_id}" 2>/dev/null)

assert_json_field "$cancel_response" "cancelled" "true" \
    "DELETE /query/$query_id cancels successfully"

test_summary
