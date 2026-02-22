#!/bin/bash
#######
##
##  mac80211_hwsim environment setup/teardown for Alpine 802.11 tests.
##
##  Creates two virtual 802.11 radios via the mac80211_hwsim kernel module.
##  Configures them for both raw monitor-mode testing and IP-based broadcast testing.
##
##  Usage:
##    source hwsim_env.sh
##    hwsim_setup          # load module, configure interfaces
##    hwsim_teardown       # restore and unload
##
##  After setup, the following variables are exported:
##    HWSIM_MON0   - First monitor-mode interface  (e.g. mon0)
##    HWSIM_MON1   - Second monitor-mode interface  (e.g. mon1)
##    HWSIM_WLAN0  - First managed interface name   (e.g. wlan0)
##    HWSIM_WLAN1  - Second managed interface name  (e.g. wlan1)
##    HWSIM_NS0    - Network namespace for station 0
##    HWSIM_NS1    - Network namespace for station 1
##    HWSIM_IP0    - IP address for station 0       (10.55.0.1)
##    HWSIM_IP1    - IP address for station 1       (10.55.0.2)
##    HWSIM_BCAST  - Broadcast address              (10.55.0.255)
##
#######

set -e

HWSIM_IP0="10.55.0.1"
HWSIM_IP1="10.55.0.2"
HWSIM_BCAST="10.55.0.255"
HWSIM_NS0="alpine_hwsim_ns0"
HWSIM_NS1="alpine_hwsim_ns1"

# Detect the wlan interface names created by mac80211_hwsim.
# They may not be wlan0/wlan1 if other wifi hardware exists.
_detect_hwsim_interfaces() {
    # After loading mac80211_hwsim radios=2, two new phy devices appear.
    # Find the interfaces associated with the two newest phys.
    local phys
    phys=$(ls -1td /sys/class/ieee80211/phy* 2>/dev/null | head -2)

    if [ "$(echo "$phys" | wc -l)" -lt 2 ]; then
        echo "ERROR: Could not find two hwsim phy devices." >&2
        return 1
    fi

    local phy0 phy1
    phy0=$(basename "$(echo "$phys" | sed -n '2p')")  # older = first loaded
    phy1=$(basename "$(echo "$phys" | sed -n '1p')")  # newer = second loaded

    # Get interface names from the phy devices
    HWSIM_WLAN0=$(ls "/sys/class/ieee80211/$phy0/device/net/" 2>/dev/null | head -1)
    HWSIM_WLAN1=$(ls "/sys/class/ieee80211/$phy1/device/net/" 2>/dev/null | head -1)

    if [ -z "$HWSIM_WLAN0" ] || [ -z "$HWSIM_WLAN1" ]; then
        echo "ERROR: Could not determine interface names for hwsim phys." >&2
        return 1
    fi

    export HWSIM_WLAN0 HWSIM_WLAN1
    echo "Detected hwsim interfaces: $HWSIM_WLAN0 ($phy0), $HWSIM_WLAN1 ($phy1)"
}


