/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <CovertChannel.h>
#include <Log.h>
#include <NetUtils.h>
#include <UnicastWifiConnection.h>

#include <Platform.h>
#include <cstring>


#ifdef __linux__
#include <fstream>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sstream>
#include <sys/ioctl.h>
#include <unistd.h>
#endif


#ifdef __linux__


UnicastWifiConnection::UnicastWifiConnection(const string & interfaceName)
    : RawWifiConnection(interfaceName)
{
#ifdef _VERBOSE
    Log::Debug("UnicastWifiConnection constructor invoked.");
#endif
}


UnicastWifiConnection::~UnicastWifiConnection()
{
#ifdef _VERBOSE
    Log::Debug("UnicastWifiConnection destructor invoked.");
#endif
}


bool
UnicastWifiConnection::sendData(const ulong destIpAddress,
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

    // Build addressed payload: [destIp:4][destPort:2][srcIp:4][srcPort:2][data:N]
    uint addressedLen = 4 + 2 + 4 + 2 + dataLength;
    vector<byte> addressedPayload(addressedLen);
    uint off = 0;

    memcpy(addressedPayload.data() + off, &destIpAddress, 4);
    off += 4;
    memcpy(addressedPayload.data() + off, &destPort, 2);
    off += 2;

    ulong srcIp = 0;
    ushort srcPort = 0;
    memcpy(addressedPayload.data() + off, &srcIp, 4);
    off += 4;
    memcpy(addressedPayload.data() + off, &srcPort, 2);
    off += 2;
    memcpy(addressedPayload.data() + off, sendBuffer, dataLength);

    // Resolve destination MAC — prefer cache, fall back to ARP, then broadcast
    byte destMac[6];
    bool macResolved = false;

    {
        std::lock_guard lock(macMutex_);

        auto it = macCache_.find(destIpAddress);

        if (it != macCache_.end()) {
            auto expiryIt = macCacheExpiry_.find(destIpAddress);

            if (expiryIt != macCacheExpiry_.end() && std::chrono::steady_clock::now() < expiryIt->second) {
                memcpy(destMac, it->second.data(), 6);
                macResolved = true;
            } else {
                // Expired entry — remove
                macCache_.erase(it);

                if (expiryIt != macCacheExpiry_.end()) {
                    macCacheExpiry_.erase(expiryIt);
                }
            }
        }
    }

    if (!macResolved) {
        macResolved = lookupArpTable(destIpAddress, destMac);

        if (macResolved) {
            std::lock_guard lock(macMutex_);

            MacAddr mac;
            memcpy(mac.data(), destMac, 6);
            macCache_[destIpAddress] = mac;
            macCacheExpiry_[destIpAddress] =
                std::chrono::steady_clock::now() + std::chrono::seconds(MAC_CACHE_TTL_SECONDS);
        }
    }

    if (!macResolved) {
        // Graceful degradation: fall back to broadcast
        memset(destMac, 0xFF, 6);
    }

    byte frame[4096];
    uint frameLen = buildDataFrame(addressedPayload.data(), addressedLen, destMac, frame, sizeof(frame));

    if (frameLen == 0) {
        Log::Error("UnicastWifiConnection: Failed to build data frame.");
        return false;
    }

    ssize_t sent = send(rawFd_, frame, frameLen, 0);

    if (sent < 0) {
        Log::Error("UnicastWifiConnection: Failed to send frame.");
        return false;
    }

    return true;
}


