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



