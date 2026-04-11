/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AlpineDtcpConnAcceptor.h>
#include <AlpineDtcpConnConnector.h>
#include <AlpineDtcpConnMux.h>
#include <Log.h>
#include <StringUtils.h>
#include <memory>


AlpineDtcpConnMux::AlpineDtcpConnMux()
{
#ifdef _VERBOSE
    Log::Debug("AlpineDtcpConnMux constructor invoked.");
#endif
}


AlpineDtcpConnMux::~AlpineDtcpConnMux()
{
#ifdef _VERBOSE
    Log::Debug("AlpineDtcpConnMux destructor invoked.");
#endif
}


bool
AlpineDtcpConnMux::createAcceptor(DtcpBaseConnAcceptor *& acceptor)
{
#ifdef _VERBOSE
    Log::Debug("AlpineDtcpConnMux::createAcceptor invoked.");
#endif

    auto alpineAcceptor = std::make_unique<AlpineDtcpConnAcceptor>();

    acceptor = static_cast<DtcpBaseConnAcceptor *>(alpineAcceptor.release());


    return true;
}


bool
AlpineDtcpConnMux::createConnector(DtcpBaseConnConnector *& connector)
{
#ifdef _VERBOSE
    Log::Debug("AlpineDtcpConnMux::createConnector invoked.");
#endif

    auto alpineConnector = std::make_unique<AlpineDtcpConnConnector>();

    bool status;
    DtcpBaseConnMux * dtcpMux;
    dtcpMux = static_cast<DtcpBaseConnMux *>(this);
    TransportInterface * parent;

    status = getParentTransport(parent);

    if (!status) {
        Log::Error("Could not retrieve parent transport in AlpineDtcpConnMux::createConnector."s);
        return false;
    }

    status = alpineConnector->initialize(dtcpMux, parent);

    if (!status) {
        Log::Error("Could not initialize connector in AlpineDtcpConnMux::createConnector."s);
        return false;
    }

    connector = static_cast<DtcpBaseConnConnector *>(alpineConnector.release());


    return true;
}
