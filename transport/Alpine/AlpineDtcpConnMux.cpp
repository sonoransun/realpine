/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AlpineDtcpConnMux.h>
#include <AlpineDtcpConnAcceptor.h>
#include <AlpineDtcpConnConnector.h>
#include <Log.h>
#include <StringUtils.h>


AlpineDtcpConnMux::AlpineDtcpConnMux ()
{
#ifdef _VERBOSE
    Log::Debug ("AlpineDtcpConnMux constructor invoked.");
#endif
}


    
AlpineDtcpConnMux::~AlpineDtcpConnMux ()
{
#ifdef _VERBOSE
    Log::Debug ("AlpineDtcpConnMux destructor invoked.");
#endif
}



bool 
AlpineDtcpConnMux::createAcceptor (DtcpBaseConnAcceptor *&  acceptor)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineDtcpConnMux::createAcceptor invoked.");
#endif

    AlpineDtcpConnAcceptor * alpineAcceptor;
    alpineAcceptor = new AlpineDtcpConnAcceptor ();

    acceptor = static_cast<DtcpBaseConnAcceptor *>(alpineAcceptor);


    return true;
}



bool 
AlpineDtcpConnMux::createConnector (DtcpBaseConnConnector *&  connector)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineDtcpConnMux::createConnector invoked.");
#endif

    AlpineDtcpConnConnector * alpineConnector;
    alpineConnector =  new AlpineDtcpConnConnector ();

    bool status;
    DtcpBaseConnMux * dtcpMux;
    dtcpMux = static_cast<DtcpBaseConnMux *>(this);
    TransportInterface * parent;

    status = getParentTransport (parent);

    if (!status) {
        Log::Error ("Could not retrieve parent transport in AlpineDtcpConnMux::createConnector.");
        return false;
    }

    status = alpineConnector->initialize (dtcpMux, parent);

    if (!status) {
        Log::Error ("Could not initialize connector in AlpineDtcpConnMux::createConnector.");
        return false;
    }

    connector =  static_cast<DtcpBaseConnConnector *>(alpineConnector);


    return true;
}



