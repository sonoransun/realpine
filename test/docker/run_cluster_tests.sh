#!/usr/bin/env bash
#
# run_cluster_tests.sh — Docker cluster integration test driver for CI.
#
# Brings up a 3-node Alpine P2P cluster via docker compose, exercises the
# REST API of the cluster end-to-end, and asserts that nodes see each
# other. Tears the cluster down on exit (success or failure).
#
# Expected environment:
#   - docker + docker buildx + docker compose v2
#   - jq and curl on the host
#   - Run from repo root (or anywhere — the script resolves its own path)
#
# Exit code: 0 on success, 1 on any assertion failure.

set -e
set -u
set -o pipefail

# ── Paths ──────────────────────────────────────────────────────────────
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"
COMPOSE_FILE_BASE="${REPO_ROOT}/docker/docker-compose.yml"
COMPOSE_FILE_CI="${REPO_ROOT}/test/docker/compose.ci.yml"
IMAGE_TAG="alpine:ci-local"

compose() {
    docker compose \
        -f "${COMPOSE_FILE_BASE}" \
        -f "${COMPOSE_FILE_CI}" \
        "$@"
}

# ── Cleanup trap ───────────────────────────────────────────────────────
cleanup() {
    local exit_code=$?
    echo
    echo "=== Cleanup: tearing down cluster ==="
    compose logs --no-color --tail=50 || true
    compose down -v --remove-orphans || true
    exit "${exit_code}"
}
trap cleanup EXIT

# ── Pre-flight ─────────────────────────────────────────────────────────
command -v docker >/dev/null 2>&1 || { echo "error: docker not found in PATH"; exit 1; }
command -v curl   >/dev/null 2>&1 || { echo "error: curl not found in PATH"; exit 1; }
command -v jq     >/dev/null 2>&1 || { echo "error: jq not found in PATH"; exit 1; }

cd "${REPO_ROOT}"

# ── Step 1: Build image if not cached ──────────────────────────────────
echo "=== Step 1: Ensure docker image ${IMAGE_TAG} exists ==="
if ! docker image inspect "${IMAGE_TAG}" >/dev/null 2>&1; then
    echo "Image not cached — building..."
    docker buildx build --load \
        -f docker/Dockerfile \
        -t "${IMAGE_TAG}" \
        .
else
    echo "Image ${IMAGE_TAG} already present, skipping build"
fi

# ── Step 2: Bring up cluster ───────────────────────────────────────────
echo
echo "=== Step 2: Bring up cluster ==="
compose up -d

# ── Step 3: Wait for /status on all 3 host-published ports ────────────
echo
echo "=== Step 3: Wait for node1/2/3 REST readiness ==="

wait_for_status() {
    local port="$1"
    local max_wait=90
    local elapsed=0
    while [[ ${elapsed} -lt ${max_wait} ]]; do
        if curl -sf --max-time 2 "http://localhost:${port}/status" >/dev/null 2>&1; then
            echo "  node on port ${port} is ready (after ${elapsed}s)"
            return 0
        fi
        sleep 2
        elapsed=$((elapsed + 2))
    done
    echo "  node on port ${port} DID NOT become ready within ${max_wait}s"
    return 1
}

wait_for_status 8081
wait_for_status 8082
wait_for_status 8083

# ── Step 4: POST a query to node1, capture queryId ─────────────────────
echo
echo "=== Step 4: POST /query on node1 ==="
QUERY_PAYLOAD='{"queryString":"test","priority":128}'
QUERY_RESPONSE="$(curl -sf -X POST \
    -H 'Content-Type: application/json' \
    -d "${QUERY_PAYLOAD}" \
    "http://localhost:8081/query")"
echo "query response: ${QUERY_RESPONSE}"

QUERY_ID="$(echo "${QUERY_RESPONSE}" | jq -r '.queryId // empty')"
if [[ -z "${QUERY_ID}" ]]; then
    echo "error: could not extract queryId from response"
    exit 1
fi
echo "queryId = ${QUERY_ID}"

# ── Step 5: Wait for propagation, GET query results, assert totalPeers ─
echo
echo "=== Step 5: Wait 5s then GET /query/${QUERY_ID}/results ==="
sleep 5

RESULTS_RESPONSE="$(curl -sf "http://localhost:8081/query/${QUERY_ID}/results")"
echo "results response: ${RESULTS_RESPONSE}"

TOTAL_PEERS="$(echo "${RESULTS_RESPONSE}" | jq -r '.totalPeers // 0')"
echo "totalPeers = ${TOTAL_PEERS}"

if [[ "${TOTAL_PEERS}" -lt 2 ]]; then
    echo "error: expected totalPeers >= 2 after broadcast, got ${TOTAL_PEERS}"
    exit 1
fi
echo "PASS: query saw at least 2 peers"

# ── Step 6: GET /cluster/status and verify it references node2+node3 ──
echo
echo "=== Step 6: GET /cluster/status — expect node2, node3 references ==="
CLUSTER_RESPONSE="$(curl -sf "http://localhost:8081/cluster/status")"
echo "cluster response: ${CLUSTER_RESPONSE}"

if ! echo "${CLUSTER_RESPONSE}" | grep -q "node2"; then
    echo "error: /cluster/status does not mention node2"
    exit 1
fi
if ! echo "${CLUSTER_RESPONSE}" | grep -q "node3"; then
    echo "error: /cluster/status does not mention node3"
    exit 1
fi
echo "PASS: cluster status references node2 and node3"

# ── Step 7: Stop node3, allow the cluster to detect it ─────────────────
echo
echo "=== Step 7: Stop node3 and wait for cluster to detect ==="
NODE3_CONTAINER="$(compose ps -q node3 || true)"
if [[ -z "${NODE3_CONTAINER}" ]]; then
    # fall back to discovered container_name from docker-compose.yml
    NODE3_CONTAINER="alpine-node3"
fi
echo "stopping container: ${NODE3_CONTAINER}"
docker stop "${NODE3_CONTAINER}"

echo "waiting 60s for cluster to mark node3 degraded/offline..."
sleep 60

# ── Step 8: Verify node3 is reported degraded/offline ─────────────────
echo
echo "=== Step 8: GET /cluster/status — expect node3 degraded/offline ==="
DEGRADED_RESPONSE="$(curl -sf "http://localhost:8081/cluster/status")"
echo "post-stop cluster response: ${DEGRADED_RESPONSE}"

# node3 should still appear in the JSON (it's a known member) but
# should be marked degraded, offline, or otherwise unhealthy.
if echo "${DEGRADED_RESPONSE}" | jq -e '
    .nodes // [] |
    map(select(.nodeId == "node3" or .name == "node3" or .hostname == "node3")) |
    .[0] // {} |
    (.status // .state // "") |
    ascii_downcase |
    test("degrad|offline|unhealth|down|unreach")
' >/dev/null 2>&1; then
    echo "PASS: node3 reported as degraded/offline"
else
    # Fallback: if structured check fails, accept any substring signal
    if echo "${DEGRADED_RESPONSE}" | grep -iE "node3.*(degraded|offline|unhealthy|down|unreachable)" >/dev/null; then
        echo "PASS: node3 appears to be degraded/offline (substring match)"
    else
        echo "error: node3 does not appear as degraded/offline in /cluster/status"
        exit 1
    fi
fi

echo
echo "=== ALL CHECKS PASSED ==="
exit 0
