/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <AutoThread.h>
#include <Common.h>


class DtcpBaseUdpTransport;


class DtcpSocketThread : public AutoThread
{
  public:
    DtcpSocketThread(DtcpBaseUdpTransport * udpTransport);

    virtual ~DtcpSocketThread();


    virtual void threadMain();


  private:
    DtcpBaseUdpTransport * udpTransport_;
};
