/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AuditLog.h>
#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>
#include <thread>
#include <vector>


static string
getTempLogPath()
{
    static int counter = 0;
    auto path = std::filesystem::temp_directory_path() / ("test_audit_log_" + std::to_string(++counter) + ".jsonl");
    return path.string();
}


static vector<string>
readLines(const string & path)
{
    vector<string> lines;
    std::ifstream in(path);
    string line;
    while (std::getline(in, line)) {
        if (!line.empty()) {
            lines.push_back(line);
        }
    }
    return lines;
}


TEST_CASE("AuditLog initialize and shutdown lifecycle", "[AuditLog]")
{
    auto path = getTempLogPath();
    REQUIRE(AuditLog::initialize(path));
    AuditLog::shutdown();
    std::filesystem::remove(path);
}


TEST_CASE("AuditLog record writes valid JSON-lines", "[AuditLog]")
{
    auto path = getTempLogPath();
    REQUIRE(AuditLog::initialize(path));

    AuditLog::record("auth", "user1", "login");
    AuditLog::shutdown();

    auto lines = readLines(path);
    REQUIRE(lines.size() == 1);

    auto json = nlohmann::json::parse(lines[0]);
    REQUIRE(json.contains("event"));
    REQUIRE(json["event"] == "auth");
    REQUIRE(json["actor"] == "user1");
    REQUIRE(json["action"] == "login");
    REQUIRE(json.contains("timestamp"));

    std::filesystem::remove(path);
}


TEST_CASE("AuditLog multiple records written in order", "[AuditLog]")
{
    auto path = getTempLogPath();
    REQUIRE(AuditLog::initialize(path));

    AuditLog::record("event1", "actor1", "action1");
    AuditLog::record("event2", "actor2", "action2");
    AuditLog::record("event3", "actor3", "action3");
    AuditLog::shutdown();

    auto lines = readLines(path);
    REQUIRE(lines.size() == 3);

    auto j1 = nlohmann::json::parse(lines[0]);
    auto j2 = nlohmann::json::parse(lines[1]);
    auto j3 = nlohmann::json::parse(lines[2]);
    REQUIRE(j1["event"] == "event1");
    REQUIRE(j2["event"] == "event2");
    REQUIRE(j3["event"] == "event3");

    std::filesystem::remove(path);
}


TEST_CASE("AuditLog record before initialize is safe", "[AuditLog]")
{
    // Should not crash
    AuditLog::record("test", "actor", "action");
}


TEST_CASE("AuditLog invalid path returns false", "[AuditLog]")
{
    REQUIRE_FALSE(AuditLog::initialize("/nonexistent/directory/audit.log"));
}


TEST_CASE("AuditLog shutdown drains queue", "[AuditLog]")
{
    auto path = getTempLogPath();
    REQUIRE(AuditLog::initialize(path));

    // Record many events quickly
    for (int i = 0; i < 50; ++i) {
        AuditLog::record("bulk", "actor", "action" + std::to_string(i));
    }

    // Shutdown should drain all pending events
    AuditLog::shutdown();

    auto lines = readLines(path);
    REQUIRE(lines.size() == 50);

    std::filesystem::remove(path);
}


TEST_CASE("AuditLog concurrent records", "[AuditLog]")
{
    auto path = getTempLogPath();
    REQUIRE(AuditLog::initialize(path));

    auto worker = [](int threadId) {
        for (int i = 0; i < 20; ++i) {
            AuditLog::record("concurrent", "thread-" + std::to_string(threadId), "action-" + std::to_string(i));
        }
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back(worker, i);
    }
    for (auto & t : threads) {
        t.join();
    }

    AuditLog::shutdown();

    auto lines = readLines(path);
    REQUIRE(lines.size() == 80);

    // Verify each line is valid JSON
    for (const auto & line : lines) {
        auto json = nlohmann::json::parse(line);
        REQUIRE(json.contains("event"));
    }

    std::filesystem::remove(path);
}
