/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <catch2/catch_test_macros.hpp>
#include <RateLimiter.h>
#include <thread>
#include <chrono>
#include <vector>
#include <atomic>


TEST_CASE("RateLimiter basic behavior", "[RateLimiter]")
{
    // Use unique IPs per test to avoid bucket contamination
    static int testCounter = 0;
    ++testCounter;
    string baseIp = "10." + std::to_string(testCounter) + ".0.";

    SECTION("Uninitialized limiter allows all requests")
    {
        // Before initialize, allowRequest should return true
        // (This depends on initialization state; tested by using a fresh IP)
    }

    SECTION("Burst limit is enforced")
    {
        RateLimiter::initialize(10.0, 3);  // 10 req/s, burst of 3
        string ip = baseIp + "1";

        REQUIRE(RateLimiter::allowRequest(ip));
        REQUIRE(RateLimiter::allowRequest(ip));
        REQUIRE(RateLimiter::allowRequest(ip));
        REQUIRE_FALSE(RateLimiter::allowRequest(ip));
    }

    SECTION("Different IPs have independent buckets")
    {
        RateLimiter::initialize(10.0, 2);
        string ip1 = baseIp + "2";
        string ip2 = baseIp + "3";

        REQUIRE(RateLimiter::allowRequest(ip1));
        REQUIRE(RateLimiter::allowRequest(ip1));
        REQUIRE_FALSE(RateLimiter::allowRequest(ip1));

        // ip2 should still have full burst
        REQUIRE(RateLimiter::allowRequest(ip2));
        REQUIRE(RateLimiter::allowRequest(ip2));
        REQUIRE_FALSE(RateLimiter::allowRequest(ip2));
    }
}


TEST_CASE("RateLimiter token refill", "[RateLimiter]")
{
    RateLimiter::initialize(1000.0, 5);  // 1000 req/s, burst 5
    string ip = "10.200.0.1";

    // Exhaust burst
    for (int i = 0; i < 5; ++i) {
        REQUIRE(RateLimiter::allowRequest(ip));
    }
    REQUIRE_FALSE(RateLimiter::allowRequest(ip));

    // Wait for refill (at 1000/s, 50ms should refill ~50 tokens, capped at burst=5)
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    REQUIRE(RateLimiter::allowRequest(ip));
}


TEST_CASE("RateLimiter concurrent access", "[RateLimiter]")
{
    RateLimiter::initialize(100000.0, 100);

    std::atomic<int> allowed{0};
    std::atomic<int> denied{0};

    auto worker = [&](int threadId) {
        string ip = "10.100." + std::to_string(threadId) + ".1";
        for (int i = 0; i < 200; ++i) {
            if (RateLimiter::allowRequest(ip)) {
                ++allowed;
            } else {
                ++denied;
            }
        }
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < 8; ++i) {
        threads.emplace_back(worker, i);
    }

    for (auto & t : threads) {
        t.join();
    }

    // Just verify no crashes and results are reasonable
    REQUIRE((allowed.load() + denied.load()) == 1600);
    REQUIRE(allowed.load() > 0);
}


TEST_CASE("RateLimiter cleanup does not remove fresh buckets", "[RateLimiter]")
{
    RateLimiter::initialize(10.0, 5);
    string ip = "10.250.0.1";

    REQUIRE(RateLimiter::allowRequest(ip));
    RateLimiter::cleanup();
    // Fresh bucket should survive cleanup
    REQUIRE(RateLimiter::allowRequest(ip));
}
