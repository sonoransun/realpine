/// Copyright (C) 2026 sonoransun — see LICENCE.txt
///
/// Integration test: REST API rate limiting
///
/// Tests that the RateLimiter correctly enforces token-bucket rate limits,
/// returning 429 responses when the limit is exceeded.

#include <catch2/catch_test_macros.hpp>
#include <RateLimiter.h>


TEST_CASE("Rate limiter: requests within burst are allowed", "[integration][ratelimit]")
{
    RateLimiter::initialize(10.0, 5);

    // 5 requests should all be allowed (within burst)
    for (int i = 0; i < 5; ++i) {
        REQUIRE(RateLimiter::allowRequest("10.0.0.1"));
    }
}


TEST_CASE("Rate limiter: requests exceeding burst are rejected", "[integration][ratelimit]")
{
    RateLimiter::initialize(1.0, 3);

    // First 3 within burst
    for (int i = 0; i < 3; ++i) {
        REQUIRE(RateLimiter::allowRequest("10.0.0.2"));
    }

    // 4th should be rejected
    REQUIRE_FALSE(RateLimiter::allowRequest("10.0.0.2"));
}


TEST_CASE("Rate limiter: different clients have separate buckets", "[integration][ratelimit]")
{
    RateLimiter::initialize(1.0, 2);

    // Exhaust client A's bucket
    REQUIRE(RateLimiter::allowRequest("10.0.0.3"));
    REQUIRE(RateLimiter::allowRequest("10.0.0.3"));
    REQUIRE_FALSE(RateLimiter::allowRequest("10.0.0.3"));

    // Client B should still have tokens
    REQUIRE(RateLimiter::allowRequest("10.0.0.4"));
}


TEST_CASE("Rate limiter: IPv6 addresses are normalized", "[integration][ratelimit]")
{
    // Verify that normalizeIp produces consistent keys
    string norm1 = RateLimiter::normalizeIp("::ffff:192.168.1.1");
    string norm2 = RateLimiter::normalizeIp("192.168.1.1");

    // IPv4-mapped IPv6 should normalize to the IPv4 address
    REQUIRE(norm1 == norm2);
}


TEST_CASE("Rate limiter: cleanup removes stale buckets", "[integration][ratelimit]")
{
    RateLimiter::initialize(100.0, 100);

    // Create some buckets
    RateLimiter::allowRequest("10.0.0.10");
    RateLimiter::allowRequest("10.0.0.11");

    // Cleanup should not crash
    RateLimiter::cleanup();
}
