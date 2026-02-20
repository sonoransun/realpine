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


#pragma once
#include <Common.h>
#include <ConnectorInterface.h>


class DtcpBaseConnMux;
class DtcpBaseConnTransport;


class DtcpBaseConnConnector : public ConnectorInterface
{
  public:

    DtcpBaseConnConnector ();

    virtual ~DtcpBaseConnConnector ();


   
    ////
    //
    // ConnectorInterface operations
    //
    virtual bool initialize (DtcpBaseConnMux *     multiplexor,
                             TransportInterface *  parentTransport);

    virtual bool requestConnection ();

    virtual bool createTransport (TransportInterface *& transport);

    virtual bool receiveTransport (TransportInterface * transport);

    virtual bool handleRequestFailure ();



    ////
    //
    // Dtcp specific operations
    //
    virtual bool setDestination (ulong  ipAddress,
                                 int    port);

    virtual bool getDestination (ulong &  ipAddress,
                                 int &    port);

    virtual bool requestConnection (ulong  ipAddress,
                                    int    port);


    ////
    // 
    // Derived handlers
    //
    virtual bool createTransport (DtcpBaseConnTransport *& transport) = 0;

    virtual bool receiveTransport (DtcpBaseConnTransport * transport) = 0;

    virtual bool handleRequestFailure (ulong   ipAddress,
                                       ushort  port) = 0;



  protected:

    ulong   destIpAddress_;
    int     destPort_;

    DtcpBaseConnMux *     mux_;
    TransportInterface *  parentTransport_;

};


