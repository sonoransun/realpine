/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <AlpineQueryOptions.h>
#include <AlpineQueryStatus.h>
#include <ReadWriteSem.h>
#include <OptHash.h>
#include <atomic>
#include <vector>
#include <memory>
#include <shared_mutex>
#include <functional>


class AlpineQuery;
class AlpineQueryPacket;
class AlpineQueryResults;


class AlpineQueryMgr
{
  public:


    // Public types
    //
    using t_QueryIdList = vector<ulong>;

    // Callback invoked when a query receives new results from a peer.
    // Parameters: queryId, peerId that just replied.
    using QueryResultCallback = std::function<void(ulong queryId, ulong peerId)>;


    // Public operations
    //
    [[nodiscard]] static bool  createQuery (AlpineQueryOptions &  options,
                              ulong &               queryId);

    [[nodiscard]] static bool  getQueryStatus (ulong                queryId,
                                 AlpineQueryStatus &  status);

    [[nodiscard]] static bool  exists (ulong  queryId);

    [[nodiscard]] static bool  isActive (ulong  queryId);

    [[nodiscard]] static bool  inProgress (ulong  queryId);

    [[nodiscard]] static bool  pauseQuery (ulong  queryId);

    [[nodiscard]] static bool  resumeQuery (ulong  queryId);

    [[nodiscard]] static bool  cancelQuery (ulong  queryId);

    [[nodiscard]] static bool  getAllActiveQueryIds (t_QueryIdList  queryIdList);

    [[nodiscard]] static bool  getAllPastQueryIds (t_QueryIdList  queryIdList);

    [[nodiscard]] static bool  getQueryResults (ulong                  queryId,
                                  AlpineQueryResults *&  results);

    static bool  registerResultCallback (ulong                queryId,
                                         QueryResultCallback  callback);

    [[nodiscard]] static bool  getPeerActiveQueryList (ulong            peerId,
                                         t_QueryIdList &  queryIdList);

    [[nodiscard]] static bool  getPeerPastQueryList (ulong            peerId,
                                       t_QueryIdList &  queryIdList);



    // Internal types
    //
    using t_PendingReplyIndex = std::unordered_map< ulong, // peer ID
                      ulong, // reliable request ID
                      OptHash<ulong>,
                      equal_to<ulong> >;

    using t_PendingReplyIndexPair = std::pair<ulong, ulong>;


    struct t_QueryState {
        ulong                                  queryId;
        std::unique_ptr<AlpineQuery>           query;    // only present while active
        std::unique_ptr<AlpineQueryResults>    results;
        std::unique_ptr<t_PendingReplyIndex>   pending;
        mutable std::shared_mutex              queryMutex;  // per-query lock for result accumulation
        QueryResultCallback                    onResultCallback;  // invoked when new results arrive
    };

    using t_QueryStateIndex = std::unordered_map< ulong,   // query ID
                      std::unique_ptr<t_QueryState>,
                      OptHash<ulong>,
                      equal_to<ulong> >;

    using t_QueryStateIndexPair = std::pair<ulong, std::unique_ptr<t_QueryState>>;


    using t_PeerQueryIndex = std::unordered_map< ulong,   // peer ID
                      std::unique_ptr<t_QueryIdList>,
                      OptHash<ulong>,
                      equal_to<ulong> >;

    using t_PeerQueryIndexPair = std::pair<ulong, std::unique_ptr<t_QueryIdList>>;



  private:

    static bool                                    initialized_s;
    static ulong                                   maxConncurent_s;
    static std::atomic<ulong>                      currQueryId_s;
    static std::unique_ptr<t_QueryStateIndex>      activeQueryIndex_s;
    static std::unique_ptr<t_QueryStateIndex>      pastQueryIndex_s;
    static std::unique_ptr<t_PeerQueryIndex>       activePeerQueryIndex_s;
    static std::unique_ptr<t_PeerQueryIndex>       pastPeerQueryIndex_s;

    // Lock ordering (to prevent deadlocks):
    //   activeQueryLock_s > pastQueryLock_s > queryMutex (per-query)
    // Never acquire a lock that precedes one you already hold.
    //
    // Split locks: active queries (hot path) vs past queries (cold, read-mostly)
    static ReadWriteSem                            activeQueryLock_s;
    static ReadWriteSem                            pastQueryLock_s;



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
    // Returns list of query IDs that completed this cycle (for async callback notification).
    //
    static std::vector<ulong>  processTimedEvents ();


    static void  cleanUp ();


    // Helper methods
    //
    static bool  removePendingRequest (ulong  peerId,
                                       ulong  requestId);



    friend class AlpineStack;
    friend class AlpineDtcpConnTransport;
};


