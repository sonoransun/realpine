/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <DtcpBaseUdpTransport.h>


class AlpineRawWifiUdpTransport : public DtcpBaseUdpTransport
{
  public:

    AlpineRawWifiUdpTransport (const ulong    ipAddress,
                                const int      port,
                                const string & interfaceName);

    ~AlpineRawWifiUdpTransport () override;


    bool createMux (DtcpBaseConnMux *& connMux) override;

    bool handleData (const byte * data,
                     uint         dataLength) override;

  protected:

    UdpConnection * createConnection () override;

  private:

    string  interfaceName_;

};
