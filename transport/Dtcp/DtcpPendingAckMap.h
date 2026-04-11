/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <OptHash.h>
#include <Platform.h>
#include <chrono>
#include <queue>
#include <random>


class DataBlock;
class DtcpBaseUdpTransport;
class DtcpBaseConnTransport;


class DtcpPendingAckMap
{
  public:
    DtcpPendingAckMap(DtcpBaseUdpTransport * udpTransport);

    ~DtcpPendingAckMap();


    bool add(DtcpBaseConnTransport * requestor, DataBlock * data, ulong destIpAddress, ushort destPort, ulong & id);

    bool remove(ulong id);

    bool processTimers();


    // Types
    //
    using t_Clock = std::chrono::steady_clock;
    using t_TimePoint = t_Clock::time_point;
    using t_Duration = t_Clock::duration;

    struct t_PendingRecord
    {
        ulong id;
        DtcpBaseConnTransport * requestor;
        DataBlock * data;
        ulong destIpAddress;
        ushort destPort;
        t_TimePoint sendTime;
        t_TimePoint nextRetryTime;
        ushort retryCount;
        t_Duration currentRto;
    };

    // Priority queue entry: ordered by next retry time (earliest first)
    //
    struct t_RetryEntry
    {
        ulong id;
        t_TimePoint nextRetryTime;

        bool
        operator>(const t_RetryEntry & other) const
        {
            return nextRetryTime > other.nextRetryTime;
        }
    };

    using t_RetryQueue = std::priority_queue<t_RetryEntry, std::vector<t_RetryEntry>, std::greater<t_RetryEntry>>;

    using t_RecordIndex = std::unordered_map<ulong, t_PendingRecord *, OptHash<ulong>, equal_to<ulong>>;

    using t_RecordIndexPair = std::pair<ulong, t_PendingRecord *>;


    // Per-peer RTT estimation (TCP-style EWMA)
    //
    struct t_PeerRtt
    {
        double srtt = 0.0;    // smoothed RTT in milliseconds
        double rttvar = 0.0;  // RTT variance in milliseconds
        bool initialized = false;
    };

    using t_PeerRttIndex = std::unordered_map<ulong, t_PeerRtt, OptHash<ulong>, equal_to<ulong>>;


  private:
    DtcpBaseUdpTransport * udpTransport_;

    t_RecordIndex * recordIndex_;
    t_RetryQueue retryQueue_;
    t_PeerRttIndex peerRttIndex_;
    ulong currId_;

    ushort maxRetries_;

    std::mt19937 rng_;


    t_Duration computeRto(ulong destIpAddress) const;

    t_Duration applyJitter(t_Duration base);

    void updatePeerRtt(ulong destIpAddress, double sampleMs);

    static constexpr double kSrttAlpha = 0.125;
    static constexpr double kSrttBeta = 0.25;
    static constexpr double kDefaultRtoMs = 200.0;
    static constexpr double kMaxRtoMs = 30000.0;
    static constexpr double kJitterFraction = 0.25;
    static constexpr ushort kDefaultMaxRetries = 8;
};
