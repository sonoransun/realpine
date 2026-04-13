/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <UdpConnection.h>

#include <atomic>
#include <deque>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>


class SdrDemodulator;


class RtlSdrConnection : public UdpConnection
{
  public:
    RtlSdrConnection(uint centerFreqHz, uint sampleRate, int gainTenths, std::unique_ptr<SdrDemodulator> demodulator);

    ~RtlSdrConnection() override;

    bool create(ulong ipAddress = 0, ushort port = 0) override;
    void close() override;
    int getFd() override;

    bool
    sendData(const ulong destIpAddress, const ushort destPort, const byte * dataBuffer, const uint dataLength) override;

    bool receiveData(byte * dataBuffer,
                     const uint bufferLength,
                     ulong & sourceIpAddress,
                     ushort & sourcePort,
                     uint & dataLength) override;

  private:
#ifdef __linux__
    void * rtlDev_;
    int pipeFds_[2];

    std::thread asyncThread_;
    std::atomic<bool> running_;

    struct PacketEntry
    {
        vector<byte> data;
        ulong sourceIp;
        ushort sourcePort;
    };

    std::deque<PacketEntry> packetQueue_;
    std::mutex queueMutex_;
    static constexpr uint MAX_QUEUE_SIZE = 1024;

    std::unique_ptr<SdrDemodulator> demodulator_;

    uint centerFreqHz_;
    uint sampleRate_;
    int gainTenths_;

#ifdef ALPINE_RTLSDR_ENABLED
    static void rtlsdrCallback(unsigned char * buf, uint32_t len, void * ctx);
#endif
    void processIqSamples(const int8_t * samples, uint numSamples);
    void asyncReaderThread();
#else
    std::unique_ptr<SdrDemodulator> demodulator_;
    uint centerFreqHz_;
    uint sampleRate_;
    int gainTenths_;
#endif
};
