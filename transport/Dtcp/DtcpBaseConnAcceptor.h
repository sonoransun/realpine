/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <AcceptorInterface.h>


class MuxInterface;
class TransportInterface;
class DtcpBaseConnTransport;


class DtcpBaseConnAcceptor : public AcceptorInterface
{
  public:

    DtcpBaseConnAcceptor ();
    virtual ~DtcpBaseConnAcceptor ();


    // AcceptorInterface operations
    //
    virtual bool initialize (MuxInterface *        multiplexor,
                             TransportInterface *  parentTransport);


    virtual bool acceptConnection (StackLinkInterface * request);

    virtual bool createTransport (TransportInterface *& transport);

    virtual bool receiveTransport (TransportInterface * transport);



    // derived implementation handlers
    //
    virtual bool acceptConnection (ulong   ipAddress,
                                   ushort  port) = 0;

    virtual bool createTransport (DtcpBaseConnTransport *& transport) = 0;

    virtual bool receiveTransport (DtcpBaseConnTransport * transport) = 0;


  protected:

    MuxInterface *        mux_;
    TransportInterface *  parentTransport_;

};