bool
UnicastWifiConnection::receiveData(
    byte * dataBuffer, const uint bufferLength, ulong & sourceIpAddress, ushort & sourcePort, uint & dataLength)
{
    if (rawFd_ < 0) {
        return false;
    }

    // Read the raw frame to extract the source MAC before parsing
    byte frame[4096];
    ssize_t bytesRead = recv(rawFd_, frame, sizeof(frame), 0);

    if (bytesRead < 0) {
        if (errno == EAGAIN) {
            return false;
        }
        Log::Error("UnicastWifiConnection: recv() failed.");
        return false;
    }

    if (static_cast<uint>(bytesRead) < FRAME_OVERHEAD) {
        return false;
    }

    // Parse frame
    byte payload[4096];
    uint payloadLen = 0;
    ulong srcIp = 0;
    ushort srcPort = 0;

    if (!parseDataFrame(frame, static_cast<uint>(bytesRead), payload, payloadLen, srcIp, srcPort)) {
        return false;
    }

    if (payloadLen < 12) {
        return false;
    }

    // Extract source addressing from payload prefix
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

    // Cache the source MAC from the 802.11 Address 2 field
    // Radiotap length is at bytes 2-3 (LE), Address 2 starts at radiotapLen + 10
    if (sourceIpAddress != 0) {
        uint rtLen = frame[2] | (frame[3] << 8);

        if (rtLen + DOT11_HDR_SIZE <= static_cast<uint>(bytesRead)) {
            std::lock_guard lock(macMutex_);

            MacAddr mac;
            memcpy(mac.data(), frame + rtLen + 10, 6);
            macCache_[sourceIpAddress] = mac;
            macCacheExpiry_[sourceIpAddress] =
                std::chrono::steady_clock::now() + std::chrono::seconds(MAC_CACHE_TTL_SECONDS);
        }
    }

    return true;
}


bool
UnicastWifiConnection::setDestinationMac(ulong ipAddress, const byte mac[6])
{
    if (ipAddress == 0) {
        return false;
    }

    std::lock_guard lock(macMutex_);

    MacAddr addr;
    memcpy(addr.data(), mac, 6);
    macCache_[ipAddress] = addr;
    macCacheExpiry_[ipAddress] = std::chrono::steady_clock::now() + std::chrono::seconds(MAC_CACHE_TTL_SECONDS);

    return true;
}


bool
UnicastWifiConnection::resolveDestinationMac(ulong ipAddress, byte mac[6])
{
    std::lock_guard lock(macMutex_);

    auto it = macCache_.find(ipAddress);

    if (it == macCache_.end()) {
        return false;
    }

    auto expiryIt = macCacheExpiry_.find(ipAddress);

    if (expiryIt != macCacheExpiry_.end() && std::chrono::steady_clock::now() >= expiryIt->second) {
        macCache_.erase(it);
        macCacheExpiry_.erase(expiryIt);
        return false;
    }

    memcpy(mac, it->second.data(), 6);

    return true;
}


bool
UnicastWifiConnection::lookupArpTable(ulong ipAddress, byte mac[6])
{
    string targetIp;
    NetUtils::longIpToString(ipAddress, targetIp);

    std::ifstream arpFile("/proc/net/arp");

    if (!arpFile.is_open()) {
        return false;
    }

    string line;
    std::getline(arpFile, line);  // skip header

    while (std::getline(arpFile, line)) {
        std::istringstream iss(line);
        string ip, hwType, flags, hwAddr;
        iss >> ip >> hwType >> flags >> hwAddr;

        if (ip != targetIp) {
            continue;
        }

        // Parse MAC address "aa:bb:cc:dd:ee:ff"
        if (hwAddr.size() != 17) {
            return false;
        }

        unsigned int octets[6];
        int parsed = sscanf(hwAddr.c_str(),
                            "%x:%x:%x:%x:%x:%x",
                            &octets[0],
                            &octets[1],
                            &octets[2],
                            &octets[3],
                            &octets[4],
                            &octets[5]);

        if (parsed != 6) {
            return false;
        }

        for (int i = 0; i < 6; ++i) {
            mac[i] = static_cast<byte>(octets[i]);
        }

        return true;
    }

    return false;
}


#else  // Non-Linux stubs


UnicastWifiConnection::UnicastWifiConnection(const string & interfaceName)
    : RawWifiConnection(interfaceName)
{}


UnicastWifiConnection::~UnicastWifiConnection() {}


bool
UnicastWifiConnection::sendData(const ulong destIpAddress,
                                const ushort destPort,
                                const byte * dataBuffer,
                                const uint dataLength)
{
    return false;
}


bool
UnicastWifiConnection::receiveData(
    byte * dataBuffer, const uint bufferLength, ulong & sourceIpAddress, ushort & sourcePort, uint & dataLength)
{
    return false;
}


bool
UnicastWifiConnection::setDestinationMac(ulong ipAddress, const byte mac[6])
{
    return false;
}


bool
UnicastWifiConnection::resolveDestinationMac(ulong ipAddress, byte mac[6])
{
    return false;
}


#endif
