/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <RawWifiConnection.h>

#include <array>
#include <chrono>
#include <mutex>
#include <unordered_map>


class UnicastWifiConnection : public RawWifiConnection
{
  public:
    UnicastWifiConnection(const string & interfaceName);
    ~UnicastWifiConnection() override;

    bool
    sendData(const ulong destIpAddress, const ushort destPort, const byte * dataBuffer, const uint dataLength) override;

    bool receiveData(byte * dataBuffer,
                     const uint bufferLength,
                     ulong & sourceIpAddress,
                     ushort & sourcePort,
                     uint & dataLength) override;

    // Allow external callers (e.g. discovery system) to pre-populate the MAC cache.
    bool setDestinationMac(ulong ipAddress, const byte mac[6]);

    // Query the cache for a resolved MAC.  Returns false if not cached.
    bool resolveDestinationMac(ulong ipAddress, byte mac[6]);

  private:
#ifdef __linux__
    using MacAddr = std::array<byte, 6>;
    using TimePoint = std::chrono::steady_clock::time_point;

    std::unordered_map<ulong, MacAddr> macCache_;
    std::unordered_map<ulong, TimePoint> macCacheExpiry_;
    std::mutex macMutex_;

    static constexpr int MAC_CACHE_TTL_SECONDS = 300;

    bool lookupArpTable(ulong ipAddress, byte mac[6]);
    void cacheSourceMac(const byte * frame, uint frameLen, ulong sourceIp);
#endif
};
