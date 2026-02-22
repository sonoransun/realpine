/// Copyright (C) 2026 sonoransun — see LICENCE.txt


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


