/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <catch2/catch_test_macros.hpp>
#include <StringUtils.h>
#include <string>

using namespace std::string_literals;


TEST_CASE("sanitizeForLog: empty string returns empty", "[StringUtils]")
{
    REQUIRE(StringUtils::sanitizeForLog("") == ""s);
}

TEST_CASE("sanitizeForLog: clean printable string is unchanged", "[StringUtils]")
{
    auto input = "Hello, World! 123 ~`@#$%^&*()_+-={}[]|:;<>?,./\""s;
    REQUIRE(StringUtils::sanitizeForLog(input) == input);
}

TEST_CASE("sanitizeForLog: embedded null byte is escaped", "[StringUtils]")
{
    auto input = "before\0after"s;
    REQUIRE(StringUtils::sanitizeForLog(input) == "before<0x00>after"s);
}

TEST_CASE("sanitizeForLog: ANSI escape sequences are escaped", "[StringUtils]")
{
    auto input = "normal\x1B[31mred\x1B[0mnormal"s;
    REQUIRE(StringUtils::sanitizeForLog(input) == "normal<0x1B>[31mred<0x1B>[0mnormal"s);
}

TEST_CASE("sanitizeForLog: newlines tabs and carriage returns are preserved", "[StringUtils]")
{
    auto input = "line1\nline2\tindented\r\n"s;
    REQUIRE(StringUtils::sanitizeForLog(input) == input);
}

TEST_CASE("sanitizeForLog: mixed content", "[StringUtils]")
{
    auto input = "ok\x01\n\x02\ttab\x7F"s;
    REQUIRE(StringUtils::sanitizeForLog(input) == "ok<0x01>\n<0x02>\ttab<0x7F>"s);
}

TEST_CASE("sanitizeForLog: all control chars", "[StringUtils]")
{
    // Build a string of all control chars that should be escaped:
    // 0x00-0x08, 0x0B, 0x0C, 0x0E-0x1F, 0x7F
    string input;
    string expected;

    for (int i = 0x00; i <= 0x08; ++i)
    {
        input += static_cast<char>(i);
        char buf[7];
        std::snprintf(buf, sizeof(buf), "<0x%02X>", i);
        expected += buf;
    }

    // 0x09 (\t), 0x0A (\n), 0x0D (\r) are preserved — skip them in this test
    input += '\x0B';
    expected += "<0x0B>"s;
    input += '\x0C';
    expected += "<0x0C>"s;

    for (int i = 0x0E; i <= 0x1F; ++i)
    {
        input += static_cast<char>(i);
        char buf[7];
        std::snprintf(buf, sizeof(buf), "<0x%02X>", i);
        expected += buf;
    }

    input += '\x7F';
    expected += "<0x7F>"s;

    REQUIRE(StringUtils::sanitizeForLog(input) == expected);
}
