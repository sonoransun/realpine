/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <WifiDiscovery.h>
#include <MulticastDiscovery.h>
#include <WifiBeaconInjector.h>
#include <Log.h>


bool                                 WifiDiscovery::enabled_s          = false;
string                               WifiDiscovery::multicastGroup_s   = "224.0.1.75";
ushort                               WifiDiscovery::multicastPort_s    = 41750;
int                                  WifiDiscovery::announceInterval_s = 5;
int                                  WifiDiscovery::peerTimeout_s      = 30;
string                               WifiDiscovery::wifiInterface_s;
int                                  WifiDiscovery::beaconInterval_s   = 3000;

WifiDiscovery::t_PeerMap             WifiDiscovery::peers_s;
Mutex                                WifiDiscovery::peerLock_s;
WifiDiscovery::t_PeerCallback        WifiDiscovery::peerCallback_s;

MulticastDiscovery *                 WifiDiscovery::multicast_s = nullptr;
WifiBeaconInjector *                 WifiDiscovery::beacon_s    = nullptr;



void
WifiDiscovery::setMulticastGroup (const string & group)
{
    multicastGroup_s = group;
}


void
WifiDiscovery::setMulticastPort (ushort port)
{
    multicastPort_s = port;
}


void
WifiDiscovery::setAnnounceInterval (int seconds)
{
    announceInterval_s = seconds;
}


void
WifiDiscovery::setPeerTimeout (int seconds)
{
    peerTimeout_s = seconds;
}


void
WifiDiscovery::setWifiInterface (const string & ifName)
{
    wifiInterface_s = ifName;
}


void
WifiDiscovery::setBeaconInterval (int milliseconds)
{
    beaconInterval_s = milliseconds;
}



bool
WifiDiscovery::initialize (const string & peerId, const string & ipAddress,
                           ushort port, ushort restPort, uint capabilities)
{
    bool anyStarted = false;

    // Start multicast discovery
    multicast_s = new MulticastDiscovery();

    if (multicast_s->initialize(multicastGroup_s, multicastPort_s,
                                peerId, ipAddress, port, restPort,
                                capabilities, announceInterval_s,
                                peerTimeout_s)) {
        multicast_s->run();
        anyStarted = true;
        Log::Info("WiFi multicast discovery started on "s +
                  multicastGroup_s + ":" + std::to_string(multicastPort_s));
    } else {
        Log::Error("WiFi multicast discovery failed to initialize.");
        delete multicast_s;
        multicast_s = nullptr;
    }

    // Start beacon injector if interface configured
    if (!wifiInterface_s.empty()) {
        beacon_s = new WifiBeaconInjector();

        if (beacon_s->initialize(wifiInterface_s, peerId, ipAddress,
                                 port, restPort, capabilities,
                                 beaconInterval_s)) {
            beacon_s->run();
            anyStarted = true;
            Log::Info("WiFi beacon injector started on interface "s +
                      wifiInterface_s);
        } else {
            Log::Error("WiFi beacon injector failed to initialize (continuing without beacons).");
            delete beacon_s;
            beacon_s = nullptr;
        }
    }

    enabled_s = anyStarted;
    return anyStarted;
}



void
WifiDiscovery::shutdown ()
{
    if (beacon_s) {
        beacon_s->stop();
        delete beacon_s;
        beacon_s = nullptr;
    }

    if (multicast_s) {
        multicast_s->stop();
        delete multicast_s;
        multicast_s = nullptr;
    }

    peerLock_s.acquire();
    peers_s.clear();
    peerLock_s.release();

    enabled_s = false;
}



bool
WifiDiscovery::isEnabled ()
{
    return enabled_s;
}



bool
WifiDiscovery::getDiscoveredPeers (t_PeerMap & peers)
{
    peerLock_s.acquire();
    expireStale();
    peers = peers_s;
    peerLock_s.release();
    return !peers.empty();
}



uint
WifiDiscovery::getDiscoveredPeerCount ()
{
    peerLock_s.acquire();
    expireStale();
    uint count = static_cast<uint>(peers_s.size());
    peerLock_s.release();
    return count;
}



void
WifiDiscovery::setPeerCallback (t_PeerCallback callback)
{
    peerLock_s.acquire();
    peerCallback_s = std::move(callback);
    peerLock_s.release();
}



void
WifiDiscovery::reportPeer (const t_DiscoveredPeer & peer)
{
    peerLock_s.acquire();

    bool isNew = (peers_s.find(peer.peerId) == peers_s.end());
    peers_s[peer.peerId] = peer;

    t_PeerCallback cb = peerCallback_s;
    peerLock_s.release();

    if (cb) {
        cb(peer, isNew);
    }

    if (isNew) {
        Log::Info("WiFi discovery: new peer "s + peer.peerId +
                  " at " + peer.ipAddress + ":" + std::to_string(peer.port));
    }
}



void
WifiDiscovery::expireStale ()
{
    // Must be called with peerLock_s held
    auto now = std::chrono::steady_clock::now();

    for (auto it = peers_s.begin(); it != peers_s.end(); ) {
        auto age = std::chrono::duration_cast<std::chrono::seconds>(
                       now - it->second.lastSeen).count();

        if (age > peerTimeout_s) {
            Log::Debug("WiFi discovery: peer "s + it->second.peerId + " expired.");

            t_PeerCallback cb = peerCallback_s;
            if (cb) {
                cb(it->second, false);
            }

            it = peers_s.erase(it);
        } else {
            ++it;
        }
    }
}
