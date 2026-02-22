/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <AlpineQueryOptions.h>
#include <AlpineQueryStatus.h>
#include <ReadWriteSem.h>
#include <OptHash.h>
#include <vector>


class AlpineQuery;
class AlpineQueryPacket;
class AlpineQueryResults;


class AlpineQueryMgr
{
  public:


    // Public types
    //
    using t_QueryIdList = vector<ulong>;


    // Public operations
    //
    static bool  createQuery (AlpineQueryOptions &  options,
                              ulong &               queryId);

    static bool  getQueryStatus (ulong                queryId,
                                 AlpineQueryStatus &  status);

    static bool  exists (ulong  queryId);

    static bool  isActive (ulong  queryId);

    static bool  inProgress (ulong  queryId);

    static bool  pauseQuery (ulong  queryId);

    static bool  resumeQuery (ulong  queryId);

    static bool  cancelQuery (ulong  queryId);

    static bool  getAllActiveQueryIds (t_QueryIdList  queryIdList);

    static bool  getAllPastQueryIds (t_QueryIdList  queryIdList);

    static bool  getQueryResults (ulong                  queryId,
                                  AlpineQueryResults *&  results);

    static bool  getPeerActiveQueryList (ulong            peerId,
                                         t_QueryIdList &  queryIdList);

    static bool  getPeerPastQueryList (ulong            peerId,
                                       t_QueryIdList &  queryIdList);



    // Internal types
    //
    using t_PendingReplyIndex = std::unordered_map< ulong, // peer ID
                      ulong, // reliable request ID
                      OptHash<ulong>,
                      equal_to<ulong> >;

    using t_PendingReplyIndexPair = std::pair<ulong, ulong>;


    struct t_QueryState {
        ulong                  queryId;
        AlpineQuery *          query;  // only present while active
        AlpineQueryResults *   results;
        t_PendingReplyIndex *  pending;
    };

    using t_QueryStateIndex = std::unordered_map< ulong,   // query ID
                      t_QueryState *,
                      OptHash<ulong>,
                      equal_to<ulong> >;

    using t_QueryStateIndexPair = std::pair<ulong, t_QueryState *>;


    using t_PeerQueryIndex = std::unordered_map< ulong,   // peer ID
                      t_QueryIdList *,
                      OptHash<ulong>,
                      equal_to<ulong> >;

    using t_PeerQueryIndexPair = std::pair<ulong, t_QueryIdList *>;



  private:

    static bool                        initialized_s;
    static ulong                       maxConncurent_s;
    static ulong                       currQueryId_s;
    static t_QueryStateIndex *         activeQueryIndex_s;
    static t_QueryStateIndex *         pastQueryIndex_s;
    static t_PeerQueryIndex *          activePeerQueryIndex_s;
    static t_PeerQueryIndex *          pastPeerQueryIndex_s;
    static ReadWriteSem                dataLock_s;



    // Initialization performed by AlpineStack
    //
    static bool  initialize (ulong maxConcurrent);


    // Handlers for AlpineDtcpTransport events.
    //
    static bool  handleQueryDiscover (ulong                peerId,
                                      AlpineQueryPacket *  discoverPacket);

    static bool  handleQueryOffer (ulong                peerId,
                                   AlpineQueryPacket *  offerPacket);

    static bool  handleQueryRequest (ulong                peerId,
                                     AlpineQueryPacket *  requestPacket);

    static bool  handleQueryReply (ulong                peerId,
                                      AlpineQueryPacket *  replyPacket);

    static bool  handleSendReceived (ulong  peerId,
                                     ulong  requestId);

    static bool  handleSendFailure (ulong  peerId,
                                    ulong  requestId);

    static bool  cancelAll (ulong  peerId);


    // Timer and event management entry point
    // Used solely by the AlpineStack
    //
    static void  processTimedEvents ();


    static void  cleanUp ();


    // Helper methods
    //
    static bool  removePendingRequest (ulong  peerId,
                                       ulong  requestId);



    friend class AlpineStack;
    friend class AlpineDtcpConnTransport;
};


