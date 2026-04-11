/// Unit tests for SO_REUSEPORT on UdpConnection: two sockets bind the same
/// port, second bind succeeds because UdpConnection::create() sets
/// SO_REUSEADDR + SO_REUSEPORT before bind. Linux-only behavior.

#include <NetUtils.h>
#include <Platform.h>
#include <UdpConnection.h>
#include <catch2/catch_test_macros.hpp>


#ifdef __linux__


TEST_CASE("Two UdpConnections bind the same port via SO_REUSEPORT", "[UdpConnection][SO_REUSEPORT]")
{
    ulong loopback = 0;
    REQUIRE(NetUtils::stringIpToLong("127.0.0.1", loopback));

    // First socket: ephemeral port on loopback.
    UdpConnection first;
    REQUIRE(first.create(loopback, 0));
    REQUIRE(first.getFd() >= 0);

    ulong boundIp = 0;
    ushort boundPort = 0;
    REQUIRE(NetUtils::getLocalEndpoint(first.getFd(), boundIp, boundPort));
    REQUIRE(boundPort != 0);

    // Second socket: bind the same (loopback, port). With SO_REUSEPORT +
    // SO_REUSEADDR set in UdpConnection::create() before bind(), Linux
    // accepts the second bind.
    UdpConnection second;
    REQUIRE(second.create(loopback, boundPort));
    REQUIRE(second.getFd() >= 0);

    ulong boundIp2 = 0;
    ushort boundPort2 = 0;
    REQUIRE(NetUtils::getLocalEndpoint(second.getFd(), boundIp2, boundPort2));
    REQUIRE(boundPort2 == boundPort);

    first.close();
    second.close();
}


TEST_CASE("Three UdpConnections share the same port via SO_REUSEPORT", "[UdpConnection][SO_REUSEPORT]")
{
    ulong loopback = 0;
    REQUIRE(NetUtils::stringIpToLong("127.0.0.1", loopback));

    UdpConnection first;
    REQUIRE(first.create(loopback, 0));

    ulong ip = 0;
    ushort port = 0;
    REQUIRE(NetUtils::getLocalEndpoint(first.getFd(), ip, port));
    REQUIRE(port != 0);

    UdpConnection second;
    REQUIRE(second.create(loopback, port));

    UdpConnection third;
    REQUIRE(third.create(loopback, port));

    REQUIRE(first.getFd() >= 0);
    REQUIRE(second.getFd() >= 0);
    REQUIRE(third.getFd() >= 0);
}


#else  // !__linux__


// SO_REUSEPORT semantics are not portable. On non-Linux platforms the test
// is a no-op — Catch2WithMain runs zero registered test cases and exits 0.
//


#endif
