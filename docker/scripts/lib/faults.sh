#!/bin/bash
# faults.sh — tc/netem network fault injection

# Inject latency on a node's network.
# Usage: inject_latency <node_index> <delay_ms> [jitter_ms]
inject_latency() {
    local idx="$1"
    local delay="$2"
    local jitter="${3:-0}"
    local cid
    cid=$(node_container "$idx")

    echo "  Injecting ${delay}ms latency (+/-${jitter}ms jitter) on node $idx ($cid)"
    docker exec "$cid" tc qdisc add dev eth0 root netem delay "${delay}ms" "${jitter}ms"
}

# Inject packet loss on a node's network.
# Usage: inject_loss <node_index> <percent>
inject_loss() {
    local idx="$1"
    local pct="$2"
    local cid
    cid=$(node_container "$idx")

    echo "  Injecting ${pct}% packet loss on node $idx ($cid)"
    docker exec "$cid" tc qdisc add dev eth0 root netem loss "${pct}%"
}

# Partition a node (100% packet drop).
# Usage: partition_node <node_index>
partition_node() {
    local idx="$1"
    local cid
    cid=$(node_container "$idx")

    echo "  Partitioning node $idx ($cid) — 100% packet drop"
    docker exec "$cid" tc qdisc add dev eth0 root netem loss 100%
}

# Clear all netem rules from a node.
# Usage: clear_faults <node_index>
clear_faults() {
    local idx="$1"
    local cid
    cid=$(node_container "$idx")

    echo "  Clearing faults on node $idx ($cid)"
    docker exec "$cid" tc qdisc del dev eth0 root 2>/dev/null || true
}

# Clear faults on all nodes.
# Usage: clear_all_faults
clear_all_faults() {
    echo "  Clearing faults on all $NODE_COUNT nodes"
    for i in $(seq 0 $((NODE_COUNT - 1))); do
        clear_faults "$i"
    done
}
