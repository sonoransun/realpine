/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <AutoThread.h>


class DtcpBaseUdpTransport;


class DtcpStackThread : public AutoThread
{
  public:

    DtcpStackThread (DtcpBaseUdpTransport *  udpTransport);

    virtual ~DtcpStackThread ();



    virtual void threadMain ();



  private:

    DtcpBaseUdpTransport *  udpTransport_;

};

