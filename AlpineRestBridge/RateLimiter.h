/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <unordered_map>
#include <mutex>
#include <chrono>


class RateLimiter
{
  public:

    static void  initialize (double  requestsPerSecond,
                             uint    burstSize);

    static bool  allowRequest (const string & clientIp);

    static void  cleanup ();


  private:

    struct t_TokenBucket {
        double  tokens;
        std::chrono::steady_clock::time_point  lastRefill;
    };

    static void  refillBucket (t_TokenBucket & bucket);

    static double                                         rate_s;
    static uint                                           burst_s;
    static std::unordered_map<string, t_TokenBucket>      buckets_s;
    static std::mutex                                     mutex_s;
    static bool                                           initialized_s;

    static constexpr auto STALE_TIMEOUT = std::chrono::minutes(5);

};
