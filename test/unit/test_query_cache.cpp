/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <catch2/catch_test_macros.hpp>
#include <QueryCache.h>
#include <thread>
#include <chrono>


TEST_CASE("QueryCache basic operations", "[QueryCache]")
{
    QueryCache::clear();
    QueryCache::configure(100, 300);

    SECTION("Store and lookup returns cached result")
    {
        QueryCache::store("key1", "result1");
        auto result = QueryCache::lookup("key1");
        REQUIRE(result.has_value());
        REQUIRE(*result == "result1");
    }

    SECTION("Lookup miss returns nullopt")
    {
        auto result = QueryCache::lookup("nonexistent");
        REQUIRE_FALSE(result.has_value());
    }

    SECTION("Store overwrites existing entry")
    {
        QueryCache::store("k", "v1");
        QueryCache::store("k", "v2");
        auto result = QueryCache::lookup("k");
        REQUIRE(result.has_value());
        REQUIRE(*result == "v2");
        REQUIRE(QueryCache::size() == 1);
    }

    SECTION("Invalidate removes entry")
    {
        QueryCache::store("k", "v");
        QueryCache::invalidate("k");
        auto result = QueryCache::lookup("k");
        REQUIRE_FALSE(result.has_value());
    }

    SECTION("Clear empties cache")
    {
        QueryCache::store("a", "1");
        QueryCache::store("b", "2");
        QueryCache::store("c", "3");
        REQUIRE(QueryCache::size() == 3);
        QueryCache::clear();
        REQUIRE(QueryCache::size() == 0);
    }
}


TEST_CASE("QueryCache TTL expiration", "[QueryCache]")
{
    QueryCache::clear();
    QueryCache::configure(100, 1);  // 1-second TTL

    QueryCache::store("ttl-key", "ttl-value");
    auto result = QueryCache::lookup("ttl-key");
    REQUIRE(result.has_value());

    std::this_thread::sleep_for(std::chrono::milliseconds(1200));

    result = QueryCache::lookup("ttl-key");
    REQUIRE_FALSE(result.has_value());
}


TEST_CASE("QueryCache LRU eviction", "[QueryCache]")
{
    QueryCache::clear();
    QueryCache::configure(3, 300);  // capacity 3

    QueryCache::store("a", "1");
    QueryCache::store("b", "2");
    QueryCache::store("c", "3");
    REQUIRE(QueryCache::size() == 3);

    SECTION("Evicts least recently used on overflow")
    {
        QueryCache::store("d", "4");  // should evict "a"
        REQUIRE_FALSE(QueryCache::lookup("a").has_value());
        REQUIRE(QueryCache::lookup("b").has_value());
        REQUIRE(QueryCache::lookup("c").has_value());
        REQUIRE(QueryCache::lookup("d").has_value());
    }

    SECTION("Lookup promotes to MRU")
    {
        QueryCache::lookup("a");       // promote "a" to most-recently-used
        QueryCache::store("d", "4");   // should evict "b" (now LRU)
        REQUIRE(QueryCache::lookup("a").has_value());
        REQUIRE_FALSE(QueryCache::lookup("b").has_value());
        REQUIRE(QueryCache::lookup("c").has_value());
        REQUIRE(QueryCache::lookup("d").has_value());
    }
}
