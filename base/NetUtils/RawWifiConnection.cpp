/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <CovertChannel.h>
#include <Log.h>
#include <NetUtils.h>
#include <RawWifiConnection.h>

#include <Platform.h>
#include <cstring>


#ifdef __linux__
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <unistd.h>
#endif


#ifdef __linux__


RawWifiConnection::RawWifiConnection(const string & interfaceName)
    : rawFd_(-1),
      ifIndex_(0),
      interfaceName_(interfaceName)
{
    memset(localMac_, 0, sizeof(localMac_));
}


RawWifiConnection::~RawWifiConnection()
{
    close();
}


bool
RawWifiConnection::create(ulong ipAddress, ushort port)
{
    if (rawFd_ >= 0) {
        close();
    }

    rawFd_ = socket(AF_PACKET, SOCK_RAW | SOCK_CLOEXEC, htons(ETH_P_ALL));

    if (rawFd_ < 0) {
        if (errno == EPERM) {
            Log::Error("RawWifiConnection: AF_PACKET socket denied (EPERM). "
                       "Run as root or grant CAP_NET_RAW: setcap cap_net_raw=ep <binary>"s);
        } else {
            Log::Error("RawWifiConnection: socket() failed (need root/CAP_NET_RAW).");
        }
        return false;
    }

    // Get interface index
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, interfaceName_.c_str(), IFNAMSIZ - 1);

    if (ioctl(rawFd_, SIOCGIFINDEX, &ifr) < 0) {
        Log::Error("RawWifiConnection: Failed to get interface index for "s + interfaceName_);
        ::close(rawFd_);
        rawFd_ = -1;
        return false;
    }

    ifIndex_ = ifr.ifr_ifindex;

    // Get MAC address
    if (ioctl(rawFd_, SIOCGIFHWADDR, &ifr) < 0) {
        Log::Error("RawWifiConnection: Failed to get MAC address for "s + interfaceName_);
        ::close(rawFd_);
        rawFd_ = -1;
        return false;
    }

    memcpy(localMac_, ifr.ifr_hwaddr.sa_data, 6);

    // Bind to interface
    struct sockaddr_ll sll;
    memset(&sll, 0, sizeof(sll));
    sll.sll_family = AF_PACKET;
    sll.sll_ifindex = ifIndex_;
    sll.sll_protocol = htons(ETH_P_ALL);

    if (bind(rawFd_, (struct sockaddr *)&sll, sizeof(sll)) < 0) {
        Log::Error("RawWifiConnection: bind() failed.");
        ::close(rawFd_);
        rawFd_ = -1;
        return false;
    }

    // Set non-blocking
    NetUtils::nonBlocking(rawFd_);

    Log::Info("RawWifiConnection: Created on interface "s + interfaceName_);

    return true;
}


void
RawWifiConnection::close()
{
    if (rawFd_ >= 0) {
        ::close(rawFd_);
        rawFd_ = -1;
    }
}


int
RawWifiConnection::getFd()
{
    return rawFd_;
}


