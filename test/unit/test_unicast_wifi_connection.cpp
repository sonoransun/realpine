/// Unit tests for UnicastWifiConnection MAC cache and resolution

#include <UnicastWifiConnection.h>
#include <catch2/catch_test_macros.hpp>
#include <cstring>


#ifdef __linux__


TEST_CASE("UnicastWifiConnection MAC cache stores and retrieves entries", "[UnicastWifiConnection]")
{
    UnicastWifiConnection conn("lo");

    byte mac[6] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};
    ulong testIp = 0xC0A80101;  // 192.168.1.1

    REQUIRE(conn.setDestinationMac(testIp, mac));

    byte resolved[6] = {};
    REQUIRE(conn.resolveDestinationMac(testIp, resolved));
    REQUIRE(memcmp(mac, resolved, 6) == 0);
}


TEST_CASE("UnicastWifiConnection MAC cache returns false for unknown IP", "[UnicastWifiConnection]")
{
    UnicastWifiConnection conn("lo");

    byte resolved[6] = {};
    REQUIRE_FALSE(conn.resolveDestinationMac(0xC0A80102, resolved));
}


TEST_CASE("UnicastWifiConnection setDestinationMac rejects zero IP", "[UnicastWifiConnection]")
{
    UnicastWifiConnection conn("lo");

    byte mac[6] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
    REQUIRE_FALSE(conn.setDestinationMac(0, mac));
}


TEST_CASE("UnicastWifiConnection MAC cache overwrites existing entry", "[UnicastWifiConnection]")
{
    UnicastWifiConnection conn("lo");

    ulong testIp = 0x0A000001;  // 10.0.0.1

    byte mac1[6] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
    byte mac2[6] = {0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6};

    REQUIRE(conn.setDestinationMac(testIp, mac1));
    REQUIRE(conn.setDestinationMac(testIp, mac2));

    byte resolved[6] = {};
    REQUIRE(conn.resolveDestinationMac(testIp, resolved));
    REQUIRE(memcmp(mac2, resolved, 6) == 0);
}


TEST_CASE("UnicastWifiConnection multiple IPs in cache", "[UnicastWifiConnection]")
{
    UnicastWifiConnection conn("lo");

    ulong ip1 = 0xC0A80101;
    ulong ip2 = 0xC0A80102;

    byte mac1[6] = {0x11, 0x11, 0x11, 0x11, 0x11, 0x11};
    byte mac2[6] = {0x22, 0x22, 0x22, 0x22, 0x22, 0x22};

    REQUIRE(conn.setDestinationMac(ip1, mac1));
    REQUIRE(conn.setDestinationMac(ip2, mac2));

    byte resolved[6] = {};

    REQUIRE(conn.resolveDestinationMac(ip1, resolved));
    REQUIRE(memcmp(mac1, resolved, 6) == 0);

    REQUIRE(conn.resolveDestinationMac(ip2, resolved));
    REQUIRE(memcmp(mac2, resolved, 6) == 0);
}


#else  // Non-Linux


TEST_CASE("UnicastWifiConnection stubs return false on non-Linux", "[UnicastWifiConnection]")
{
    UnicastWifiConnection conn("wlan0");

    byte mac[6] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
    REQUIRE_FALSE(conn.setDestinationMac(0xC0A80101, mac));

    byte resolved[6] = {};
    REQUIRE_FALSE(conn.resolveDestinationMac(0xC0A80101, resolved));

    REQUIRE_FALSE(conn.sendData(0xC0A80101, 8080, nullptr, 0));

    ulong srcIp = 0;
    ushort srcPort = 0;
    uint dataLen = 0;
    byte buf[128] = {};
    REQUIRE_FALSE(conn.receiveData(buf, sizeof(buf), srcIp, srcPort, dataLen));
}


#endif
