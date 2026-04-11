/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <DtcpBaseConnConnector.h>
#include <DtcpBaseConnTransport.h>
#include <OptHash.h>
#include <ReadWriteSem.h>
#include <chrono>
#include <random>
#include <unordered_map>


class AlpineDtcpConnConnector : public DtcpBaseConnConnector
{
  public:
    AlpineDtcpConnConnector();
    virtual ~AlpineDtcpConnConnector();


    virtual bool createTransport(DtcpBaseConnTransport *& transport);

    virtual bool receiveTransport(DtcpBaseConnTransport * transport);

    virtual bool handleRequestFailure(ulong ipAddress, ushort port);

    /// Returns true if backoff has expired and a reconnect attempt is allowed.
    /// Also resets backoff on success path via resetBackoff().
    ///
    static bool allowReconnect(ulong peerId);

    /// Reset the backoff state for a peer after a successful connection.
    ///
    static void resetBackoff(ulong peerId);


  private:
    static constexpr double INITIAL_BACKOFF_SECS = 1.0;
    static constexpr double MAX_BACKOFF_SECS = 60.0;
    static constexpr double BACKOFF_MULTIPLIER = 2.0;
    static constexpr double JITTER_FACTOR = 0.25;

    using Clock = std::chrono::steady_clock;
    using TimePoint = Clock::time_point;

    struct BackoffState
    {
        double currentDelaySecs = INITIAL_BACKOFF_SECS;
        TimePoint nextAllowedAt{};
        bool active = false;
    };

    using t_BackoffIndex = std::unordered_map<ulong, BackoffState, OptHash<ulong>, std::equal_to<ulong>>;

    static t_BackoffIndex backoffIndex_s;
    static ReadWriteSem backoffLock_s;
    static std::mt19937 rng_s;

    static double applyJitter(double delaySecs);
};
