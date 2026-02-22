/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <WifiBeaconInjector.h>
#include <WifiDiscovery.h>
#include <Log.h>

#include <Platform.h>
#include <cstring>


#ifdef __linux__
#include <net/if.h>
#include <sys/ioctl.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <netinet/in.h>
#endif



WifiBeaconInjector::WifiBeaconInjector ()
    :
#ifdef __linux__
      rawFd_(-1),
      ifIndex_(0),
#endif
      port_(0),
      restPort_(0),
      capabilities_(0),
      beaconIntervalMs_(3000)
{
#ifdef __linux__
    memset(localMac_, 0, sizeof(localMac_));
#endif
}



WifiBeaconInjector::~WifiBeaconInjector ()
{
    stop();

#ifdef __linux__
    if (rawFd_ >= 0)
        ::close(rawFd_);
#endif
}



#ifdef __linux__

bool
WifiBeaconInjector::initialize (const string & interfaceName, const string & peerId,
                                const string & ipAddress, ushort port, ushort restPort,
                                uint capabilities, int beaconIntervalMs)
{
    interfaceName_    = interfaceName;
    peerId_           = peerId;
    ipAddress_        = ipAddress;
    port_             = port;
    restPort_         = restPort;
    capabilities_     = capabilities;
    beaconIntervalMs_ = beaconIntervalMs;

    if (!openRawSocket()) {
        Log::Error("WifiBeaconInjector: Failed to open raw socket on "s +
                   interfaceName_);
        return false;
    }

    Log::Info("WifiBeaconInjector: Initialized on interface "s + interfaceName_);
    return true;
}



bool
WifiBeaconInjector::openRawSocket ()
{
    rawFd_ = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

    if (rawFd_ < 0) {
        Log::Error("WifiBeaconInjector: socket() failed (need root/CAP_NET_RAW).");
        return false;
    }

    // Get interface index
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, interfaceName_.c_str(), IFNAMSIZ - 1);

    if (ioctl(rawFd_, SIOCGIFINDEX, &ifr) < 0) {
        Log::Error("WifiBeaconInjector: Failed to get interface index for "s +
                   interfaceName_);
        ::close(rawFd_);
        rawFd_ = -1;
        return false;
    }

    ifIndex_ = ifr.ifr_ifindex;

    // Get MAC address
    if (ioctl(rawFd_, SIOCGIFHWADDR, &ifr) < 0) {
        Log::Error("WifiBeaconInjector: Failed to get MAC address for "s +
                   interfaceName_);
        ::close(rawFd_);
        rawFd_ = -1;
        return false;
    }

    memcpy(localMac_, ifr.ifr_hwaddr.sa_data, 6);

    // Bind to interface
    struct sockaddr_ll sll;
    memset(&sll, 0, sizeof(sll));
    sll.sll_family   = AF_PACKET;
    sll.sll_ifindex  = ifIndex_;
    sll.sll_protocol = htons(ETH_P_ALL);

    if (bind(rawFd_, (struct sockaddr *)&sll, sizeof(sll)) < 0) {
        Log::Error("WifiBeaconInjector: bind() failed.");
        ::close(rawFd_);
        rawFd_ = -1;
        return false;
    }

    return true;
}



void
WifiBeaconInjector::threadMain ()
{
    Log::Info("WifiBeaconInjector: Thread started.");

    int ticksPerBeacon = beaconIntervalMs_ / POLL_TIMEOUT_MS;
    if (ticksPerBeacon < 1) ticksPerBeacon = 1;
    int tickCount = ticksPerBeacon;  // send immediately on start

    while (isActive())
    {
        struct pollfd pfd;
        pfd.fd      = rawFd_;
        pfd.events  = POLLIN;
        pfd.revents = 0;

        int ret = alpine_poll(&pfd, 1, POLL_TIMEOUT_MS);

        if (ret > 0 && (pfd.revents & POLLIN))
        {
            byte buffer[4096];
            ssize_t bytesRead = recv(rawFd_, buffer, sizeof(buffer), 0);

            if (bytesRead > 0) {
                parseBeaconFrame(buffer, static_cast<uint>(bytesRead));
            }
        }

        tickCount++;

        if (tickCount >= ticksPerBeacon) {
            byte frame[512];
            uint frameLen = buildBeaconFrame(frame, sizeof(frame));

            if (frameLen > 0) {
                ssize_t sent = send(rawFd_, frame, frameLen, 0);
                if (sent < 0) {
                    Log::Debug("WifiBeaconInjector: Failed to inject beacon frame.");
                }
            }

            tickCount = 0;
        }
    }

    Log::Info("WifiBeaconInjector: Thread exiting.");
}



