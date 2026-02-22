/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <DtcpBaseUdpTransport.h>


class AlpineMulticastUdpTransport : public DtcpBaseUdpTransport
{
  public:

    AlpineMulticastUdpTransport (const ulong   ipAddress,
                                 const int     port,
                                 const string & multicastGroup,
                                 ushort         multicastPort);

    ~AlpineMulticastUdpTransport () override;


    bool createMux (DtcpBaseConnMux *& connMux) override;

    bool handleData (const byte * data,
                     uint         dataLength) override;

  protected:

    UdpConnection * createConnection () override;

  private:

    string  multicastGroup_;
    ushort  multicastPort_;

};
