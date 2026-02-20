#!/bin/bash
# wsl.sh — WSL2 detection and networking workarounds

WSL_DETECTED=false

# Detect if running under WSL2
detect_wsl() {
    if [[ -f /proc/version ]]; then
        if grep -qi microsoft /proc/version 2>/dev/null; then
            WSL_DETECTED=true
            echo "WSL2 detected — applying networking workarounds"
            return 0
        fi
    fi

    if [[ -n "$WSL_DISTRO_NAME" ]]; then
        WSL_DETECTED=true
        echo "WSL2 detected (via WSL_DISTRO_NAME) — applying networking workarounds"
        return 0
    fi

    return 1
}

# WSL2 uses a NAT-based virtual network. Docker containers run behind
# an additional NAT layer. UDP broadcast does not cross these boundaries
# without explicit configuration.
#
# Workarounds applied:
#   1. Use unicast peer seeding instead of relying on broadcast discovery
#   2. Increase discovery timeouts (WSL2 networking adds latency)
#   3. Skip broadcast-dependent tests when broadcast is unreliable

# Configure environment for WSL2 Docker networking
apply_wsl_workarounds() {
    if [[ "$WSL_DETECTED" != "true" ]]; then
        return 0
    fi

    # WSL2 adds networking latency — increase timeouts
    export DISCOVERY_WAIT="${DISCOVERY_WAIT:-15}"
    export HEALTH_TIMEOUT="${HEALTH_TIMEOUT:-90}"

    # Docker Desktop on WSL2 may need the docker socket path
    if [[ -z "$DOCKER_HOST" ]] && [[ -S /var/run/docker.sock ]]; then
        export DOCKER_HOST="unix:///var/run/docker.sock"
    fi

    echo "  WSL2 workarounds applied:"
    echo "    DISCOVERY_WAIT=${DISCOVERY_WAIT}s (extended)"
    echo "    HEALTH_TIMEOUT=${HEALTH_TIMEOUT}s (extended)"
}

# Check if UDP broadcast is likely to work
wsl_broadcast_available() {
    if [[ "$WSL_DETECTED" == "true" ]]; then
        # WSL2 mirrored networking (Windows 11 23H2+) supports broadcast
        # WSL2 NAT mode (default) does NOT
        if [[ -f /proc/sys/net/ipv4/conf/eth0/bc_forwarding ]]; then
            local bc_fwd
            bc_fwd=$(cat /proc/sys/net/ipv4/conf/eth0/bc_forwarding 2>/dev/null)
            if [[ "$bc_fwd" == "1" ]]; then
                return 0
            fi
        fi
        echo "  WARNING: UDP broadcast may not work under WSL2 NAT networking"
        return 1
    fi
    return 0
}

# Get the host IP as seen from WSL2 containers
wsl_host_ip() {
    if [[ "$WSL_DETECTED" == "true" ]]; then
        # In WSL2, the Windows host IP is typically the default gateway
        ip route show default 2>/dev/null | awk '{print $3}' | head -1
    else
        echo "127.0.0.1"
    fi
}
