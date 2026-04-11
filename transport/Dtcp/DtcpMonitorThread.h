/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <AutoThread.h>
#include <Common.h>


class DtcpBaseUdpTransport;


class DtcpMonitorThread : public AutoThread
{
  public:
    DtcpMonitorThread(DtcpBaseUdpTransport * udpTransport);

    virtual ~DtcpMonitorThread();


    virtual void threadMain();


  private:
    DtcpBaseUdpTransport * udpTransport_;
};
