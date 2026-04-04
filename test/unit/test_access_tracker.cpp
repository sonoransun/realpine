/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <catch2/catch_test_macros.hpp>
#include <AccessTracker.h>
#include <nlohmann/json.hpp>


// Use high base IDs to avoid collisions with other test runs
static constexpr ulong BASE_RESOURCE = 100000;
static constexpr ulong BASE_PEER     = 200000;


TEST_CASE("AccessTracker record and retrieve resource stats", "[AccessTracker]")
{
    AccessTracker::reset();

    ulong rid = BASE_RESOURCE + 1;
    ulong pid = BASE_PEER + 1;

    AccessTracker::recordResourceAccess(rid, pid, "test.mp3", 1024);

    AccessTracker::t_ResourceStats stats{};
    REQUIRE(AccessTracker::getResourceStats(rid, stats));
    REQUIRE(stats.accessCount == 1);
    REQUIRE(stats.totalBytesRead == 1024);
    REQUIRE(stats.description == "test.mp3");
}


TEST_CASE("AccessTracker multiple accesses accumulate", "[AccessTracker]")
{
    AccessTracker::reset();

    ulong rid = BASE_RESOURCE + 2;
    ulong pid = BASE_PEER + 2;

    AccessTracker::recordResourceAccess(rid, pid, "file.bin", 100);
    AccessTracker::recordResourceAccess(rid, pid, "file.bin", 200);
    AccessTracker::recordResourceAccess(rid, pid, "file.bin", 300);

    AccessTracker::t_ResourceStats stats{};
    REQUIRE(AccessTracker::getResourceStats(rid, stats));
    REQUIRE(stats.accessCount == 3);
    REQUIRE(stats.totalBytesRead == 600);
}


TEST_CASE("AccessTracker peer stats aggregate across resources", "[AccessTracker]")
{
    AccessTracker::reset();

    ulong rid1 = BASE_RESOURCE + 10;
    ulong rid2 = BASE_RESOURCE + 11;
    ulong pid  = BASE_PEER + 10;

    AccessTracker::recordResourceAccess(rid1, pid, "a.mp3", 500);
    AccessTracker::recordResourceAccess(rid2, pid, "b.mp3", 700);

    AccessTracker::t_PeerStats ps{};
    REQUIRE(AccessTracker::getPeerStats(pid, ps));
    REQUIRE(ps.aggregateAccessCount == 2);
    REQUIRE(ps.distinctResourcesAccessed == 2);
    REQUIRE(ps.aggregateBytesRead == 1200);
}


TEST_CASE("AccessTracker unknown resource returns false", "[AccessTracker]")
{
    AccessTracker::reset();

    AccessTracker::t_ResourceStats stats{};
    REQUIRE_FALSE(AccessTracker::getResourceStats(999999, stats));
}


TEST_CASE("AccessTracker unknown peer returns false", "[AccessTracker]")
{
    AccessTracker::reset();

    AccessTracker::t_PeerStats stats{};
    REQUIRE_FALSE(AccessTracker::getPeerStats(999999, stats));
}


TEST_CASE("AccessTracker getMostAccessedResources ordering", "[AccessTracker]")
{
    AccessTracker::reset();

    ulong rid1 = BASE_RESOURCE + 20;
    ulong rid2 = BASE_RESOURCE + 21;
    ulong rid3 = BASE_RESOURCE + 22;
    ulong pid  = BASE_PEER + 20;

    // Access rid2 most, rid1 second, rid3 least
    for (int i = 0; i < 10; ++i) {
        AccessTracker::recordResourceAccess(rid2, pid, "popular.mp3", 100);
    }
    for (int i = 0; i < 5; ++i) {
        AccessTracker::recordResourceAccess(rid1, pid, "medium.mp3", 100);
    }
    AccessTracker::recordResourceAccess(rid3, pid, "rare.mp3", 100);

    vector<AccessTracker::t_ResourceStats> list;
    AccessTracker::getMostAccessedResources(list, 50);

    REQUIRE(list.size() == 3);
    REQUIRE(list[0].resourceId == rid2);
    REQUIRE(list[1].resourceId == rid1);
    REQUIRE(list[2].resourceId == rid3);
}


TEST_CASE("AccessTracker getMostAccessedResources limit", "[AccessTracker]")
{
    AccessTracker::reset();

    ulong pid = BASE_PEER + 30;

    for (ulong i = 0; i < 10; ++i) {
        AccessTracker::recordResourceAccess(BASE_RESOURCE + 30 + i, pid,
                                            "file-" + std::to_string(i), 100);
    }

    vector<AccessTracker::t_ResourceStats> list;
    AccessTracker::getMostAccessedResources(list, 3);
    REQUIRE(list.size() == 3);
}


TEST_CASE("AccessTracker query term tracking", "[AccessTracker]")
{
    AccessTracker::reset();

    AccessTracker::recordQueryAccess("alpine music");
    AccessTracker::recordQueryAccess("alpine music");
    AccessTracker::recordQueryAccess("other query");

    string json = AccessTracker::serializeJson();
    auto parsed = nlohmann::json::parse(json);
    REQUIRE(parsed.contains("queryTerms"));
}


TEST_CASE("AccessTracker serializeJson produces valid JSON", "[AccessTracker]")
{
    AccessTracker::reset();

    AccessTracker::recordResourceAccess(BASE_RESOURCE + 50, BASE_PEER + 50,
                                        "test.mp3", 512);

    string json = AccessTracker::serializeJson();
    auto parsed = nlohmann::json::parse(json);
    REQUIRE(parsed.contains("resources"));
    REQUIRE(parsed.contains("peers"));
    REQUIRE(parsed.contains("queryTerms"));
}


TEST_CASE("AccessTracker serializeText produces readable output", "[AccessTracker]")
{
    AccessTracker::reset();

    AccessTracker::recordResourceAccess(BASE_RESOURCE + 60, BASE_PEER + 60,
                                        "test.mp3", 256);

    string text = AccessTracker::serializeText();
    REQUIRE(text.contains("Resource Access Statistics"));
}
