/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>


class StackLinkInterface;
class TransportInterface;
class MuxInterface;


class AcceptorInterface
{
  public:

    AcceptorInterface () = default;
    virtual ~AcceptorInterface ();


    virtual bool acceptConnection (StackLinkInterface * request) = 0;
     
    virtual bool createTransport (TransportInterface *& transport) = 0;

    virtual bool receiveTransport (TransportInterface * transport) = 0;

};


