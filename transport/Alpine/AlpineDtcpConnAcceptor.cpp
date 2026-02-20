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



