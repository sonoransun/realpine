/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <MulticastUdpConnection.h>
#include <Log.h>
#include <NetUtils.h>

#include <Platform.h>
#include <cstring>



MulticastUdpConnection::MulticastUdpConnection (const string & multicastGroup,
                                                  ushort multicastPort)
    : multicastGroup_(multicastGroup),
      multicastPort_(multicastPort)
{
    memset(&groupAddr_, 0, sizeof(groupAddr_));
}



MulticastUdpConnection::~MulticastUdpConnection ()
{
    close();
}



bool
MulticastUdpConnection::create (ulong ipAddress, ushort port)
{
    // Create the base UDP socket bound to INADDR_ANY on the multicast port.
    // ipAddress param is ignored — multicast binds to INADDR_ANY.
    // port param is ignored — we use multicastPort_.
    //
    if (!UdpConnection::create(0, htons(multicastPort_))) {
        return false;
    }

    int fd = getFd();

    // Join multicast group
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
        close();
        return false;
    }

    // Set TTL=1 (link-local only)
    int ttl = 1;
    setsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));

    // Disable loopback
    int loop = 0;
    setsockopt(fd, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop));

    // Prepare the multicast group destination address for sendData
    memset(&groupAddr_, 0, sizeof(groupAddr_));
    groupAddr_.sin_family      = AF_INET;
    groupAddr_.sin_addr.s_addr = inet_addr(multicastGroup_.c_str());
    groupAddr_.sin_port        = htons(multicastPort_);

    Log::Info("MulticastUdpConnection: Joined group "s + multicastGroup_ +
              ":" + std::to_string(multicastPort_));

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
