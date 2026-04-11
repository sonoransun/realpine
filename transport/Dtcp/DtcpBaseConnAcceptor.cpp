/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <DtcpBaseConnAcceptor.h>
#include <DtcpBaseConnTransport.h>
#include <DtcpPacket.h>
#include <Log.h>
#include <StringUtils.h>


DtcpBaseConnAcceptor::DtcpBaseConnAcceptor()
{
#ifdef _VERBOSE
    Log::Debug("DtcpBaseConnAcceptor constructor invoked.");
#endif

    mux_ = nullptr;
    parentTransport_ = nullptr;
}


DtcpBaseConnAcceptor::~DtcpBaseConnAcceptor()
{
#ifdef _VERBOSE
    Log::Debug("DtcpBaseConnAcceptor destructor invoked.");
#endif
}


bool
DtcpBaseConnAcceptor::initialize(MuxInterface * multiplexor, TransportInterface * parentTransport)
{
#ifdef _VERBOSE
    Log::Debug("DtcpBaseConnAcceptor::initialize invoked.");
#endif

    mux_ = multiplexor;
    parentTransport_ = parentTransport;

    return true;
}


bool
DtcpBaseConnAcceptor::acceptConnection(StackLinkInterface * request)
{
#ifdef _VERBOSE
    Log::Debug("DtcpBaseConnAcceptor::acceptConnection invoked.");
#endif

    DtcpPacket * dtcpPacket;

    dtcpPacket = dynamic_cast<DtcpPacket *>(request);

    if (!dtcpPacket) {
        return false;
    }

    bool status;
    ulong ipAddress;
    ushort port;

    status = dtcpPacket->getPeerLocation(ipAddress, port);

    if (!status) {
        // Invalid packet?
        return false;
    }

    status = this->acceptConnection(ipAddress, port);

    return status;
}


bool
DtcpBaseConnAcceptor::createTransport(TransportInterface *& transport)
{
#ifdef _VERBOSE
    Log::Debug("DtcpBaseConnAcceptor::createTransport invoked.");
#endif

    bool status;
    DtcpBaseConnTransport * connTransport;

    status = createTransport(connTransport);

    if (status == false) {
        Log::Error("Derived createTransport failed in DtcpBaseConnAcceptor::createTransport.");
        return false;
    }

    transport = static_cast<TransportInterface *>(connTransport);


    return true;
}


bool
DtcpBaseConnAcceptor::receiveTransport(TransportInterface * transport)
{
#ifdef _VERBOSE
    Log::Debug("DtcpBaseConnAcceptor::receiveTransport invoked.");
#endif

    bool status;
    DtcpBaseConnTransport * connTransport;

    connTransport = dynamic_cast<DtcpBaseConnTransport *>(transport);

    if (!connTransport) {
        Log::Error("Invalid transport type passed to DtcpBaseConnAcceptor::receiveTransport.");
        return false;
    }

    status = receiveTransport(connTransport);


    return status;
}
