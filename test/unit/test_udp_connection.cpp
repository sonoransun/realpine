/// Unit tests for UdpConnection

#include <NetUtils.h>
#include <Platform.h>
#include <UdpConnection.h>
#include <catch2/catch_test_macros.hpp>
#include <cstring>
#include <poll.h>


namespace {


// Wait until a UDP socket has data pending, up to timeoutMs. Returns true if
// data is ready, false on timeout.
//
bool
waitForUdpData(int fd, int timeoutMs)
{
    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = POLLIN;
    pfd.revents = 0;
    int rc = ::poll(&pfd, 1, timeoutMs);
    return rc > 0 && (pfd.revents & POLLIN) != 0;
}


}  // namespace


TEST_CASE("UdpConnection default state has invalid FD", "[UdpConnection]")
{
    UdpConnection conn;
    REQUIRE(conn.getFd() < 0);
}


TEST_CASE("UdpConnection create on loopback with ephemeral port succeeds", "[UdpConnection]")
{
    ulong loopback = 0;
    REQUIRE(NetUtils::stringIpToLong("127.0.0.1", loopback));

    UdpConnection conn;
    REQUIRE(conn.create(loopback, 0));
    REQUIRE(conn.getFd() >= 0);

    ulong boundIp = 0;
    ushort boundPort = 0;
    REQUIRE(NetUtils::getLocalEndpoint(conn.getFd(), boundIp, boundPort));
    REQUIRE(boundPort != 0);

    conn.close();
    REQUIRE(conn.getFd() < 0);
}


TEST_CASE("UdpConnection loopback send / receive round-trip", "[UdpConnection]")
{
    ulong loopback = 0;
    REQUIRE(NetUtils::stringIpToLong("127.0.0.1", loopback));

    UdpConnection receiver;
    REQUIRE(receiver.create(loopback, 0));

    ulong receiverIp = 0;
    ushort receiverPort = 0;
    REQUIRE(NetUtils::getLocalEndpoint(receiver.getFd(), receiverIp, receiverPort));
    REQUIRE(receiverPort != 0);

    UdpConnection sender;
    REQUIRE(sender.create(loopback, 0));

    ulong senderIp = 0;
    ushort senderPort = 0;
    REQUIRE(NetUtils::getLocalEndpoint(sender.getFd(), senderIp, senderPort));

    const byte payload[] = {'A', 'l', 'p', 'i', 'n', 'e', '!', 0x42};
    REQUIRE(sender.sendData(loopback, receiverPort, payload, sizeof(payload)));

    // Sockets are non-blocking by default — wait for data to arrive.
    REQUIRE(waitForUdpData(receiver.getFd(), 1000));

    byte buffer[128];
    std::memset(buffer, 0, sizeof(buffer));
    ulong srcIp = 0;
    ushort srcPort = 0;
    uint dataLength = 0;
    REQUIRE(receiver.receiveData(buffer, sizeof(buffer), srcIp, srcPort, dataLength));

    REQUIRE(dataLength == sizeof(payload));
    REQUIRE(std::memcmp(buffer, payload, sizeof(payload)) == 0);
    REQUIRE(srcIp == loopback);
    REQUIRE(srcPort == senderPort);
}


TEST_CASE("UdpConnection binding to already-used port succeeds with SO_REUSEPORT", "[UdpConnection]")
{
    ulong loopback = 0;
    REQUIRE(NetUtils::stringIpToLong("127.0.0.1", loopback));

    UdpConnection first;
    REQUIRE(first.create(loopback, 0));

    ulong ip = 0;
    ushort port = 0;
    REQUIRE(NetUtils::getLocalEndpoint(first.getFd(), ip, port));
    REQUIRE(port != 0);

    // Second connection binds to the same (loopback, port) pair.
    // SO_REUSEPORT + SO_REUSEADDR are set in UdpConnection::create() so this
    // succeeds on Linux 3.9+ / BSD. On platforms without SO_REUSEPORT the
    // bind would fail.
    UdpConnection second;
#ifdef SO_REUSEPORT
    REQUIRE(second.create(loopback, port));
    REQUIRE(second.getFd() >= 0);
#else
    REQUIRE_FALSE(second.create(loopback, port));
    REQUIRE(second.getFd() < 0);
#endif
}


TEST_CASE("UdpConnection sendData on closed connection returns false", "[UdpConnection]")
{
    ulong loopback = 0;
    REQUIRE(NetUtils::stringIpToLong("127.0.0.1", loopback));

    UdpConnection conn;
    // Never created — FD is -1.
    REQUIRE(conn.getFd() < 0);

    const byte payload[] = {0x01, 0x02, 0x03};
    REQUIRE_FALSE(conn.sendData(loopback, htons(12345), payload, sizeof(payload)));

    // Now create it, then close it, then try again
    REQUIRE(conn.create(loopback, 0));
    REQUIRE(conn.getFd() >= 0);
    conn.close();
    REQUIRE(conn.getFd() < 0);

    REQUIRE_FALSE(conn.sendData(loopback, htons(12345), payload, sizeof(payload)));
}


TEST_CASE("UdpConnection receiveData on closed connection returns false", "[UdpConnection]")
{
    UdpConnection conn;

    byte buffer[32];
    ulong srcIp = 0;
    ushort srcPort = 0;
    uint dataLength = 0;
    REQUIRE_FALSE(conn.receiveData(buffer, sizeof(buffer), srcIp, srcPort, dataLength));
}


TEST_CASE("UdpConnection create can be called twice (rebinds)", "[UdpConnection]")
{
    ulong loopback = 0;
    REQUIRE(NetUtils::stringIpToLong("127.0.0.1", loopback));

    UdpConnection conn;
    REQUIRE(conn.create(loopback, 0));
    int firstFd = conn.getFd();
    REQUIRE(firstFd >= 0);

    // Recreating closes the old FD and opens a new one.
    REQUIRE(conn.create(loopback, 0));
    REQUIRE(conn.getFd() >= 0);
    // Not guaranteed that the second FD differs from the first, but the
    // connection must still be usable.

    ulong ip = 0;
    ushort port = 0;
    REQUIRE(NetUtils::getLocalEndpoint(conn.getFd(), ip, port));
    REQUIRE(port != 0);
}


TEST_CASE("UdpConnection blocking / nonBlocking toggles return true on valid FD", "[UdpConnection]")
{
    ulong loopback = 0;
    REQUIRE(NetUtils::stringIpToLong("127.0.0.1", loopback));

    UdpConnection conn;
    REQUIRE(conn.create(loopback, 0));

    REQUIRE(conn.blocking());
    REQUIRE(conn.nonBlocking());
}


TEST_CASE("UdpConnection sendBufferFull is false after a successful send", "[UdpConnection]")
{
    ulong loopback = 0;
    REQUIRE(NetUtils::stringIpToLong("127.0.0.1", loopback));

    UdpConnection receiver;
    REQUIRE(receiver.create(loopback, 0));

    ulong ip = 0;
    ushort port = 0;
    REQUIRE(NetUtils::getLocalEndpoint(receiver.getFd(), ip, port));

    UdpConnection sender;
    REQUIRE(sender.create(loopback, 0));

    const byte payload[] = {'z'};
    REQUIRE(sender.sendData(loopback, port, payload, sizeof(payload)));
    REQUIRE_FALSE(sender.sendBufferFull());
}
