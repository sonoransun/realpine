/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <RateLimiter.h>
#include <Log.h>


double                                         RateLimiter::rate_s         = 10.0;
uint                                           RateLimiter::burst_s        = 20;
std::array<RateLimiter::t_Shard, RateLimiter::SHARD_COUNT>  RateLimiter::shards_s;
bool                                           RateLimiter::initialized_s  = false;



size_t
RateLimiter::shardIndex (const string & clientIp)
{
    return std::hash<string>{}(clientIp) % SHARD_COUNT;
}



void
RateLimiter::initialize (double  requestsPerSecond,
                         uint    burstSize)
{
    rate_s = requestsPerSecond;
    burst_s = burstSize;
    initialized_s = true;
    Log::Info("RateLimiter initialized: "s +
              std::to_string(requestsPerSecond) + " req/s, burst " +
              std::to_string(burstSize));
}



bool
RateLimiter::allowRequest (const string & clientIp)
{
    if (!initialized_s)
        return true;

    auto & shard = shards_s[shardIndex(clientIp)];
    auto now = std::chrono::steady_clock::now();

    std::unique_lock lock(shard.mutex);

    auto it = shard.buckets.find(clientIp);

    if (it == shard.buckets.end()) {
        shard.buckets[clientIp] = t_TokenBucket{
            static_cast<double>(burst_s) - 1.0,
            now
        };
        return true;
    }

    auto & bucket = it->second;
    refillBucket(bucket);

    if (bucket.tokens >= 1.0) {
        bucket.tokens -= 1.0;
        return true;
    }

    return false;
}



void
RateLimiter::refillBucket (t_TokenBucket & bucket)
{
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration<double>(now - bucket.lastRefill).count();

    bucket.tokens += elapsed * rate_s;
    if (bucket.tokens > static_cast<double>(burst_s))
        bucket.tokens = static_cast<double>(burst_s);

    bucket.lastRefill = now;
}



void
RateLimiter::cleanup ()
{
    auto now = std::chrono::steady_clock::now();

    for (auto & shard : shards_s) {
        std::unique_lock lock(shard.mutex);

        auto it = shard.buckets.begin();
        while (it != shard.buckets.end()) {
            if (now - it->second.lastRefill > STALE_TIMEOUT)
                it = shard.buckets.erase(it);
            else
                ++it;
        }
    }
}