uint
WifiBeaconInjector::buildBeaconFrame (byte * buffer, uint bufferSize)
{
    if (bufferSize < 200)
        return 0;

    uint offset = 0;

    // --- Radiotap header (8 bytes) ---
    buffer[offset++] = 0x00;  // version
    buffer[offset++] = 0x00;  // pad
    buffer[offset++] = 0x08;  // length (LE)
    buffer[offset++] = 0x00;
    buffer[offset++] = 0x00;  // present flags (none)
    buffer[offset++] = 0x00;
    buffer[offset++] = 0x00;
    buffer[offset++] = 0x00;

    // --- 802.11 MAC header (24 bytes) ---
    // Frame Control: beacon (0x0080)
    buffer[offset++] = 0x80;
    buffer[offset++] = 0x00;
    // Duration
    buffer[offset++] = 0x00;
    buffer[offset++] = 0x00;
    // Destination: broadcast
    memset(buffer + offset, 0xFF, 6);
    offset += 6;
    // Source: local MAC
    memcpy(buffer + offset, localMac_, 6);
    offset += 6;
    // BSSID: local MAC
    memcpy(buffer + offset, localMac_, 6);
    offset += 6;
    // Sequence control
    buffer[offset++] = 0x00;
    buffer[offset++] = 0x00;

    // --- Fixed parameters (12 bytes) ---
    // Timestamp (8 bytes, zeros)
    memset(buffer + offset, 0, 8);
    offset += 8;
    // Beacon interval: 100 TU (0x0064)
    buffer[offset++] = 0x64;
    buffer[offset++] = 0x00;
    // Capability info: ESS
    buffer[offset++] = 0x01;
    buffer[offset++] = 0x00;

    // --- SSID IE (hidden, 2 bytes) ---
    buffer[offset++] = 0x00;  // tag: SSID
    buffer[offset++] = 0x00;  // length: 0

    // --- Vendor-specific IE ---
    offset = buildVendorIE(buffer, offset);

    return offset;
}



uint
WifiBeaconInjector::buildVendorIE (byte * buffer, uint offset)
{
    uint peerIdLen = static_cast<uint>(peerId_.length());
    if (peerIdLen > 64) peerIdLen = 64;

    // IE payload length = 3(OUI) + 1(type) + 1(version) + 1(caps) +
    //                     4(ip) + 2(port) + 2(restPort) + 1(idLen) + idLen
    uint payloadLen = 3 + 1 + 1 + 1 + 4 + 2 + 2 + 1 + peerIdLen;

    buffer[offset++] = 0xDD;                // tag: vendor-specific
    buffer[offset++] = static_cast<byte>(payloadLen);

    // OUI: 02:A1:E1
    buffer[offset++] = ALPINE_OUI[0];
    buffer[offset++] = ALPINE_OUI[1];
    buffer[offset++] = ALPINE_OUI[2];

    // OUI Type
    buffer[offset++] = ALPINE_OUI_TYPE;

    // Protocol version
    buffer[offset++] = 0x01;

    // Capability flags
    buffer[offset++] = static_cast<byte>(capabilities_ & 0xFF);

    // IPv4 address (network order)
    struct in_addr inAddr;
    inet_aton(ipAddress_.c_str(), &inAddr);
    memcpy(buffer + offset, &inAddr.s_addr, 4);
    offset += 4;

    // Alpine port (network order)
    ushort netPort = htons(port_);
    memcpy(buffer + offset, &netPort, 2);
    offset += 2;

    // REST port (network order)
    ushort netRestPort = htons(restPort_);
    memcpy(buffer + offset, &netRestPort, 2);
    offset += 2;

    // PeerID length + string
    buffer[offset++] = static_cast<byte>(peerIdLen);
    memcpy(buffer + offset, peerId_.c_str(), peerIdLen);
    offset += peerIdLen;

    return offset;
}



