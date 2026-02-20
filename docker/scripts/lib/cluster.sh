#!/bin/bash
# cluster.sh — Node discovery and health-check helpers

# Populated by discover_nodes()
NODE_IPS=()
NODE_CONTAINERS=()
NODE_COUNT=0

# Discover all alpine-node containers and their IPs.
# Usage: discover_nodes [expected_count]
discover_nodes() {
    local expected="${1:-0}"
    local project
    project=$(docker compose -f "${COMPOSE_FILE}" ps --format json 2>/dev/null \
        | jq -r '.Project // .Name' | head -1 | sed 's/-.*//')

    NODE_IPS=()
    NODE_CONTAINERS=()

    local containers
    containers=$(docker compose -f "${COMPOSE_FILE}" ps -q 2>/dev/null)

    if [[ -z "$containers" ]]; then
        echo "ERROR: No containers found"
        return 1
    fi

    while IFS= read -r cid; do
        [[ -z "$cid" ]] && continue
        local ip
        ip=$(docker inspect -f '{{range .NetworkSettings.Networks}}{{.IPAddress}}{{end}}' "$cid" 2>/dev/null)
        if [[ -n "$ip" ]]; then
            NODE_CONTAINERS+=("$cid")
            NODE_IPS+=("$ip")
        fi
    done <<< "$containers"

    NODE_COUNT=${#NODE_IPS[@]}

    if [[ "$expected" -gt 0 && "$NODE_COUNT" -ne "$expected" ]]; then
        echo "WARNING: Expected $expected nodes but found $NODE_COUNT"
        return 1
    fi

    echo "Discovered $NODE_COUNT nodes:"
    for i in "${!NODE_IPS[@]}"; do
        echo "  [$i] ${NODE_CONTAINERS[$i]:0:12} → ${NODE_IPS[$i]}"
    done
    return 0
}

# Wait for all nodes to respond to GET /status.
# Usage: wait_for_cluster [count] [timeout_sec]
wait_for_cluster() {
    local count="${1:-$NODE_COUNT}"
    local timeout="${2:-60}"
    local start=$SECONDS

    echo "Waiting for $count nodes to become healthy (timeout: ${timeout}s)..."

    while true; do
        local healthy=0
        for i in $(seq 0 $((count - 1))); do
            local ip="${NODE_IPS[$i]}"
            if curl -sf --connect-timeout 2 "http://${ip}:8080/status" >/dev/null 2>&1; then
                ((healthy++))
            fi
        done

        if [[ "$healthy" -ge "$count" ]]; then
            echo "All $count nodes healthy (${healthy}/${count}) in $((SECONDS - start))s"
            return 0
        fi

        if [[ $((SECONDS - start)) -ge "$timeout" ]]; then
            echo "TIMEOUT: Only ${healthy}/${count} nodes healthy after ${timeout}s"
            return 1
        fi

        sleep 2
    done
}

# Curl a node's REST API.
# Usage: node_rest <index> <path> [extra_curl_args...]
node_rest() {
    local idx="$1"
    local path="$2"
    shift 2
    curl -sf "http://${NODE_IPS[$idx]}:8080${path}" "$@"
}

# Return the container ID for a node index.
# Usage: node_container <index>
node_container() {
    echo "${NODE_CONTAINERS[$1]}"
}

# Re-discover nodes (after scale events).
# Usage: refresh_nodes [expected_count]
refresh_nodes() {
    discover_nodes "${1:-0}"
}
