/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <Platform.h>
#include <NetUtils.h>
#include <memory>


class TcpTransport;


class TcpAcceptor
{
  public:

    TcpAcceptor ();
    ~TcpAcceptor ();


    bool create (ulong  ipAddress, // 0 == All local addrs
                 ushort port);

    bool getEndpoint (ulong &   ipAddress,
                      ushort &  port);

    void close ();

    int  getFd ();

    bool nonBlocking ();

    bool blocking ();

    bool accept (std::unique_ptr<TcpTransport> &  transport);



  private:

    int                 socketFd_;
    ulong               ipAddress_;
    ushort              port_;
    struct sockaddr_in  socketAddress_;
    socklen_t           addressSize_;

};

