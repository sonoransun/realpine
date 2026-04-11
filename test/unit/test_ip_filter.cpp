/// Unit tests for IpFilter

#include <IpFilter.h>
#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>


// Helper to reset IpFilter state between tests
static void
resetIpFilter()
{
    IpFilter::initialize("", "");
    IpFilter::reload();
}


TEST_CASE("Empty allowlist allows all IPs", "[IpFilter]")
{
    resetIpFilter();

    REQUIRE(IpFilter::getAllowlist().empty());
    REQUIRE(IpFilter::getBlocklist().empty());

    SECTION("any IP is allowed when both lists are empty")
    {
        REQUIRE(IpFilter::isAllowed("1.2.3.4"));
        REQUIRE(IpFilter::isAllowed("255.255.255.255"));
        REQUIRE(IpFilter::isAllowed("0.0.0.0"));
    }
}


TEST_CASE("Single IP allowlist", "[IpFilter]")
{
    resetIpFilter();

    REQUIRE(IpFilter::allowIp("192.168.1.1"));

    SECTION("matching IP is allowed")
    {
        REQUIRE(IpFilter::isAllowed("192.168.1.1"));
    }

    SECTION("non-matching IP is denied")
    {
        REQUIRE_FALSE(IpFilter::isAllowed("192.168.1.2"));
    }
}


TEST_CASE("CIDR /24 allowlist", "[IpFilter]")
{
    resetIpFilter();

    REQUIRE(IpFilter::allowIp("10.0.0.0/24"));

    SECTION("IP within range is allowed")
    {
        REQUIRE(IpFilter::isAllowed("10.0.0.255"));
        REQUIRE(IpFilter::isAllowed("10.0.0.0"));
        REQUIRE(IpFilter::isAllowed("10.0.0.128"));
    }

    SECTION("IP outside range is denied")
    {
        REQUIRE_FALSE(IpFilter::isAllowed("10.0.1.0"));
        REQUIRE_FALSE(IpFilter::isAllowed("10.0.2.1"));
    }
}


TEST_CASE("CIDR /32 single host", "[IpFilter]")
{
    resetIpFilter();

    REQUIRE(IpFilter::allowIp("10.0.0.5/32"));

    SECTION("exact IP is allowed")
    {
        REQUIRE(IpFilter::isAllowed("10.0.0.5"));
    }

    SECTION("adjacent IPs are denied")
    {
        REQUIRE_FALSE(IpFilter::isAllowed("10.0.0.4"));
        REQUIRE_FALSE(IpFilter::isAllowed("10.0.0.6"));
    }
}


TEST_CASE("Blocklist takes precedence", "[IpFilter]")
{
    resetIpFilter();

    REQUIRE(IpFilter::allowIp("10.0.0.0/24"));
    REQUIRE(IpFilter::blockIp("10.0.0.5"));

    SECTION("blocked IP within allowed range is denied")
    {
        REQUIRE_FALSE(IpFilter::isAllowed("10.0.0.5"));
    }

    SECTION("non-blocked IP within allowed range is still allowed")
    {
        REQUIRE(IpFilter::isAllowed("10.0.0.6"));
        REQUIRE(IpFilter::isAllowed("10.0.0.1"));
    }
}


TEST_CASE("Empty allowlist with blocklist", "[IpFilter]")
{
    resetIpFilter();

    REQUIRE(IpFilter::blockIp("1.2.3.4"));

    SECTION("blocked IP is denied")
    {
        REQUIRE_FALSE(IpFilter::isAllowed("1.2.3.4"));
    }

    SECTION("non-blocked IP is allowed")
    {
        REQUIRE(IpFilter::isAllowed("5.6.7.8"));
    }
}


TEST_CASE("Invalid CIDR inputs", "[IpFilter]")
{
    resetIpFilter();

    SECTION("non-IP string is rejected")
    {
        REQUIRE_FALSE(IpFilter::allowIp("not-an-ip"));
    }

    SECTION("empty string is rejected")
    {
        REQUIRE_FALSE(IpFilter::allowIp(""));
    }

    SECTION("prefix length out of range is rejected")
    {
        REQUIRE_FALSE(IpFilter::allowIp("192.168.1.1/33"));
    }
}


TEST_CASE("Dynamic add and remove", "[IpFilter]")
{
    resetIpFilter();

    REQUIRE(IpFilter::allowIp("1.2.3.4"));
    REQUIRE(IpFilter::getAllowlist().size() == 1);

    SECTION("removeAllow succeeds for existing entry")
    {
        REQUIRE(IpFilter::removeAllow("1.2.3.4"));
        REQUIRE(IpFilter::getAllowlist().empty());
    }

    SECTION("removeAllow returns false for nonexistent entry")
    {
        REQUIRE_FALSE(IpFilter::removeAllow("nonexistent"));
    }

    SECTION("removeBlock returns false when blocklist is empty")
    {
        REQUIRE_FALSE(IpFilter::removeBlock("1.2.3.4"));
    }
}


TEST_CASE("File loading", "[IpFilter]")
{
    resetIpFilter();

    auto tmpDir = std::filesystem::temp_directory_path();
    auto allowFile = tmpDir / "test_ipfilter_allow.txt";

    // Write a file with valid entries, a comment, and a blank line
    {
        std::ofstream out(allowFile);
        out << "10.0.0.0/24\n"
            << "# comment\n"
            << "\n"
            << "192.168.1.0/24\n";
    }

    IpFilter::initialize(allowFile.string(), "");

    auto allowlist = IpFilter::getAllowlist();
    REQUIRE(allowlist.size() == 2);
    REQUIRE(allowlist[0] == "10.0.0.0/24");
    REQUIRE(allowlist[1] == "192.168.1.0/24");

    SECTION("loaded entries are enforced")
    {
        REQUIRE(IpFilter::isAllowed("10.0.0.50"));
        REQUIRE(IpFilter::isAllowed("192.168.1.100"));
        REQUIRE_FALSE(IpFilter::isAllowed("172.16.0.1"));
    }

    std::filesystem::remove(allowFile);
}


TEST_CASE("Reload picks up file changes", "[IpFilter]")
{
    resetIpFilter();

    auto tmpDir = std::filesystem::temp_directory_path();
    auto allowFile = tmpDir / "test_ipfilter_reload.txt";

    // Write initial file
    {
        std::ofstream out(allowFile);
        out << "10.0.0.0/24\n";
    }

    IpFilter::initialize(allowFile.string(), "");
    REQUIRE(IpFilter::getAllowlist().size() == 1);
    REQUIRE(IpFilter::isAllowed("10.0.0.1"));
    REQUIRE_FALSE(IpFilter::isAllowed("172.16.0.1"));

    // Modify the file
    {
        std::ofstream out(allowFile);
        out << "172.16.0.0/16\n";
    }

    REQUIRE(IpFilter::reload());

    SECTION("old entries are replaced by new ones")
    {
        auto allowlist = IpFilter::getAllowlist();
        REQUIRE(allowlist.size() == 1);
        REQUIRE(allowlist[0] == "172.16.0.0/16");
    }

    SECTION("new entries are enforced")
    {
        REQUIRE(IpFilter::isAllowed("172.16.0.1"));
        REQUIRE_FALSE(IpFilter::isAllowed("10.0.0.1"));
    }

    std::filesystem::remove(allowFile);
}
