/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>


class StackLinkInterface;
class TransportInterface;
class AcceptorInterface;
class ConnectorInterface;


class MuxInterface
{
  public:

    MuxInterface () = default;
    virtual ~MuxInterface ();


    virtual bool processPacket (StackLinkInterface * packet) = 0;

    virtual bool requestTransport (ConnectorInterface * requestor,
                                   TransportInterface * transport) = 0;

};


