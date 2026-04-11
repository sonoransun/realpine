/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <NetUtils.h>
#include <Platform.h>


class TcpTransport;


class TcpConnector
{
  public:
    TcpConnector();
    ~TcpConnector();


    bool setDestination(ulong ipAddress, ushort port);

    bool getDestination(ulong & ipAddress, ushort & port);

    bool connect(TcpTransport *& transport);


  private:
    ulong destIpAddress_;
    ushort destPort_;
    struct sockaddr_in socketAddress_;
    int addressSize_;
};
