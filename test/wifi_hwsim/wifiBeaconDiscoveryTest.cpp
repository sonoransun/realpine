///////
///
///  Copyright (C) 2026  sonoransun
///
///  WiFi beacon injection and discovery test using mac80211_hwsim.
///
///  Tests the full WifiDiscovery + WifiBeaconInjector flow:
///    - Station A starts beacon injection on one virtual radio
///    - Station B starts beacon injection on the other virtual radio
///    - Both should discover each other via parsed beacon vendor IEs
///
///  This validates the entire 802.11 beacon encode/decode pipeline
///  and the peer discovery state machine over the hwsim virtual medium.
///
///  Usage:
///      wifiBeaconDiscoveryTest <iface0> <iface1>
///
///  Requires: Linux, root/CAP_NET_RAW, mac80211_hwsim loaded,
///            both interfaces in monitor mode
///
///////


#ifdef __linux__

#include <Common.h>
#include <Log.h>
#include <NetUtils.h>
#include <WifiDiscovery.h>

#include <chrono>
#include <cstring>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>


// Peer configuration for the two virtual stations
struct StationConfig
{
    string peerId;
    string ipAddress;
    ushort port;
    ushort restPort;
    uint capabilities;
    string interfaceName;
};


static bool
runStation(const StationConfig & self, const string & expectedPeerId, int timeoutSec)
{
    Log::Info("Station ["s + self.peerId + "]: Starting on interface " + self.interfaceName);

    // Configure WifiDiscovery
    WifiDiscovery::setWifiInterface(self.interfaceName);
    WifiDiscovery::setBeaconInterval(500);      // 500ms for faster testing
    WifiDiscovery::setAnnounceInterval(1);      // 1s multicast announce
    WifiDiscovery::setPeerTimeout(timeoutSec);  // same as test timeout

    // Use a non-standard multicast port to avoid collisions with production
    WifiDiscovery::setMulticastPort(41799);

    if (!WifiDiscovery::initialize(self.peerId, self.ipAddress, self.port, self.restPort, self.capabilities)) {
        Log::Error("Station ["s + self.peerId + "]: WifiDiscovery init failed.");
        return false;
    }

    Log::Info("Station ["s + self.peerId +
              "]: WifiDiscovery initialized. "
              "Waiting for peer '" +
              expectedPeerId + "'...");

    // Poll for the expected peer
    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(timeoutSec);

    bool peerFound = false;

    while (std::chrono::steady_clock::now() < deadline) {
        WifiDiscovery::t_PeerMap peers;
        WifiDiscovery::getDiscoveredPeers(peers);

        auto it = peers.find(expectedPeerId);
        if (it != peers.end()) {
            const auto & peer = it->second;

            Log::Info("Station ["s + self.peerId + "]: Found peer '" + peer.peerId + "' at " + peer.ipAddress + ":" +
                      std::to_string(peer.port));

            // Validate peer info
            if (peer.port == 0) {
                Log::Error("Station ["s + self.peerId + "]: Peer port is 0, expected non-zero.");
                WifiDiscovery::shutdown();
                return false;
            }

            if (peer.ipAddress.empty()) {
                Log::Error("Station ["s + self.peerId + "]: Peer IP is empty.");
                WifiDiscovery::shutdown();
                return false;
            }

            peerFound = true;
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }

    WifiDiscovery::shutdown();

    if (!peerFound) {
        Log::Error("Station ["s + self.peerId + "]: Timeout - peer '" + expectedPeerId + "' not discovered.");
        return false;
    }

    Log::Info("Station ["s + self.peerId + "]: Peer discovery OK.");
    return true;
}


int
main(int argc, char * argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <monitor_iface0> <monitor_iface1>\n", argv[0]);
        return 1;
    }

    string iface0 = argv[1];
    string iface1 = argv[2];

    Log::initialize("/tmp/alpine_wifi_tests/beacon_discovery_test.log", Log::t_LogLevel::Debug);

    Log::Info("=== WiFi Beacon Discovery Test ===");
    Log::Info("Interface 0: "s + iface0);
    Log::Info("Interface 1: "s + iface1);

    static constexpr int DISCOVERY_TIMEOUT_SEC = 15;

    StationConfig station0{.peerId = "alpine-test-peer-A",
                           .ipAddress = "10.55.0.1",
                           .port = 9000,
                           .restPort = 9080,
                           .capabilities = WifiDiscovery::CAP_QUERY | WifiDiscovery::CAP_TRANSFER,
                           .interfaceName = iface0};

    StationConfig station1{.peerId = "alpine-test-peer-B",
                           .ipAddress = "10.55.0.2",
                           .port = 9001,
                           .restPort = 9081,
                           .capabilities = WifiDiscovery::CAP_QUERY,
                           .interfaceName = iface1};

    // Fork: child runs station 1, parent runs station 0.
    // WifiDiscovery uses static state, so they must be in separate processes.
    pid_t pid = fork();

    if (pid < 0) {
        Log::Error("fork() failed.");
        return 1;
    }

    if (pid == 0) {
        // Child: station 1 expects to find station 0
        Log::initialize("/tmp/alpine_wifi_tests/beacon_station1.log", Log::t_LogLevel::Debug);

        bool ok = runStation(station1, station0.peerId, DISCOVERY_TIMEOUT_SEC);
        _exit(ok ? 0 : 1);
    }

    // Parent: station 0 expects to find station 1
    bool station0Ok = runStation(station0, station1.peerId, DISCOVERY_TIMEOUT_SEC);

    // Wait for child
    int status = 0;
    waitpid(pid, &status, 0);
    bool station1Ok = WIFEXITED(status) && WEXITSTATUS(status) == 0;

    if (station0Ok && station1Ok) {
        Log::Info("=== TEST PASSED: Both stations discovered each other ===");
        return 0;
    }

    if (!station0Ok)
        Log::Error("Station 0 failed.");
    if (!station1Ok)
        Log::Error("Station 1 failed.");
    Log::Error("=== TEST FAILED ===");
    return 1;
}


#else  // Non-Linux


#include <cstdio>

int
main(int argc, char * argv[])
{
    fprintf(stderr, "SKIP: Beacon injection tests require Linux.\n");
    return 77;
}


#endif
