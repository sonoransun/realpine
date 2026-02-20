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



