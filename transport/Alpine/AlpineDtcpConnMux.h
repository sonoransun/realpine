/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <DtcpBaseConnMux.h>


class AlpineDtcpConnMux : public DtcpBaseConnMux
{
  public:

    AlpineDtcpConnMux ();
    
    virtual ~AlpineDtcpConnMux ();



    virtual bool createAcceptor (DtcpBaseConnAcceptor *&  acceptor);

    virtual bool createConnector (DtcpBaseConnConnector *&  connector);

};

