///////
///
///  Copyright (C) 2026  sonoransun
///
///  Permission is hereby granted, free of charge, to any person obtaining a copy
///  of this software and associated documentation files (the "Software"), to deal
///  in the Software without restriction, including without limitation the rights
///  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
///  copies of the Software, and to permit persons to whom the Software is
///  furnished to do so, subject to the following conditions:
///
///  The above copyright notice and this permission notice shall be included in all
///  copies or substantial portions of the Software.
///
///  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
///  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
///  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
///  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
///  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
///  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
///  SOFTWARE.
///
///////



#include <DtcpBaseConnConnector.h>
#include <DtcpBaseConnMux.h>
#include <DtcpBaseConnTransport.h>
#include <DtcpBaseUdpTransport.h>
#include <Log.h>
#include <StringUtils.h>



DtcpBaseConnConnector::DtcpBaseConnConnector ()
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnConnector constructor invoked.");
#endif

    destIpAddress_   = 0;
    destPort_        = 0;
    mux_             = nullptr;
    parentTransport_ = nullptr;
}



DtcpBaseConnConnector::~DtcpBaseConnConnector ()
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnConnector destructor invoked.");
#endif

}



bool 
DtcpBaseConnConnector::initialize (DtcpBaseConnMux *     multiplexor,
                                   TransportInterface *  parentTransport)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnConnector::initialize invoked.");
#endif

    destIpAddress_   = 0;
    destPort_        = 0;
    mux_             = multiplexor;
    parentTransport_ = parentTransport;


    return true;
}



bool 
DtcpBaseConnConnector::requestConnection ()
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnConnector::requestConnection invoked.");
#endif

    if (destIpAddress_ == 0) {
        Log::Error ("Connection requested without destination set.");
        return false;
    }


    bool status;
    DtcpBaseConnTransport * connTransport;
   
    status = createTransport (connTransport);

    if (!status) {
        Log::Error ("createTransport failed.");
        return false;
    }

    DtcpBaseUdpTransport * udpTransport;
    udpTransport = dynamic_cast<DtcpBaseUdpTransport *>(parentTransport_);

    if (!udpTransport) {
        // MRP_TEMP
        delete connTransport;
        return false;
    }

    connTransport->setParent (udpTransport);
    connTransport->setPeerLocation (destIpAddress_, destPort_);

    status = mux_->requestTransport (this, connTransport);


    return status;
}



bool
DtcpBaseConnConnector::createTransport (TransportInterface *& transport)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnConnector::createTransport invoked.");
#endif

    DtcpBaseConnTransport * connTransport;

    bool status;
    status = this->createTransport (connTransport);

    if (!status) {
        Log::Error ("Create transport for DtcpBaseConnTransport "
                             "failed in DtcpBaseConnConnector::createTransport.");
        return false;
    }

    transport = static_cast<TransportInterface *>(connTransport);


    return true;
}



bool 
DtcpBaseConnConnector::receiveTransport (TransportInterface * transport)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnConnector::receiveTransport invoked.");
#endif

    DtcpBaseConnTransport * connTransport;
    connTransport = dynamic_cast<DtcpBaseConnTransport *>(transport);

    if (!connTransport) {
        Log::Error ("Invalid transport type in DtcpBaseConnConnector::receiveTransport!");

        return false;
    }

    // Invoke derived handler...
    //
    bool status;
    status = receiveTransport (connTransport);


    return status;
}



bool 
DtcpBaseConnConnector::handleRequestFailure ()
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnConnector::handleRequestFailure invoked.");
#endif

    bool status;

    status = handleRequestFailure (destIpAddress_,
                                   destPort_);

    return status;
}



bool 
DtcpBaseConnConnector::setDestination (ulong  ipAddress,
                                       int    port)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnConnector::setDestination invoked.");
#endif

    destIpAddress_ = ipAddress;
    destPort_      = port;

    return true;
}



bool 
DtcpBaseConnConnector::getDestination (ulong &  ipAddress,
                                       int &    port)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnConnector::getDestination invoked.");
#endif

    ipAddress = destIpAddress_;
    port      = destPort_;

    return true;
}



bool 
DtcpBaseConnConnector::requestConnection (ulong  ipAddress,
                                          int    port)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnConnector::requestConnection invoked.");
#endif

    destIpAddress_ = ipAddress;
    destPort_      = port;

    bool status;

    status = requestConnection ();


    return status;
}




