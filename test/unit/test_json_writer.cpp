/// Copyright (C) 2026 sonoransun — see LICENCE.txt

#include <JsonWriter.h>
#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>


// Helper: parse result and verify it is valid JSON
static nlohmann::json
parseResult(JsonWriter & writer)
{
    auto str = writer.result();
    auto doc = nlohmann::json::parse(str);
    return doc;
}


TEST_CASE("JsonWriter empty object", "[JsonWriter]")
{
    JsonWriter writer;
    writer.beginObject();
    writer.endObject();

    REQUIRE(writer.result() == "{}");

    auto doc = nlohmann::json::parse(writer.result());
    REQUIRE(doc.is_object());
    REQUIRE(doc.empty());
}


TEST_CASE("JsonWriter object with string value", "[JsonWriter]")
{
    JsonWriter writer;
    writer.beginObject();
    writer.key("name");
    writer.value("alpine"s);
    writer.endObject();

    auto doc = parseResult(writer);
    REQUIRE(doc.is_object());
    REQUIRE(doc["name"] == "alpine");
}


TEST_CASE("JsonWriter object with ulong value", "[JsonWriter]")
{
    JsonWriter writer;
    writer.beginObject();
    writer.key("count");
    writer.value(static_cast<ulong>(42));
    writer.endObject();

    auto doc = parseResult(writer);
    REQUIRE(doc["count"] == 42);
}


TEST_CASE("JsonWriter object with bool values", "[JsonWriter]")
{
    JsonWriter writer;
    writer.beginObject();
    writer.key("active");
    writer.value(true);
    writer.key("deleted");
    writer.value(false);
    writer.endObject();

    auto doc = parseResult(writer);
    REQUIRE(doc["active"] == true);
    REQUIRE(doc["deleted"] == false);
}


TEST_CASE("JsonWriter object with multiple keys", "[JsonWriter]")
{
    JsonWriter writer;
    writer.beginObject();
    writer.key("queryId");
    writer.value(static_cast<ulong>(123));
    writer.key("inProgress");
    writer.value(true);
    writer.key("totalPeers");
    writer.value(static_cast<ulong>(5));
    writer.key("status");
    writer.value("running"s);
    writer.endObject();

    auto doc = parseResult(writer);
    REQUIRE(doc["queryId"] == 123);
    REQUIRE(doc["inProgress"] == true);
    REQUIRE(doc["totalPeers"] == 5);
    REQUIRE(doc["status"] == "running");
}


TEST_CASE("JsonWriter nested objects", "[JsonWriter]")
{
    JsonWriter writer;
    writer.beginObject();
    writer.key("status");
    writer.value("running"s);
    writer.key("services");
    writer.beginObject();
    writer.key("fuse");
    writer.value(false);
    writer.endObject();
    writer.key("cluster");
    writer.beginObject();
    writer.key("nodeId");
    writer.value("node-abc"s);
    writer.key("nodeCount");
    writer.value(static_cast<ulong>(3));
    writer.endObject();
    writer.endObject();

    auto doc = parseResult(writer);
    REQUIRE(doc["status"] == "running");
    REQUIRE(doc["services"]["fuse"] == false);
    REQUIRE(doc["cluster"]["nodeId"] == "node-abc");
    REQUIRE(doc["cluster"]["nodeCount"] == 3);
}


TEST_CASE("JsonWriter empty array", "[JsonWriter]")
{
    JsonWriter writer;
    writer.beginObject();
    writer.key("items");
    writer.beginArray();
    writer.endArray();
    writer.endObject();

    auto doc = parseResult(writer);
    REQUIRE(doc["items"].is_array());
    REQUIRE(doc["items"].empty());
}


TEST_CASE("JsonWriter array of strings", "[JsonWriter]")
{
    JsonWriter writer;
    writer.beginObject();
    writer.key("locators");
    writer.beginArray();
    writer.value("http://example.com/a"s);
    writer.value("http://example.com/b"s);
    writer.value("http://example.com/c"s);
    writer.endArray();
    writer.endObject();

    auto doc = parseResult(writer);
    REQUIRE(doc["locators"].size() == 3);
    REQUIRE(doc["locators"][0] == "http://example.com/a");
    REQUIRE(doc["locators"][1] == "http://example.com/b");
    REQUIRE(doc["locators"][2] == "http://example.com/c");
}


