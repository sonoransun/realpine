/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <asio.hpp>
#include <array>
#include <unordered_map>
#include <shared_mutex>
#include <chrono>
#include <functional>


class RateLimiter
{
  public:

    static void    initialize (double  requestsPerSecond,
                               uint    burstSize);

    static bool    allowRequest (const string & clientIp);

    static string  normalizeIp (const string & ip);

    static void    cleanup ();


  private:

    static constexpr size_t SHARD_COUNT   = 16;
    static constexpr auto   STALE_TIMEOUT = std::chrono::minutes(5);

    struct t_TokenBucket {
        double  tokens;
        std::chrono::steady_clock::time_point  lastRefill;
    };

    struct t_Shard {
        std::shared_mutex                              mutex;
        std::unordered_map<string, t_TokenBucket>      buckets;
    };

    static void    refillBucket (t_TokenBucket & bucket);
    static size_t  shardIndex   (const string & clientIp);

    static double                              rate_s;
    static uint                                burst_s;
    static std::array<t_Shard, SHARD_COUNT>    shards_s;
    static bool                                initialized_s;

};