hwsim_setup() {
    echo "=== Setting up mac80211_hwsim test environment ==="

    # Check privileges
    if [ "$(id -u)" -ne 0 ]; then
        echo "ERROR: Must run as root (need CAP_NET_RAW, namespace creation, module loading)." >&2
        return 1
    fi

    # Check for required tools
    for cmd in iw ip modprobe; do
        if ! command -v "$cmd" >/dev/null 2>&1; then
            echo "ERROR: Required command '$cmd' not found." >&2
            return 1
        fi
    done

    # Load mac80211_hwsim with 2 radios
    if lsmod | grep -q mac80211_hwsim; then
        echo "mac80211_hwsim already loaded, removing first..."
        modprobe -r mac80211_hwsim 2>/dev/null || true
        sleep 1
    fi

    modprobe mac80211_hwsim radios=2
    sleep 1

    _detect_hwsim_interfaces || return 1

    # -----------------------------------------------------------
    # Monitor-mode interfaces for raw 802.11 frame injection tests
    # -----------------------------------------------------------
    # Create monitor-mode virtual interfaces on top of each phy
    local phy0 phy1
    phy0=$(basename "$(readlink -f "/sys/class/net/$HWSIM_WLAN0/phy80211")")
    phy1=$(basename "$(readlink -f "/sys/class/net/$HWSIM_WLAN1/phy80211")")

    iw phy "$phy0" interface add mon0 type monitor 2>/dev/null || true
    iw phy "$phy1" interface add mon1 type monitor 2>/dev/null || true

    ip link set mon0 up 2>/dev/null || true
    ip link set mon1 up 2>/dev/null || true

    HWSIM_MON0="mon0"
    HWSIM_MON1="mon1"
    export HWSIM_MON0 HWSIM_MON1

    echo "Monitor interfaces: $HWSIM_MON0, $HWSIM_MON1"

    # -----------------------------------------------------------
    # Ad-hoc network for broadcast UDP tests
    # -----------------------------------------------------------
    # Create network namespaces to isolate the two stations
    ip netns add "$HWSIM_NS0" 2>/dev/null || true
    ip netns add "$HWSIM_NS1" 2>/dev/null || true

    # Move wlan interfaces into their namespaces
    iw phy "$phy0" set netns name "$HWSIM_NS0"
    iw phy "$phy1" set netns name "$HWSIM_NS1"

    # Configure ad-hoc (IBSS) mode in each namespace
    # Station 0
    ip netns exec "$HWSIM_NS0" ip link set "$HWSIM_WLAN0" down
    ip netns exec "$HWSIM_NS0" iw dev "$HWSIM_WLAN0" set type ibss
    ip netns exec "$HWSIM_NS0" ip link set "$HWSIM_WLAN0" up
    ip netns exec "$HWSIM_NS0" iw dev "$HWSIM_WLAN0" ibss join AlpineTest 2412
    ip netns exec "$HWSIM_NS0" ip addr add "$HWSIM_IP0/24" broadcast "$HWSIM_BCAST" dev "$HWSIM_WLAN0"

    # Station 1
    ip netns exec "$HWSIM_NS1" ip link set "$HWSIM_WLAN1" down
    ip netns exec "$HWSIM_NS1" iw dev "$HWSIM_WLAN1" set type ibss
    ip netns exec "$HWSIM_NS1" ip link set "$HWSIM_WLAN1" up
    ip netns exec "$HWSIM_NS1" iw dev "$HWSIM_WLAN1" ibss join AlpineTest 2412
    ip netns exec "$HWSIM_NS1" ip addr add "$HWSIM_IP1/24" broadcast "$HWSIM_BCAST" dev "$HWSIM_WLAN1"

    sleep 2  # allow IBSS association

    export HWSIM_NS0 HWSIM_NS1 HWSIM_IP0 HWSIM_IP1 HWSIM_BCAST

    echo "Ad-hoc network: $HWSIM_IP0 <-> $HWSIM_IP1 (broadcast $HWSIM_BCAST)"
    echo "Namespaces: $HWSIM_NS0, $HWSIM_NS1"
    echo "=== hwsim environment ready ==="
}


hwsim_teardown() {
    echo "=== Tearing down mac80211_hwsim test environment ==="

    # Remove monitor interfaces
    ip link set mon0 down 2>/dev/null || true
    ip link set mon1 down 2>/dev/null || true
    iw dev mon0 del 2>/dev/null || true
    iw dev mon1 del 2>/dev/null || true

    # Remove namespaces (this also moves interfaces back)
    ip netns del "$HWSIM_NS0" 2>/dev/null || true
    ip netns del "$HWSIM_NS1" 2>/dev/null || true

    # Unload module
    modprobe -r mac80211_hwsim 2>/dev/null || true

    unset HWSIM_MON0 HWSIM_MON1 HWSIM_WLAN0 HWSIM_WLAN1
    unset HWSIM_NS0 HWSIM_NS1 HWSIM_IP0 HWSIM_IP1 HWSIM_BCAST

    echo "=== hwsim environment torn down ==="
}
