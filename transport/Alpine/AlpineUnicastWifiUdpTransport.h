/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <DtcpBaseUdpTransport.h>


class AlpineUnicastWifiUdpTransport : public DtcpBaseUdpTransport
{
  public:
    AlpineUnicastWifiUdpTransport(const ulong ipAddress, const int port, const string & interfaceName);

    ~AlpineUnicastWifiUdpTransport() override;


    bool createMux(DtcpBaseConnMux *& connMux) override;

    bool handleData(const byte * data, uint dataLength) override;

  protected:
    UdpConnection * createConnection() override;

  private:
    string interfaceName_;
};
