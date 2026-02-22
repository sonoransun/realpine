/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <DtcpBaseConnAcceptor.h>
#include <DtcpBaseConnTransport.h>


class AlpineDtcpConnAcceptor : public DtcpBaseConnAcceptor
{
  public:

    AlpineDtcpConnAcceptor ();
    virtual ~AlpineDtcpConnAcceptor ();


    virtual bool acceptConnection (ulong   ipAddress,
                                   ushort  port);

    virtual bool createTransport (DtcpBaseConnTransport *& transport);

    virtual bool receiveTransport (DtcpBaseConnTransport * transport);

};


