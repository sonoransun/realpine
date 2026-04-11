/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <Mutex.h>
#include <chrono>


class MulticastDiscovery;
class WifiBeaconInjector;


class WifiDiscovery
{
  public:
    struct t_DiscoveredPeer
    {
        string peerId;
        string ipAddress;
        ushort port;
        ushort restPort;
        uint protocolVersion;
        uint capabilities;
        std::chrono::steady_clock::time_point lastSeen;
    };

    using t_PeerMap = std::unordered_map<string, t_DiscoveredPeer>;

    static constexpr uint CAP_QUERY = 0x01;
    static constexpr uint CAP_TRANSFER = 0x02;
    static constexpr uint CAP_MEDIA = 0x04;

    // Config (call before initialize)
    static void setMulticastGroup(const string & group);
    static void setMulticastPort(ushort port);
    static void setAnnounceInterval(int seconds);
    static void setPeerTimeout(int seconds);
    static void setWifiInterface(const string & ifName);
    static void setBeaconInterval(int milliseconds);

    // Lifecycle
    static bool
    initialize(const string & peerId, const string & ipAddress, ushort port, ushort restPort, uint capabilities);
    static void shutdown();
    static bool isEnabled();

    // Peer access
    static bool getDiscoveredPeers(t_PeerMap & peers);
    static uint getDiscoveredPeerCount();

    // Callback (optional)
    using t_PeerCallback = std::function<void(const t_DiscoveredPeer &, bool added)>;
    static void setPeerCallback(t_PeerCallback callback);

    // Called by MulticastDiscovery and WifiBeaconInjector
    static void reportPeer(const t_DiscoveredPeer & peer);


  private:
    static void expireStale();

    static bool enabled_s;
    static string multicastGroup_s;
    static ushort multicastPort_s;
    static int announceInterval_s;
    static int peerTimeout_s;
    static string wifiInterface_s;
    static int beaconInterval_s;

    static t_PeerMap peers_s;
    static Mutex peerLock_s;
    static t_PeerCallback peerCallback_s;

    static MulticastDiscovery * multicast_s;
    static WifiBeaconInjector * beacon_s;
};
