/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <Log.h>
#include <MulticastDiscovery.h>
#include <WifiDiscovery.h>

#include <Platform.h>
#include <cstring>
#include <ctime>


MulticastDiscovery::MulticastDiscovery()
    : mcastFd_(-1),
      multicastPort_(0),
      port_(0),
      restPort_(0),
      capabilities_(0),
      announceIntervalSec_(5),
      peerTimeoutSec_(30)
{}


MulticastDiscovery::~MulticastDiscovery()
{
    stop();

    if (mcastFd_ >= 0)
        alpine_close_socket(mcastFd_);
}


bool
MulticastDiscovery::initialize(const string & multicastGroup,
                               ushort multicastPort,
                               const string & peerId,
                               const string & ipAddress,
                               ushort port,
                               ushort restPort,
                               uint capabilities,
                               int announceIntervalSec,
                               int peerTimeoutSec)
{
    multicastGroup_ = multicastGroup;
    multicastPort_ = multicastPort;
    peerId_ = peerId;
    ipAddress_ = ipAddress;
    port_ = port;
    restPort_ = restPort;
    capabilities_ = capabilities;
    announceIntervalSec_ = announceIntervalSec;
    peerTimeoutSec_ = peerTimeoutSec;

    mcastFd_ = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);

    if (mcastFd_ < 0) {
        Log::Error("MulticastDiscovery: Failed to create socket.");
        return false;
    }

    int reuse = 1;
    setsockopt(mcastFd_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

#ifdef SO_REUSEPORT
    setsockopt(mcastFd_, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse));
#endif

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(multicastPort_);

    if (bind(mcastFd_, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        Log::Error("MulticastDiscovery: Failed to bind to port "s + std::to_string(multicastPort_) + ".");
        alpine_close_socket(mcastFd_);
        mcastFd_ = -1;
        return false;
    }

    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr(multicastGroup_.c_str());
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);

    if (setsockopt(mcastFd_, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
        Log::Error("MulticastDiscovery: Failed to join multicast group "s + multicastGroup_ + ".");
        alpine_close_socket(mcastFd_);
        mcastFd_ = -1;
        return false;
    }

    // Set TTL=1 (link-local only)
    int ttl = 1;
    setsockopt(mcastFd_, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));

    // Disable loopback
    int loop = 0;
    setsockopt(mcastFd_, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop));

    Log::Info("MulticastDiscovery: Initialized on "s + multicastGroup_ + ":" + std::to_string(multicastPort_));
    return true;
}


void
MulticastDiscovery::threadMain()
{
    Log::Info("MulticastDiscovery: Thread started.");

    sendAnnouncement();

    int tickCount = 0;

    while (isActive()) {
        struct pollfd pfd;
        pfd.fd = mcastFd_;
        pfd.events = POLLIN;
        pfd.revents = 0;

        int ret = alpine_poll(&pfd, 1, POLL_TIMEOUT_MS);

        if (ret > 0 && (pfd.revents & POLLIN)) {
            byte buffer[2048];
            struct sockaddr_in srcAddr;
            socklen_t srcLen = sizeof(srcAddr);

            ssize_t bytesRead = recvfrom(mcastFd_, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)&srcAddr, &srcLen);

            if (bytesRead > 0) {
                buffer[bytesRead] = 0;
                handleReceived(
                    buffer, static_cast<uint>(bytesRead), ntohl(srcAddr.sin_addr.s_addr), ntohs(srcAddr.sin_port));
            }
        }

        tickCount++;

        if (tickCount >= announceIntervalSec_) {
            sendAnnouncement();
            tickCount = 0;
        }
    }

    Log::Info("MulticastDiscovery: Thread exiting.");
}