TEST_CASE("JsonWriter array of ulong values", "[JsonWriter]")
{
    JsonWriter writer;
    writer.beginObject();
    writer.key("peerIds");
    writer.beginArray();
    writer.value(static_cast<ulong>(100));
    writer.value(static_cast<ulong>(200));
    writer.value(static_cast<ulong>(300));
    writer.endArray();
    writer.endObject();

    auto doc = parseResult(writer);
    REQUIRE(doc["peerIds"].size() == 3);
    REQUIRE(doc["peerIds"][0] == 100);
    REQUIRE(doc["peerIds"][1] == 200);
    REQUIRE(doc["peerIds"][2] == 300);
}


TEST_CASE("JsonWriter array of objects", "[JsonWriter]")
{
    JsonWriter writer;
    writer.beginObject();
    writer.key("data");
    writer.beginArray();

    writer.beginObject();
    writer.key("peerId");
    writer.value(static_cast<ulong>(1));
    writer.key("ipAddress");
    writer.value("10.0.0.1"s);
    writer.endObject();

    writer.beginObject();
    writer.key("peerId");
    writer.value(static_cast<ulong>(2));
    writer.key("ipAddress");
    writer.value("10.0.0.2"s);
    writer.endObject();

    writer.endArray();
    writer.key("total");
    writer.value(static_cast<ulong>(2));
    writer.endObject();

    auto doc = parseResult(writer);
    REQUIRE(doc["data"].size() == 2);
    REQUIRE(doc["data"][0]["peerId"] == 1);
    REQUIRE(doc["data"][0]["ipAddress"] == "10.0.0.1");
    REQUIRE(doc["data"][1]["peerId"] == 2);
    REQUIRE(doc["data"][1]["ipAddress"] == "10.0.0.2");
    REQUIRE(doc["total"] == 2);
}


TEST_CASE("JsonWriter escaping — quotes and backslashes", "[JsonWriter]")
{
    JsonWriter writer;
    writer.beginObject();
    writer.key("desc");
    writer.value("He said \"hello\""s);
    writer.key("path");
    writer.value("C:\\Users\\test"s);
    writer.endObject();

    auto doc = parseResult(writer);
    REQUIRE(doc["desc"] == "He said \"hello\"");
    REQUIRE(doc["path"] == "C:\\Users\\test");
}


TEST_CASE("JsonWriter escaping — newlines and tabs", "[JsonWriter]")
{
    JsonWriter writer;
    writer.beginObject();
    writer.key("text");
    writer.value("line1\nline2\ttab"s);
    writer.endObject();

    auto doc = parseResult(writer);
    REQUIRE(doc["text"] == "line1\nline2\ttab");
}


TEST_CASE("JsonWriter escaping — control characters", "[JsonWriter]")
{
    JsonWriter writer;
    writer.beginObject();
    writer.key("ctrl");

    string val;
    val += '\x01';
    val += '\x1F';
    writer.value(val);
    writer.endObject();

    auto doc = parseResult(writer);
    REQUIRE(doc["ctrl"] == val);
}


TEST_CASE("JsonWriter escaping in keys", "[JsonWriter]")
{
    JsonWriter writer;
    writer.beginObject();
    writer.key("key\"with\"quotes"s);
    writer.value("ok"s);
    writer.endObject();

    auto doc = parseResult(writer);
    REQUIRE(doc.contains("key\"with\"quotes"));
    REQUIRE(doc["key\"with\"quotes"] == "ok");
}


TEST_CASE("JsonWriter QueryHandler startQuery pattern", "[JsonWriter]")
{
    // Matches QueryHandler::startQuery output pattern
    ulong queryId = 42;

    JsonWriter writer;
    writer.beginObject();
    writer.key("queryId");
    writer.value(queryId);
    writer.endObject();

    auto doc = parseResult(writer);
    REQUIRE(doc["queryId"] == 42);
    REQUIRE(writer.result() == "{\"queryId\":42}");
}


