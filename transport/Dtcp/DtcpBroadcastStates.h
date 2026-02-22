/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <ReadWriteSem.h>
#include <list>


class DtcpBaseConnTransport;
class StackLinkInterface;


class DtcpBroadcastStates
{
  public:

    DtcpBroadcastStates ();
    ~DtcpBroadcastStates ();


    using t_TransportList = list<DtcpBaseConnTransport *>;


    bool addState (ulong                 requestId,
                   StackLinkInterface *  packet,
                   t_TransportList *     destinations);

    // Expensive operation, use cautiously.
    //
    bool removeState (ulong  requestId);


    ulong  requestsPending ();



    // Internal types
    //
    struct t_RequestData {
        ulong                 requestId;
        StackLinkInterface *  packet;
        t_TransportList *     remainingDestinations;
    };

    using t_RequestList = list<t_RequestData *>;

    
  private:

    ulong             totalRequests_;
    t_RequestList     requestList_;
    ReadWriteSem      dataLock_;


    bool  getCurrentRequest (StackLinkInterface *&     packet,
                             DtcpBaseConnTransport *&  destination);


    friend class DtcpSendQueue;
};

