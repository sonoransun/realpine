/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <DtcpBaseUdpTransport.h>


class AlpineDtcpUdpTransport : public DtcpBaseUdpTransport
{
  public:
    AlpineDtcpUdpTransport(const ulong ipAddress, const int port);

    virtual ~AlpineDtcpUdpTransport();


    virtual bool createMux(DtcpBaseConnMux *& connMux);

    virtual bool handleData(const byte * data, uint dataLength);
};
