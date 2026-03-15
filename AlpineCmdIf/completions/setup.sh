#!/usr/bin/env sh
# Alpine CLI setup — source this file to get the 'alpc' alias and completions
#
# Usage:
#   source /path/to/AlpineCmdIf/completions/setup.sh
#
# Or add to your .bashrc/.zshrc:
#   [ -f /path/to/AlpineCmdIf/completions/setup.sh ] && . /path/to/AlpineCmdIf/completions/setup.sh
#
# Prerequisites:
#   - AlpineCmdIf binary must be in PATH (or set ALPINE_CMD_PATH)

_ALPINE_SETUP_DIR="$(cd "$(dirname "${BASH_SOURCE[0]:-$0}")" && pwd)"

# Core alias
alias alpc='AlpineCmdIf'

# Quick-action aliases (pipe-friendly, JSON output, quiet)
alias alpc-peers='AlpineCmdIf --command getExtendedPeerList --json --quiet'
alias alpc-status='AlpineCmdIf --command getStatus --json --quiet'
alias alpc-query='AlpineCmdIf --command beginQuery --json --quiet'
alias alpc-results='AlpineCmdIf --command getQueryResults --json --quiet'
alias alpc-modules='AlpineCmdIf --command listActiveModules --json --quiet'
alias alpc-groups='AlpineCmdIf --command getUserGroupList --json --quiet'
alias alpc-ping='AlpineCmdIf --command pingDtcpPeer --json --quiet'
alias alpc-interactive='AlpineCmdIf --command interactive'

# Table-formatted aliases for human-readable output
alias alpc-peers-t='AlpineCmdIf --command getExtendedPeerList --format table --quiet'
alias alpc-status-t='AlpineCmdIf --command getStatus --format table --quiet'
alias alpc-modules-t='AlpineCmdIf --command listActiveModules --format table --quiet'
alias alpc-groups-t='AlpineCmdIf --command getUserGroupList --format table --quiet'

# Helper functions for common workflows
alpc-search() {
    # Usage: alpc-search "search terms" [max_hits]
    local query="$1"
    local hits="${2:-50}"
    if [ -z "$query" ]; then
        echo "Usage: alpc-search <query> [max_hits]" >&2
        return 1
    fi
    AlpineCmdIf --command beginQuery --queryString "$query" --numHits "$hits" --json --quiet
}

alpc-peer-add() {
    # Usage: alpc-peer-add <ip> <port>
    local ip="$1"
    local port="$2"
    if [ -z "$ip" ] || [ -z "$port" ]; then
        echo "Usage: alpc-peer-add <ip> <port>" >&2
        return 1
    fi
    AlpineCmdIf --command addDtcpPeer --ipAddress "$ip" --port "$port" --json --quiet
}

alpc-peer-info() {
    # Usage: alpc-peer-info <peer_id>
    local pid="$1"
    if [ -z "$pid" ]; then
        echo "Usage: alpc-peer-info <peer_id>" >&2
        return 1
    fi
    AlpineCmdIf --command getDtcpPeerStatus --peerId "$pid" --format table --quiet
}

alpc-query-watch() {
    # Usage: alpc-query-watch <query_id> [interval_seconds]
    # Polls query status until complete
    local qid="$1"
    local interval="${2:-2}"
    if [ -z "$qid" ]; then
        echo "Usage: alpc-query-watch <query_id> [interval_seconds]" >&2
        return 1
    fi
    while true; do
        local status
        status=$(AlpineCmdIf --command getQueryStatus --queryId "$qid" --json --quiet 2>/dev/null)
        echo "$status"
        if echo "$status" | grep -q '"inProgress":false'; then
            echo "Query $qid complete."
            break
        fi
        sleep "$interval"
    done
}

alpc-block() {
    # Usage: alpc-block <ip>
    local ip="$1"
    if [ -z "$ip" ]; then
        echo "Usage: alpc-block <ip>" >&2
        return 1
    fi
    AlpineCmdIf --command excludeHost --ipAddress "$ip" --json --quiet
}

alpc-unblock() {
    # Usage: alpc-unblock <ip>
    local ip="$1"
    if [ -z "$ip" ]; then
        echo "Usage: alpc-unblock <ip>" >&2
        return 1
    fi
    AlpineCmdIf --command allowHost --ipAddress "$ip" --json --quiet
}

# Load shell-specific completions
if [ -n "$ZSH_VERSION" ]; then
    # Zsh
    if [ -f "$_ALPINE_SETUP_DIR/alpc.zsh" ]; then
        source "$_ALPINE_SETUP_DIR/alpc.zsh"
    fi
elif [ -n "$BASH_VERSION" ]; then
    # Bash
    if [ -f "$_ALPINE_SETUP_DIR/alpc.bash" ]; then
        source "$_ALPINE_SETUP_DIR/alpc.bash"
    fi
fi

unset _ALPINE_SETUP_DIR
