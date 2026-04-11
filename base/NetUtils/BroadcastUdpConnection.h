/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <UdpConnection.h>


class BroadcastUdpConnection : public UdpConnection
{
  public:
    BroadcastUdpConnection();
    ~BroadcastUdpConnection() override;

    bool create(ulong ipAddress = 0, ushort port = 0) override;
};
