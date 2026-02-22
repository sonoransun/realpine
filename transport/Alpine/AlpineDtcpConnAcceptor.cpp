/// Copyright (C) 2026 sonoransun — see LICENCE.txt



#include <AlpineDtcpConnAcceptor.h>
#include <AlpineDtcpConnTransport.h>
#include <AlpineStack.h>
#include <Log.h>
#include <StringUtils.h>
#include <NetUtils.h>



AlpineDtcpConnAcceptor::AlpineDtcpConnAcceptor ()
{
#ifdef _VERBOSE
    Log::Debug ("AlpineDtcpConnAcceptor constructor invoked.");
#endif
}



AlpineDtcpConnAcceptor::~AlpineDtcpConnAcceptor ()
{
#ifdef _VERBOSE
    Log::Debug ("AlpineDtcpConnAcceptor destructor invoked.");
#endif
}



bool 
AlpineDtcpConnAcceptor::acceptConnection (ulong   ipAddress,
                                          ushort  port)
{
#ifdef _VERBOSE
    string  ipAddressStr;
    NetUtils::longIpToString (ipAddress, ipAddressStr);

    Log::Debug ("AlpineDtcpConnAcceptor::acceptConnection invoked.  Parameters:"s +
                "\nIP Address: "s + ipAddressStr +
                "\nPort: "s + std::to_string (ntohs(port)) );
#endif

    return true;
}



bool 
AlpineDtcpConnAcceptor::createTransport (DtcpBaseConnTransport *& transport)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineDtcpConnAcceptor::createTransport invoked.");
#endif

    AlpineDtcpConnTransport * alpineTransport;
    alpineTransport =  new AlpineDtcpConnTransport ();

    transport = static_cast<DtcpBaseConnTransport *>(alpineTransport);


    return true;
}



bool 
AlpineDtcpConnAcceptor::receiveTransport (DtcpBaseConnTransport * transport)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineDtcpConnAcceptor::receiveTransport invoked.");
#endif


    // downcast to AlpineDtcpConnTransport type.
    AlpineDtcpConnTransport * alpineTransport;
    alpineTransport =  dynamic_cast<AlpineDtcpConnTransport *>(transport);

    if (!alpineTransport) {
        // Invalid transport type???
        //
        Log::Error ("Invalid transport type passed to AlpineDtcpConnAcceptor::receiveTransport.");
        return false;
    }

    bool   status;
    ulong  peerId;

    transport->getTransportId (peerId);

    status = AlpineStack::registerTransport (peerId, alpineTransport);

    if (!status) {
        Log::Error ("Register transport failed in AlpineDtcpConnAcceptor::receiveTransport!");
        return false;
    }


    return true;
}



