/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <MulticastUdpConnection.h>
#include <Log.h>
#include <NetUtils.h>

#include <Platform.h>
#include <cstring>



MulticastUdpConnection::MulticastUdpConnection (const string & multicastGroup,
                                                  ushort multicastPort)
    : multicastGroup_(multicastGroup),
      multicastPort_(multicastPort),
      isIPv6_(false)
{
    memset(&groupAddr_,  0, sizeof(groupAddr_));
    memset(&groupAddr6_, 0, sizeof(groupAddr6_));
    isIPv6_ = isIPv6Group();
}



MulticastUdpConnection::~MulticastUdpConnection ()
{
    close();
}



bool
MulticastUdpConnection::isIPv6Group () const
{
    return multicastGroup_.contains(':');
}



bool
MulticastUdpConnection::createIPv4 (int fd)
{
    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr(multicastGroup_.c_str());
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);

    if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                   &mreq, sizeof(mreq)) < 0) {
        string errorCode;
        NetUtils::socketErrorAsString(alpine_socket_errno(), errorCode);
        Log::Error("Failed to join multicast group "s + multicastGroup_ +
                   ": " + errorCode +
                   " in MulticastUdpConnection::create.");
        return false;
    }

    int ttl = 1;
    setsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));

    int loop = 0;
    setsockopt(fd, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop));

    memset(&groupAddr_, 0, sizeof(groupAddr_));
    groupAddr_.sin_family      = AF_INET;
    groupAddr_.sin_addr.s_addr = inet_addr(multicastGroup_.c_str());
    groupAddr_.sin_port        = htons(multicastPort_);

    return true;
}



bool
MulticastUdpConnection::createIPv6 (int fd)
{
    struct sockaddr_in6 mcastAddr;
    memset(&mcastAddr, 0, sizeof(mcastAddr));
    if (inet_pton(AF_INET6, multicastGroup_.c_str(), &mcastAddr.sin6_addr) != 1) {
        Log::Error("Invalid IPv6 multicast address: "s + multicastGroup_);
        return false;
    }

    struct ipv6_mreq mreq6;
    mreq6.ipv6mr_multiaddr = mcastAddr.sin6_addr;
    mreq6.ipv6mr_interface = 0;

    if (setsockopt(fd, IPPROTO_IPV6, IPV6_JOIN_GROUP,
                   &mreq6, sizeof(mreq6)) < 0) {
        string errorCode;
        NetUtils::socketErrorAsString(alpine_socket_errno(), errorCode);
        Log::Error("Failed to join IPv6 multicast group "s + multicastGroup_ +
                   ": " + errorCode +
                   " in MulticastUdpConnection::create.");
        return false;
    }

    int hopLimit = 1;
    setsockopt(fd, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, &hopLimit, sizeof(hopLimit));

    int loop = 0;
    setsockopt(fd, IPPROTO_IPV6, IPV6_MULTICAST_LOOP, &loop, sizeof(loop));

    memset(&groupAddr6_, 0, sizeof(groupAddr6_));
    groupAddr6_.sin6_family = AF_INET6;
    groupAddr6_.sin6_addr   = mcastAddr.sin6_addr;
    groupAddr6_.sin6_port   = htons(multicastPort_);

    return true;
}



bool
MulticastUdpConnection::create (ulong ipAddress, ushort port)
{
    // Create the base UDP socket bound to INADDR_ANY on the multicast port.
    // ipAddress param is ignored — multicast binds to INADDR_ANY.
    // port param is ignored — we use multicastPort_.
    //
    if (!UdpConnection::create(0, htons(multicastPort_)))
        return false;

    int fd = getFd();

    if (isIPv6_) {
        // For IPv6 multicast groups, join the group on this socket.
        // The base socket is AF_INET; full native IPv6 socket support
        // requires refactoring UdpConnection to support AF_INET6.
        // On dual-stack systems, the IPv6 group join may work on an
        // AF_INET socket if the OS supports it, otherwise it will
        // log an error and the connection remains IPv4-only.
        if (!createIPv6(fd)) {
            Log::Info("MulticastUdpConnection: IPv6 multicast join failed, "
                      "continuing with IPv4 only.");
        } else {
            Log::Info("MulticastUdpConnection: Joined IPv6 group "s +
                      multicastGroup_ + ":" + std::to_string(multicastPort_));
        }
    }
    else {
        if (!createIPv4(fd)) {
            close();
            return false;
        }

        Log::Info("MulticastUdpConnection: Joined group "s + multicastGroup_ +
                  ":" + std::to_string(multicastPort_));
    }

    return true;
}



bool
MulticastUdpConnection::sendData (const ulong    destIpAddress,
                                   const ushort   destPort,
                                   const byte *   dataBuffer,
                                   const uint     dataLength)
{
    // Always send to the multicast group address, ignoring the
    // destination parameters (which come from DTCP's IP/port addressing)
    return UdpConnection::sendData(groupAddr_.sin_addr.s_addr,
                                   groupAddr_.sin_port,
                                   dataBuffer,
                                   dataLength);
}
