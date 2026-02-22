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

    string              multicastGroup_;
    ushort              multicastPort_;
    struct sockaddr_in  groupAddr_;

};
