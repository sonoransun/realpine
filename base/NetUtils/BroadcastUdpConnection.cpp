/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <BroadcastUdpConnection.h>
#include <Log.h>
#include <NetUtils.h>



BroadcastUdpConnection::BroadcastUdpConnection ()
{
}



BroadcastUdpConnection::~BroadcastUdpConnection ()
{
    close();
}



bool
BroadcastUdpConnection::create (ulong ipAddress, ushort port)
{
    // Call base class to create and bind the socket
    if (!UdpConnection::create(ipAddress, port)) {
        return false;
    }

    // Enable broadcast on the socket
    int broadcast = 1;
    int fd = getFd();

    if (setsockopt(fd, SOL_SOCKET, SO_BROADCAST,
                   reinterpret_cast<const byte *>(&broadcast),
                   sizeof(broadcast)) < 0) {
        string errorCode;
        NetUtils::socketErrorAsString(alpine_socket_errno(), errorCode);
        Log::Error("Set SO_BROADCAST error: "s + errorCode +
                   " in BroadcastUdpConnection::create.");
        close();
        return false;
    }

    Log::Info("BroadcastUdpConnection: Socket created with SO_BROADCAST enabled.");

    return true;
}
