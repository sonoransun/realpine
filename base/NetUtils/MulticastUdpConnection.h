/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <UdpConnection.h>


class MulticastUdpConnection : public UdpConnection
{
  public:

    MulticastUdpConnection (const string & multicastGroup, ushort multicastPort);
    ~MulticastUdpConnection () override;

    bool create (ulong ipAddress = 0, ushort port = 0) override;

    bool sendData (const ulong    destIpAddress,
                   const ushort   destPort,
                   const byte *   dataBuffer,
                   const uint     dataLength) override;

  private:

    bool isIPv6Group () const;
    bool createIPv4 (int fd);
    bool createIPv6 (int fd);

    string              multicastGroup_;
    ushort              multicastPort_;
    bool                isIPv6_;
    struct sockaddr_in  groupAddr_;
    struct sockaddr_in6 groupAddr6_;

};
