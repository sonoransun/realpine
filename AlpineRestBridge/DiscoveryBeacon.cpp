/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <DiscoveryBeacon.h>
#include <JsonWriter.h>
#include <Log.h>
#include <ThreadUtils.h>

#include <Platform.h>

#include <cstring>

#ifdef __linux__
#include <netinet/in.h>
#endif


DiscoveryBeacon::DiscoveryBeacon()
    : restPort_(0),
      beaconPort_(0)
{}


DiscoveryBeacon::~DiscoveryBeacon()
{
    DiscoveryBeacon::stop();
    udpSocket_.close();
}


bool
DiscoveryBeacon::initialize(ushort restPort, ushort beaconPort)
{
    return initialize(restPort, beaconPort, ""s);
}


bool
DiscoveryBeacon::initialize(ushort restPort, ushort beaconPort, const string & multicastGroup)
{
    restPort_ = restPort;
    beaconPort_ = beaconPort;
    multicastGroup_ = multicastGroup;

    if (!udpSocket_.create()) {
        Log::Error("DiscoveryBeacon: Failed to create UDP socket.");
        return false;
    }
    int broadcastEnable = 1;

    if (setsockopt(udpSocket_.getFd(), SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable)) < 0) {
        Log::Error("DiscoveryBeacon: Failed to enable SO_BROADCAST.");
        udpSocket_.close();
        return false;
    }

    // IP_FREEBIND lets the daemon start before its interface has an address
    // assigned (important for systemd dynamic networking). Linux-only and
    // best-effort: log on failure but do not abort initialization.
    //
#ifdef IP_FREEBIND
    int freebind = 1;
    if (setsockopt(udpSocket_.getFd(), IPPROTO_IP, IP_FREEBIND, &freebind, sizeof(freebind)) < 0) {
        Log::Info("DiscoveryBeacon: IP_FREEBIND not available (continuing).");
    }
#endif

    // If an IPv6 multicast group is configured, join it
    if (!multicastGroup_.empty() && multicastGroup_.contains(':')) {
        struct sockaddr_in6 mcastAddr;
        memset(&mcastAddr, 0, sizeof(mcastAddr));
        if (inet_pton(AF_INET6, multicastGroup_.c_str(), &mcastAddr.sin6_addr) == 1) {
            struct ipv6_mreq mreq6;
            mreq6.ipv6mr_multiaddr = mcastAddr.sin6_addr;
            mreq6.ipv6mr_interface = 0;

            if (setsockopt(udpSocket_.getFd(), IPPROTO_IPV6, IPV6_JOIN_GROUP, &mreq6, sizeof(mreq6)) < 0) {
                Log::Info("DiscoveryBeacon: IPv6 multicast join failed, "
                          "using IPv4 broadcast only.");
            } else {
                Log::Info("DiscoveryBeacon: Joined IPv6 multicast group "s + multicastGroup_);
            }
        }
    }

    Log::Info("DiscoveryBeacon: Initialized on beacon port "s + std::to_string(beaconPort_));

    return true;
}


void
DiscoveryBeacon::threadMain()
{
    ThreadUtils::setCurrentThreadName("alpine-beacon"s);

    Log::Info("DiscoveryBeacon: Thread started.");

    while (isActive()) {

        // Build beacon JSON payload
        //
        JsonWriter json;
        json.beginObject();
        json.key("service");
        json.value("alpine-bridge");
        json.key("version");
        json.value("1");
        json.key("restPort");
        json.value((ulong)restPort_);
        json.key("bridgeVersion");
        json.value("devel-00019");
        json.endObject();

        string payload = json.result();

        // Send to broadcast address
        //
        bool sent =
            udpSocket_.sendData(INADDR_BROADCAST, htons(beaconPort_), (const byte *)payload.c_str(), payload.length());

        if (!sent) {
            Log::Debug("DiscoveryBeacon: Failed to send beacon.");
            continue;
        }

        // Wait for the next beacon interval, waking immediately on stop()
        //
        {
            std::unique_lock lock(cvMutex_);
            cv_.wait_for(lock, std::chrono::seconds(BEACON_INTERVAL_SEC), [this] { return !isActive(); });
        }
    }

    Log::Info("DiscoveryBeacon: Thread exiting.");
}


bool
DiscoveryBeacon::stop()
{
    cv_.notify_all();
    return SysThread::stop();
}