bool
RawWifiConnection::sendData(const ulong destIpAddress,
                            const ushort destPort,
                            const byte * dataBuffer,
                            const uint dataLength)
{
    if (rawFd_ < 0) {
        return false;
    }

    const byte * sendBuffer = dataBuffer;
    vector<byte> obfuscatedBuffer;

    if (CovertChannel::isEnabled()) {
        obfuscatedBuffer.assign(dataBuffer, dataBuffer + dataLength);
        CovertChannel::obfuscate(obfuscatedBuffer.data(), dataLength);
        sendBuffer = obfuscatedBuffer.data();
    }

    // Build the 802.11 data frame with embedded IP/port addressing
    // in the LLC/SNAP header so DTCP's addressing still works.
    //
    // We encode the destination IP and port in the payload prefix
    // so the receiver can extract them for DTCP's connection mux.
    //
    // Payload format: [destIp:4][destPort:2][srcIp:4][srcPort:2][data:N]
    //
    uint addressedLen = 4 + 2 + 4 + 2 + dataLength;
    vector<byte> addressedPayload(addressedLen);
    uint off = 0;

    memcpy(addressedPayload.data() + off, &destIpAddress, 4);
    off += 4;
    memcpy(addressedPayload.data() + off, &destPort, 2);
    off += 2;

    // Encode our local IP/port as source so receiver can reply
    // (these are set from localIpAddress_/localPort_ via the transport)
    ulong srcIp = 0;  // will be filled by transport layer
    ushort srcPort = 0;
    memcpy(addressedPayload.data() + off, &srcIp, 4);
    off += 4;
    memcpy(addressedPayload.data() + off, &srcPort, 2);
    off += 2;
    memcpy(addressedPayload.data() + off, sendBuffer, dataLength);

    // Build frame: broadcast destination MAC
    byte destMac[6];
    memset(destMac, 0xFF, 6);

    byte frame[4096];
    uint frameLen = buildDataFrame(addressedPayload.data(), addressedLen, destMac, frame, sizeof(frame));

    if (frameLen == 0) {
        Log::Error("RawWifiConnection: Failed to build data frame.");
        return false;
    }

    ssize_t sent = send(rawFd_, frame, frameLen, 0);

    if (sent < 0) {
        Log::Error("RawWifiConnection: Failed to send frame.");
        return false;
    }

    return true;
}


bool
RawWifiConnection::receiveData(
    byte * dataBuffer, const uint bufferLength, ulong & sourceIpAddress, ushort & sourcePort, uint & dataLength)
{
    if (rawFd_ < 0) {
        return false;
    }

    byte frame[4096];
    ssize_t bytesRead = recv(rawFd_, frame, sizeof(frame), 0);

    if (bytesRead < 0) {
        if (errno == EAGAIN) {
            return false;
        }
        Log::Error("RawWifiConnection: recv() failed.");
        return false;
    }

    if (static_cast<uint>(bytesRead) < FRAME_OVERHEAD) {
        return false;
    }

    // Parse the frame and extract payload
    byte payload[4096];
    uint payloadLen = 0;
    ulong srcIp = 0;
    ushort srcPort = 0;

    if (!parseDataFrame(frame, static_cast<uint>(bytesRead), payload, payloadLen, srcIp, srcPort)) {
        return false;
    }

    // The payload contains: [destIp:4][destPort:2][srcIp:4][srcPort:2][data:N]
    if (payloadLen < 12) {
        return false;
    }

    // Skip destIp/destPort, extract source addressing
    uint off = 4 + 2;  // skip dest
    memcpy(&sourceIpAddress, payload + off, 4);
    off += 4;
    memcpy(&sourcePort, payload + off, 2);
    off += 2;

    uint actualDataLen = payloadLen - off;
    if (actualDataLen > bufferLength) {
        actualDataLen = bufferLength;
    }

    memcpy(dataBuffer, payload + off, actualDataLen);
    dataLength = actualDataLen;

    if (CovertChannel::isEnabled()) {
        CovertChannel::deobfuscate(dataBuffer, dataLength);
    }

    return true;
}


uint
RawWifiConnection::buildDataFrame(
    const byte * payload, uint payloadLen, const byte * destMac, byte * frame, uint frameSize)
{
    uint totalLen = FRAME_OVERHEAD + payloadLen;

    if (totalLen > frameSize) {
        return 0;
    }

    uint offset = 0;

    // --- Radiotap header (8 bytes, minimal) ---
    frame[offset++] = 0x00;  // version
    frame[offset++] = 0x00;  // pad
    frame[offset++] = 0x08;  // length (LE)
    frame[offset++] = 0x00;
    frame[offset++] = 0x00;  // present flags (none)
    frame[offset++] = 0x00;
    frame[offset++] = 0x00;
    frame[offset++] = 0x00;

    // --- 802.11 MAC header (24 bytes) ---
    // Frame Control: data frame (0x0008)
    frame[offset++] = 0x08;
    frame[offset++] = 0x00;
    // Duration
    frame[offset++] = 0x00;
    frame[offset++] = 0x00;
    // Address 1 (Destination)
    memcpy(frame + offset, destMac, 6);
    offset += 6;
    // Address 2 (Source: local MAC)
    memcpy(frame + offset, localMac_, 6);
    offset += 6;
    // Address 3 (BSSID: broadcast for ad-hoc)
    memset(frame + offset, 0xFF, 6);
    offset += 6;
    // Sequence control
    frame[offset++] = 0x00;
    frame[offset++] = 0x00;

    // --- LLC/SNAP header (8 bytes) ---
    frame[offset++] = 0xAA;  // DSAP
    frame[offset++] = 0xAA;  // SSAP
    frame[offset++] = 0x03;  // Control
    // OUI: Alpine
    frame[offset++] = ALPINE_OUI[0];
    frame[offset++] = ALPINE_OUI[1];
    frame[offset++] = ALPINE_OUI[2];
    // EtherType: Alpine DTCP
    frame[offset++] = static_cast<byte>((ALPINE_ETHERTYPE >> 8) & 0xFF);
    frame[offset++] = static_cast<byte>(ALPINE_ETHERTYPE & 0xFF);

    // --- Payload ---
    memcpy(frame + offset, payload, payloadLen);
    offset += payloadLen;

    return offset;
}


