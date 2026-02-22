/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <DtcpBaseUdpTransport.h>


class AlpineBroadcastUdpTransport : public DtcpBaseUdpTransport
{
  public:

    AlpineBroadcastUdpTransport (const ulong   ipAddress,
                                 const int     port);

    ~AlpineBroadcastUdpTransport () override;


    bool createMux (DtcpBaseConnMux *& connMux) override;

    bool handleData (const byte * data,
                     uint         dataLength) override;

  protected:

    UdpConnection * createConnection () override;

};
