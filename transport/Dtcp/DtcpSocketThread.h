/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <AutoThread.h>


class DtcpBaseUdpTransport;


class DtcpSocketThread : public AutoThread
{
  public:

    DtcpSocketThread (DtcpBaseUdpTransport * udpTransport);

    virtual ~DtcpSocketThread ();



    virtual void threadMain ();


  private:

    DtcpBaseUdpTransport *  udpTransport_;

};