bool
RawWifiConnection::parseDataFrame(
    const byte * frame, uint frameLen, byte * payload, uint & payloadLen, ulong & sourceIp, ushort & sourcePort)
{
    if (frameLen < FRAME_OVERHEAD) {
        return false;
    }

    // Read radiotap header length
    uint rtLen = frame[2] | (frame[3] << 8);
    if (rtLen >= frameLen) {
        return false;
    }

    uint macOffset = rtLen;
    if (macOffset + DOT11_HDR_SIZE > frameLen) {
        return false;
    }

    // Check frame control for data frame (type 0x08 or 0x88 for QoS data)
    byte frameType = frame[macOffset] & 0x0C;
    if (frameType != 0x08) {
        return false;
    }

    // Skip to LLC/SNAP header
    uint llcOffset = macOffset + DOT11_HDR_SIZE;
    if (llcOffset + LLC_SNAP_HDR_SIZE > frameLen) {
        return false;
    }

    // Check LLC/SNAP: DSAP=0xAA, SSAP=0xAA, Control=0x03
    if (frame[llcOffset] != 0xAA || frame[llcOffset + 1] != 0xAA || frame[llcOffset + 2] != 0x03) {
        return false;
    }

    // Check OUI matches Alpine
    if (frame[llcOffset + 3] != ALPINE_OUI[0] || frame[llcOffset + 4] != ALPINE_OUI[1] ||
        frame[llcOffset + 5] != ALPINE_OUI[2]) {
        return false;
    }

    // Check EtherType
    ushort etherType = (frame[llcOffset + 6] << 8) | frame[llcOffset + 7];
    if (etherType != ALPINE_ETHERTYPE) {
        return false;
    }

    // Extract payload
    uint dataOffset = llcOffset + LLC_SNAP_HDR_SIZE;
    payloadLen = frameLen - dataOffset;

    if (payloadLen > 0) {
        memcpy(payload, frame + dataOffset, payloadLen);
    }

    // Source IP/port will be extracted from the addressed payload by caller
    sourceIp = 0;
    sourcePort = 0;

    return true;
}


#else  // Non-Linux stubs


RawWifiConnection::RawWifiConnection(const string & interfaceName)
    : interfaceName_(interfaceName)
{}


RawWifiConnection::~RawWifiConnection() {}


bool
RawWifiConnection::create(ulong ipAddress, ushort port)
{
    Log::Error("RawWifiConnection: Raw 802.11 is only supported on Linux.");
    return false;
}


void
RawWifiConnection::close()
{}


int
RawWifiConnection::getFd()
{
    return -1;
}


bool
RawWifiConnection::sendData(const ulong destIpAddress,
                            const ushort destPort,
                            const byte * dataBuffer,
                            const uint dataLength)
{
    return false;
}


bool
RawWifiConnection::receiveData(
    byte * dataBuffer, const uint bufferLength, ulong & sourceIpAddress, ushort & sourcePort, uint & dataLength)
{
    return false;
}


#endif
