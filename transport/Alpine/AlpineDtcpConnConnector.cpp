/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AlpineDtcpConnConnector.h>
#include <AlpineDtcpConnTransport.h>
#include <AlpineStack.h>
#include <MuxInterface.h>
#include <Log.h>
#include <StringUtils.h>


AlpineDtcpConnConnector::AlpineDtcpConnConnector ()
{
#ifdef _VERBOSE
    Log::Debug ("AlpineDtcpConnConnector constructor invoked.");
#endif

}



AlpineDtcpConnConnector::~AlpineDtcpConnConnector ()
{
#ifdef _VERBOSE
    Log::Debug ("AlpineDtcpConnConnector destructor invoked.");
#endif

}



bool
AlpineDtcpConnConnector::createTransport (DtcpBaseConnTransport *& transport)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineDtcpConnConnector::createTransport invoked.");
#endif


    AlpineDtcpConnTransport * alpineTransport;
    alpineTransport =  new AlpineDtcpConnTransport ();

    transport = static_cast<DtcpBaseConnTransport *>(alpineTransport);


    return true;
}



bool 
AlpineDtcpConnConnector::receiveTransport (DtcpBaseConnTransport * transport)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineDtcpConnConnector::receiveTransport invoked.");
#endif

    // Cast back to AlpineDtcpConnTransport type.
    AlpineDtcpConnTransport * alpineTransport;
    alpineTransport =  dynamic_cast<AlpineDtcpConnTransport *>(transport);

    if (!alpineTransport) {
        // Invalid transport type?
        Log::Error ("Invalid transport type passed to AlpineDtcpConnConnector::receiveTransport.");

        return false;
    }

    bool   status;
    ulong  peerId;

    transport->getTransportId (peerId);

    status = AlpineStack::registerTransport (peerId, alpineTransport);

    if (!status) {
        Log::Error ("Register transport failed in AlpineDtcpConnConnector::receiveTransport!");
        return false;
    }
   

    return true;
}



bool 
AlpineDtcpConnConnector::handleRequestFailure (ulong   ipAddress,
                                               ushort  port)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineDtcpConnConnector::handleRequestFailure invoked.");
#endif

    return true;
}