TEST_CASE("JsonWriter QueryHandler getQueryResults pattern", "[JsonWriter]")
{
    // Matches the nested array-of-objects with nested arrays pattern
    // from QueryHandler::getQueryResults

    JsonWriter writer;
    writer.beginObject();
    writer.key("queryId");
    writer.value(static_cast<ulong>(99));
    writer.key("data");
    writer.beginArray();

    // First result entry
    writer.beginObject();
    writer.key("peerId");
    writer.value(static_cast<ulong>(10));
    writer.key("resourceId");
    writer.value(static_cast<ulong>(1));
    writer.key("size");
    writer.value(static_cast<ulong>(1024));
    writer.key("description");
    writer.value("file.txt"s);
    writer.key("locators");
    writer.beginArray();
    writer.value("http://a.com/1"s);
    writer.value("http://b.com/1"s);
    writer.endArray();
    writer.endObject();

    writer.endArray();
    writer.key("total");
    writer.value(static_cast<ulong>(1));
    writer.key("limit");
    writer.value(static_cast<ulong>(100));
    writer.key("offset");
    writer.value(static_cast<ulong>(0));
    writer.key("hasMore");
    writer.value(false);
    writer.endObject();

    auto doc = parseResult(writer);
    REQUIRE(doc["queryId"] == 99);
    REQUIRE(doc["data"].size() == 1);
    REQUIRE(doc["data"][0]["peerId"] == 10);
    REQUIRE(doc["data"][0]["locators"].size() == 2);
    REQUIRE(doc["total"] == 1);
    REQUIRE(doc["hasMore"] == false);
}


TEST_CASE("JsonWriter PeerHandler getAllPeers pattern", "[JsonWriter]")
{
    // Matches PeerHandler::getAllPeers with pagination
    JsonWriter writer;
    writer.beginObject();
    writer.key("data");
    writer.beginArray();

    writer.beginObject();
    writer.key("peerId");
    writer.value(static_cast<ulong>(100));
    writer.key("ipAddress");
    writer.value("192.168.1.1"s);
    writer.key("port");
    writer.value(static_cast<ulong>(9000));
    writer.key("lastRecvTime");
    writer.value(static_cast<ulong>(1000));
    writer.key("lastSendTime");
    writer.value(static_cast<ulong>(2000));
    writer.key("avgBandwidth");
    writer.value(static_cast<ulong>(500));
    writer.key("peakBandwidth");
    writer.value(static_cast<ulong>(1000));
    writer.endObject();

    writer.endArray();
    writer.key("total");
    writer.value(static_cast<ulong>(1));
    writer.key("limit");
    writer.value(static_cast<ulong>(100));
    writer.key("offset");
    writer.value(static_cast<ulong>(0));
    writer.endObject();

    auto doc = parseResult(writer);
    REQUIRE(doc["data"].size() == 1);
    REQUIRE(doc["data"][0]["peerId"] == 100);
    REQUIRE(doc["data"][0]["ipAddress"] == "192.168.1.1");
    REQUIRE(doc["total"] == 1);
}


TEST_CASE("JsonWriter StatusHandler pattern with nested objects", "[JsonWriter]")
{
    // Matches StatusHandler::getStatus
    JsonWriter writer;
    writer.beginObject();
    writer.key("status");
    writer.value("running"s);
    writer.key("version");
    writer.value("devel-00019"s);
    writer.key("apiVersion");
    writer.value("1.0"s);
    writer.key("uptimeSeconds");
    writer.value(static_cast<ulong>(3600));
    writer.key("activePeerCount");
    writer.value(static_cast<ulong>(5));
    writer.key("services");
    writer.beginObject();
    writer.key("fuse");
    writer.value(false);
    writer.endObject();
    writer.key("cluster");
    writer.beginObject();
    writer.key("nodeId");
    writer.value("node-abc123"s);
    writer.key("nodeCount");
    writer.value(static_cast<ulong>(3));
    writer.key("region");
    writer.value("default"s);
    writer.key("isolated");
    writer.value(false);
    writer.endObject();
    writer.endObject();

    auto doc = parseResult(writer);
    REQUIRE(doc["status"] == "running");
    REQUIRE(doc["services"]["fuse"] == false);
    REQUIRE(doc["cluster"]["nodeId"] == "node-abc123");
    REQUIRE(doc["cluster"]["isolated"] == false);
}


TEST_CASE("JsonWriter AdminHandler getBannedPeers pattern", "[JsonWriter]")
{
    // Object with array of strings + total count
    JsonWriter writer;
    writer.beginObject();
    writer.key("banned");
    writer.beginArray();
    writer.value("10.0.0.1"s);
    writer.value("10.0.0.2"s);
    writer.endArray();
    writer.key("total");
    writer.value(static_cast<ulong>(2));
    writer.endObject();

    auto doc = parseResult(writer);
    REQUIRE(doc["banned"].size() == 2);
    REQUIRE(doc["banned"][0] == "10.0.0.1");
    REQUIRE(doc["total"] == 2);
}


