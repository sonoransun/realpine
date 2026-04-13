/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <DtcpBaseUdpTransport.h>


class AlpineRtlSdrUdpTransport : public DtcpBaseUdpTransport
{
  public:
    AlpineRtlSdrUdpTransport(const ulong ipAddress,
                             const int port,
                             uint centerFreqHz,
                             uint sampleRate,
                             int gainTenths,
                             const string & modulation);

    ~AlpineRtlSdrUdpTransport() override;


    bool createMux(DtcpBaseConnMux *& connMux) override;

    bool handleData(const byte * data, uint dataLength) override;

  protected:
    UdpConnection * createConnection() override;

  private:
    uint centerFreqHz_;
    uint sampleRate_;
    int gainTenths_;
    string modulation_;
};
