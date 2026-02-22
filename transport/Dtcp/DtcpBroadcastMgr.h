/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <ReadWriteSem.h>
#include <list>
#include <OptHash.h>


class DtcpBaseUdpTransport;
class DtcpBaseConnTransport;
class DtcpBroadcast;
class DtcpBroadcastSet;
class DtcpBroadcastStates;
class StackLinkInterface;


// This static class ties the per UDP transport instances
// to their broadcast requests.  All methods are private,
// as this class should only be used by the public interface
// classes
//
class DtcpBroadcastMgr
{
  public:

    DtcpBroadcastMgr () = default;
    ~DtcpBroadcastMgr () = default;



    static bool  initialize ();


    // Internal types
    //

    // Utility index used during processing of new reqeusts.
    //
    using t_TransportList = list<DtcpBaseConnTransport *>;

    using t_UdpTransportIndex = std::unordered_map<void *,  // DtcpBaseUdpTransport *
                     t_TransportList *,
                     OptHash<void *>,
                     equal_to<void *> >;

    using t_UdpTransportIndexPair = std::pair<void *, t_TransportList *>;



    // Index to map DtcpBaseUdpTransport pointer addresses to 
    // the DtcpBroadcastStates object handling broadcasts for
    // the transport.
    //
    using t_BroadcastStatesIndex = std::unordered_map<void *,  // DtcpBaseUdpTransport *
                     DtcpBroadcastStates *,
                     OptHash<void *>,
                     equal_to<void *> >;

    using t_BroadcastStatesIndexPair = std::pair<void *, DtcpBroadcastStates *>;


    // Broadcast request state to maintain status of ongoing
    // broadcast operation (can occur over multiple UdpTransports).
    //
    using t_BroadcastStatesList = list<DtcpBroadcastStates *>;

    struct t_BroadcastRequest {
        ulong                    requestId;
        DtcpBroadcast *          requestor;
        StackLinkInterface *     packet;
        ulong                    totalPackets;
        ulong                    packetsSent;
        t_BroadcastStatesList *  memberStatesList;
    };
       

    // Index to map request ID's to their respective
    // t_BroadcastRequest objects
    //
    using t_RequestIdIndex = std::unordered_map<ulong,
                     t_BroadcastRequest *,
                     OptHash<ulong>,
                     equal_to<ulong> >;

    using t_RequestIdIndexPair = std::pair<ulong, t_BroadcastRequest *>;


 
  private:

    static bool                        initialized_s;
    static ulong                       currRequestId_s;
    static t_BroadcastStatesIndex *    broadcastStatesIndex_s;
    static t_RequestIdIndex *          requestIdIndex_s;
    static ReadWriteSem                dataLock_s;



    static bool  sendPacket (DtcpBroadcast *      requestor,
                             StackLinkInterface * packet,
                             DtcpBroadcastSet *   destinations,
                             ulong &              requestId);

    static bool  populateTransportIndex (DtcpBroadcastSet *     destinations,
                                         t_UdpTransportIndex *  index);

    static bool  getStatus (ulong    requestId,
                            ulong &  packetsSent);           

    static bool  cancel (ulong  requestId);


    // Used by DtcpSendQueue to obtain its BroadcastStates object
    //
    static bool  getBroadcastStates (DtcpBaseUdpTransport *  transport,
                                     DtcpBroadcastStates *&  broadcastStates);

    static bool  packetSent (ulong  requestId,
                             ulong  transportId);



    friend class DtcpBroadcast;
    friend class DtcpBroadcastStates;
    friend class DtcpSendQueue;
};