TEST_CASE("JsonWriter separator is a no-op", "[JsonWriter]")
{
    JsonWriter writer;
    writer.beginObject();
    writer.key("a");
    writer.value("1"s);
    writer.separator();
    writer.key("b");
    writer.value("2"s);
    writer.endObject();

    auto doc = parseResult(writer);
    REQUIRE(doc["a"] == "1");
    REQUIRE(doc["b"] == "2");
}


TEST_CASE("JsonWriter DiscoveryBeacon pattern", "[JsonWriter]")
{
    // Matches DiscoveryBeacon::threadMain beacon payload
    JsonWriter writer;
    writer.beginObject();
    writer.key("service");
    writer.value("alpine-bridge"s);
    writer.key("version");
    writer.value("1"s);
    writer.key("restPort");
    writer.value(static_cast<ulong>(8080));
    writer.key("bridgeVersion");
    writer.value("devel-00019"s);
    writer.endObject();

    auto doc = parseResult(writer);
    REQUIRE(doc["service"] == "alpine-bridge");
    REQUIRE(doc["restPort"] == 8080);
}


TEST_CASE("JsonWriter ClusterCoordinator nested arrays in objects", "[JsonWriter]")
{
    // Matches ClusterCoordinator::handleClusterStatus node capabilities pattern
    JsonWriter writer;
    writer.beginObject();
    writer.key("nodeId");
    writer.value("node-abc"s);
    writer.key("nodes");
    writer.beginArray();

    writer.beginObject();
    writer.key("nodeId");
    writer.value("node-1"s);
    writer.key("capabilities");
    writer.beginArray();
    writer.value("query"s);
    writer.value("transfer"s);
    writer.value("stream"s);
    writer.endArray();
    writer.endObject();

    writer.endArray();
    writer.endObject();

    auto doc = parseResult(writer);
    REQUIRE(doc["nodes"][0]["capabilities"].size() == 3);
    REQUIRE(doc["nodes"][0]["capabilities"][0] == "query");
    REQUIRE(doc["nodes"][0]["capabilities"][2] == "stream");
}


TEST_CASE("JsonWriter multiple independent instances", "[JsonWriter]")
{
    // Matches QueryHandler::streamQueryResults pattern — multiple writers
    JsonWriter w1;
    w1.beginObject();
    w1.key("queryId");
    w1.value(static_cast<ulong>(1));
    w1.endObject();

    JsonWriter w2;
    w2.beginObject();
    w2.key("queryId");
    w2.value(static_cast<ulong>(2));
    w2.endObject();

    auto doc1 = nlohmann::json::parse(w1.result());
    auto doc2 = nlohmann::json::parse(w2.result());
    REQUIRE(doc1["queryId"] == 1);
    REQUIRE(doc2["queryId"] == 2);
}


TEST_CASE("JsonWriter empty string value", "[JsonWriter]")
{
    JsonWriter writer;
    writer.beginObject();
    writer.key("name");
    writer.value(""s);
    writer.endObject();

    auto doc = parseResult(writer);
    REQUIRE(doc["name"] == "");
}


TEST_CASE("JsonWriter zero ulong value", "[JsonWriter]")
{
    JsonWriter writer;
    writer.beginObject();
    writer.key("count");
    writer.value(static_cast<ulong>(0));
    writer.endObject();

    auto doc = parseResult(writer);
    REQUIRE(doc["count"] == 0);
}


TEST_CASE("JsonWriter exact string output for simple cases", "[JsonWriter]")
{
    {
        JsonWriter w;
        w.beginObject();
        w.endObject();
        REQUIRE(w.result() == "{}");
    }
    {
        JsonWriter w;
        w.beginObject();
        w.key("a");
        w.value("b"s);
        w.endObject();
        REQUIRE(w.result() == "{\"a\":\"b\"}");
    }
    {
        JsonWriter w;
        w.beginObject();
        w.key("x");
        w.value(static_cast<ulong>(1));
        w.key("y");
        w.value(true);
        w.endObject();
        REQUIRE(w.result() == "{\"x\":1,\"y\":true}");
    }
    {
        JsonWriter w;
        w.beginObject();
        w.key("arr");
        w.beginArray();
        w.value("a"s);
        w.value("b"s);
        w.endArray();
        w.endObject();
        REQUIRE(w.result() == "{\"arr\":[\"a\",\"b\"]}");
    }
}