void
WifiBeaconInjector::parseBeaconFrame (const byte * data, uint length)
{
    // Minimum: radiotap(8) + mac(24) + fixed(12) = 44
    if (length < 44)
        return;

    // Read radiotap header length (LE uint16 at offset 2)
    uint rtLen = data[2] | (data[3] << 8);
    if (rtLen >= length)
        return;

    uint macOffset = rtLen;
    if (macOffset + 24 > length)
        return;

    // Check frame control for beacon (0x80, 0x00)
    if (data[macOffset] != 0x80 || data[macOffset + 1] != 0x00)
        return;

    // Skip MAC header + fixed params
    uint ieOffset = macOffset + 24 + 12;
    if (ieOffset >= length)
        return;

    // Iterate tagged parameters
    while (ieOffset + 2 <= length) {
        byte tag    = data[ieOffset];
        byte tagLen = data[ieOffset + 1];

        if (ieOffset + 2 + tagLen > length)
            break;

        if (tag == 0xDD && tagLen >= 4) {
            parseVendorIE(data + ieOffset + 2, tagLen);
        }

        ieOffset += 2 + tagLen;
    }
}



bool
WifiBeaconInjector::parseVendorIE (const byte * ie, uint ieLen)
{
    // Minimum: OUI(3) + type(1) + version(1) + caps(1) + ip(4) + port(2) + restPort(2) + idLen(1) = 15
    if (ieLen < 15)
        return false;

    // Check OUI
    if (ie[0] != ALPINE_OUI[0] || ie[1] != ALPINE_OUI[1] || ie[2] != ALPINE_OUI[2])
        return false;

    // Check OUI type
    if (ie[3] != ALPINE_OUI_TYPE)
        return false;

    uint protoVersion = ie[4];
    uint caps         = ie[5];

    // IPv4 address
    struct in_addr inAddr;
    memcpy(&inAddr.s_addr, ie + 6, 4);
    string ipStr = inet_ntoa(inAddr);

    // Port
    ushort peerPort;
    memcpy(&peerPort, ie + 10, 2);
    peerPort = ntohs(peerPort);

    // REST port
    ushort peerRestPort;
    memcpy(&peerRestPort, ie + 12, 2);
    peerRestPort = ntohs(peerRestPort);

    // PeerID
    uint idLen = ie[14];
    if (idLen > 64 || 15 + idLen > ieLen)
        return false;

    string peerId(reinterpret_cast<const char *>(ie + 15), idLen);

    if (peerId.empty())
        return false;

    // Self-suppression: skip if same IP and port
    if (ipStr == ipAddress_ && peerPort == port_)
        return false;

    WifiDiscovery::t_DiscoveredPeer peer;
    peer.peerId          = peerId;
    peer.ipAddress       = ipStr;
    peer.port            = peerPort;
    peer.restPort        = peerRestPort;
    peer.protocolVersion = protoVersion;
    peer.capabilities    = caps;
    peer.lastSeen        = std::chrono::steady_clock::now();

    WifiDiscovery::reportPeer(peer);
    return true;
}


#else  // Non-Linux stubs


bool
WifiBeaconInjector::initialize (const string & interfaceName, const string & peerId,
                                const string & ipAddress, ushort port, ushort restPort,
                                uint capabilities, int beaconIntervalMs)
{
    (void)interfaceName;
    (void)peerId;
    (void)ipAddress;
    (void)port;
    (void)restPort;
    (void)capabilities;
    (void)beaconIntervalMs;

    Log::Error("WifiBeaconInjector: Raw 802.11 injection is only supported on Linux.");
    return false;
}



void
WifiBeaconInjector::threadMain ()
{
    // Non-Linux stub: should never be called since initialize() returns false
}


#endif
