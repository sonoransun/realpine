/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <DtcpBaseConnConnector.h>
#include <DtcpBaseConnTransport.h>


class AlpineDtcpConnConnector : public DtcpBaseConnConnector
{
  public:

    AlpineDtcpConnConnector ();
    virtual ~AlpineDtcpConnConnector ();


    virtual bool createTransport (DtcpBaseConnTransport *& transport);

    virtual bool receiveTransport (DtcpBaseConnTransport * transport);

    virtual bool handleRequestFailure (ulong   ipAddress,
                                       ushort  port);


};


