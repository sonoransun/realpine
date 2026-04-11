/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <AutoThread.h>
#include <Common.h>


class DtcpBaseUdpTransport;


class DtcpStackThread : public AutoThread
{
  public:
    DtcpStackThread(DtcpBaseUdpTransport * udpTransport);

    virtual ~DtcpStackThread();


    virtual void threadMain();


  private:
    DtcpBaseUdpTransport * udpTransport_;
};
