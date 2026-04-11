/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <NetUtils.h>
#include <Platform.h>


class TcpTransport
{
  public:
    ~TcpTransport();


    bool getDestination(ulong & ipAddress, ushort & port);

    bool getEndpoint(ulong & ipAddress, ushort & port);

    void close();

    int getFd();

    bool nonBlocking();

    bool blocking();

    bool send(const byte * data, ulong dataLength);

    bool receive(byte * buffer, ulong bufferSize, ulong & dataLength);


  private:
    TcpTransport(int socketFd, ulong destIpAddress, ushort destPort, ulong localIpAddress, ushort localPort);


    int socketFd_;
    ulong destIpAddress_;
    ushort destPort_;
    ulong localIpAddress_;
    ushort localPort_;


    friend class TcpAcceptor;
    friend class TcpConnector;
    friend class TcpAsyncConnector;
};
