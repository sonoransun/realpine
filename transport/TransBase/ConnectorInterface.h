/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>


class StackLinkInterface;
class TransportInterface;
class MuxInterface;


class ConnectorInterface
{
  public:

    ConnectorInterface () = default;
    virtual ~ConnectorInterface ();



    virtual bool requestConnection () = 0;
     
    virtual bool createTransport (TransportInterface *& transport) = 0;

    virtual bool receiveTransport (TransportInterface * transport) = 0;

    virtual bool handleRequestFailure () = 0;

};


