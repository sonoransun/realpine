/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <AlpineQueryOptions.h>
#include <AlpineQueryStatus.h>
#include <ReadWriteSem.h>
#include <OptHash.h>
#include <vector>
#include <memory>


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
    static ulong                                   currQueryId_s;
    static std::unique_ptr<t_QueryStateIndex>      activeQueryIndex_s;
    static std::unique_ptr<t_QueryStateIndex>      pastQueryIndex_s;
    static std::unique_ptr<t_PeerQueryIndex>       activePeerQueryIndex_s;
    static std::unique_ptr<t_PeerQueryIndex>       pastPeerQueryIndex_s;
    static ReadWriteSem                            dataLock_s;



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


