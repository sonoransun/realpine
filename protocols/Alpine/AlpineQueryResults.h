/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <AlpineQueryPacket.h>
#include <AlpineQueryOptions.h>
#include <ReadWriteSem.h>
#include <OptHash.h>
#include <vector>


class AlpineResourceDescSet;


class AlpineQueryResults
{
  public:

    ~AlpineQueryResults ();


    // Types
    //
    using t_PeerIdList = vector<ulong>;


    bool  numResources (ulong & count);

    bool  numPending (ulong & pending);

    bool  numPeersResponding (ulong &  numPeers);

    bool  getPeerList (t_PeerIdList &  peerList);

    bool  getResourceList (ulong                                    peerId,
                           AlpineQueryPacket::t_ResourceDescList &  resourceList);



    // Internal types
    //
    struct t_ReplyState {
        ulong                    peerId;
        ulong                    expectedTotal;
        ushort                   duplicateCount;
        AlpineResourceDescSet *  resourceDescSet;
    };

    using t_ReplyIndex = std::unordered_map < ulong,
                       t_ReplyState *,
                       OptHash<ulong>,
                       equal_to<ulong> >;

    using t_ReplyIndexPair = std::pair <ulong, t_ReplyState *>;


  private:

    ulong                    queryId_;
    ulong                    expectedResourceTotal_;
    ulong                    currResourceTotal_;
    ulong                    numPeers_;
    t_ReplyIndex *           replyIndex_;
    AlpineQueryOptions       queryOptions_;
    ReadWriteSem             dataLock_;
    

    // Only private methods which should be accessed by AlpineQueryMgr
    //
    AlpineQueryResults (ulong                 queryId,
                        AlpineQueryOptions &  queryOptions);

    bool  processQueryOffer (ulong                peerId,
                             AlpineQueryPacket *  offer,
                             AlpineQueryPacket *  request);

    bool  processQueryReply (ulong                peerId,
                             AlpineQueryPacket *  reply,
                             AlpineQueryPacket *  request);

    

    // Internal, NON SYNC methods
    //
    bool  verifyQueryOffer (ulong                peerId,
                            AlpineQueryPacket *  offer);

    bool  verifyQueryReply (ulong                peerId,
                            AlpineQueryPacket *  reply);

    bool  sendQueryRequest (t_ReplyState *  state);

    bool  populateQueryRequest (t_ReplyState *       state,
                                AlpineQueryPacket *  request);


    // Copy constructor and assignment operator not implemented.
    //
    AlpineQueryResults (const AlpineQueryResults & copy);
    AlpineQueryResults & operator = (const AlpineQueryResults & copy);


    friend class AlpineQueryMgr;
};


