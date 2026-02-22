# Integration tests for Alpine 802.11 wireless features using mac80211_hwsim.
#
# These tests require:
#   - Linux with mac80211_hwsim kernel module
#   - Root privileges (AF_PACKET raw sockets, module loading, namespaces)
#
# Run with: sudo test/wifi_hwsim/run_wifi_tests.sh [build_dir]

add_executable(wifiRawConnTest
    wifiRawConnTest.cpp
)
target_link_libraries(wifiRawConnTest PRIVATE
    Alpine::NetUtils
    Alpine::AppUtils
    Alpine::ThreadUtils
)

add_executable(wifiBroadcastConnTest
    wifiBroadcastConnTest.cpp
)
target_link_libraries(wifiBroadcastConnTest PRIVATE
    Alpine::NetUtils
    Alpine::AppUtils
    Alpine::ThreadUtils
)

add_executable(wifiBeaconDiscoveryTest
    wifiBeaconDiscoveryTest.cpp
)
target_link_libraries(wifiBeaconDiscoveryTest PRIVATE
    Alpine::NetUtils
    Alpine::AppUtils
    Alpine::ThreadUtils
)
