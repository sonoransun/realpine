/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <AutoThread.h>


class DtcpBaseUdpTransport;


class DtcpMonitorThread : public AutoThread
{
  public:

    DtcpMonitorThread (DtcpBaseUdpTransport * udpTransport);

    virtual ~DtcpMonitorThread ();



    virtual void threadMain ();


  private:

    DtcpBaseUdpTransport *  udpTransport_;

};