void
MulticastDiscovery::sendAnnouncement()
{
    // Build capabilities array string
    string capStr = "[";
    bool first = true;

    if (capabilities_ & WifiDiscovery::CAP_QUERY) {
        capStr += "\"query\"";
        first = false;
    }
    if (capabilities_ & WifiDiscovery::CAP_TRANSFER) {
        if (!first)
            capStr += ", ";
        capStr += "\"transfer\"";
        first = false;
    }
    if (capabilities_ & WifiDiscovery::CAP_MEDIA) {
        if (!first)
            capStr += ", ";
        capStr += "\"media\"";
    }
    capStr += "]";

    auto now = std::time(nullptr);

    string json = "{"
                  "\"type\": \"alpine_discover\", "
                  "\"version\": \"1\", "
                  "\"peerId\": \""s +
                  peerId_ +
                  "\", "
                  "\"ipAddress\": \"" +
                  ipAddress_ +
                  "\", "
                  "\"port\": " +
                  std::to_string(port_) +
                  ", "
                  "\"restPort\": " +
                  std::to_string(restPort_) +
                  ", "
                  "\"capabilities\": " +
                  capStr +
                  ", "
                  "\"timestamp\": " +
                  std::to_string(now) + "}";

    struct sockaddr_in destAddr;
    memset(&destAddr, 0, sizeof(destAddr));
    destAddr.sin_family = AF_INET;
    destAddr.sin_addr.s_addr = inet_addr(multicastGroup_.c_str());
    destAddr.sin_port = htons(multicastPort_);

    sendto(mcastFd_, json.c_str(), json.length(), 0, (struct sockaddr *)&destAddr, sizeof(destAddr));
}


void
MulticastDiscovery::handleReceived(const byte * data, uint length, ulong srcIp, ushort srcPort)
{
    string msg(reinterpret_cast<const char *>(data), length);

    // Must be an alpine_discover message
    if (!msg.contains("\"alpine_discover\""))
        return;

    // Extract peerId
    string peerId;
    auto pidPos = msg.find("\"peerId\"");
    if (pidPos != string::npos) {
        auto valStart = msg.find('"', pidPos + 8);
        if (valStart != string::npos) {
            valStart++;
            auto valEnd = msg.find('"', valStart);
            if (valEnd != string::npos)
                peerId = msg.substr(valStart, valEnd - valStart);
        }
    }

    if (peerId.empty())
        return;

    // Self-suppression
    if (peerId == peerId_)
        return;

    // Extract ipAddress
    string ipAddr;
    auto ipPos = msg.find("\"ipAddress\"");
    if (ipPos != string::npos) {
        auto valStart = msg.find('"', ipPos + 11);
        if (valStart != string::npos) {
            valStart++;
            auto valEnd = msg.find('"', valStart);
            if (valEnd != string::npos)
                ipAddr = msg.substr(valStart, valEnd - valStart);
        }
    }

    // Extract port
    ushort peerPort = 0;
    auto portPos = msg.find("\"port\"");
    if (portPos != string::npos) {
        auto valStart = msg.find(':', portPos + 6);
        if (valStart != string::npos) {
            valStart++;
            while (valStart < length && msg[valStart] == ' ')
                valStart++;
            peerPort = static_cast<ushort>(atoi(msg.c_str() + valStart));
        }
    }

    // Extract restPort
    ushort peerRestPort = 0;
    auto rpPos = msg.find("\"restPort\"");
    if (rpPos != string::npos) {
        auto valStart = msg.find(':', rpPos + 10);
        if (valStart != string::npos) {
            valStart++;
            while (valStart < length && msg[valStart] == ' ')
                valStart++;
            peerRestPort = static_cast<ushort>(atoi(msg.c_str() + valStart));
        }
    }

    // Extract capabilities bitmask from array
    uint caps = 0;
    if (msg.contains("\"query\""))
        caps |= WifiDiscovery::CAP_QUERY;
    if (msg.contains("\"transfer\""))
        caps |= WifiDiscovery::CAP_TRANSFER;
    if (msg.contains("\"media\""))
        caps |= WifiDiscovery::CAP_MEDIA;

    WifiDiscovery::t_DiscoveredPeer peer;
    peer.peerId = peerId;
    peer.ipAddress = ipAddr;
    peer.port = peerPort;
    peer.restPort = peerRestPort;
    peer.protocolVersion = 1;
    peer.capabilities = caps;
    peer.lastSeen = std::chrono::steady_clock::now();

    WifiDiscovery::reportPeer(peer);
}
