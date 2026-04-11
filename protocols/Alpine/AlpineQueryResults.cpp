/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AlpinePeerMgr.h>
#include <AlpineProtocol.h>
#include <AlpineQueryResults.h>
#include <AlpineResourceDesc.h>
#include <AlpineResourceDescSet.h>
#include <Log.h>
#include <ReadLock.h>
#include <StringUtils.h>
#include <WriteLock.h>
#include <functional>
#include <limits.h>


AlpineQueryResults::AlpineQueryResults(ulong queryId, AlpineQueryOptions & queryOptions)
    : queryOptions_(queryOptions)
{
#ifdef _VERBOSE
    Log::Debug("AlpineQueryResults constructor invoked.");
#endif

    WriteLock lock(dataLock_);

    queryId_ = queryId;
    expectedResourceTotal_ = 0;
    currResourceTotal_ = 0;
    numPeers_ = 0;
    replyIndex_ = new t_ReplyIndex;
}


AlpineQueryResults::~AlpineQueryResults()
{
#ifdef _VERBOSE
    Log::Debug("AlpineQueryResults destructor invoked.");
#endif

    WriteLock lock(dataLock_);

    if (replyIndex_) {

        for (const auto & item : *replyIndex_) {
            delete item.second;
        }

        delete replyIndex_;
    }
}


bool
AlpineQueryResults::numResources(ulong & count)
{
#ifdef _VERBOSE
    Log::Debug("AlpineQueryResults::numResources invoked.");
#endif

    ReadLock lock(dataLock_);

    count = currResourceTotal_;

    return true;
}


bool
AlpineQueryResults::numPending(ulong & pending)
{
#ifdef _VERBOSE
    Log::Debug("AlpineQueryResults::numPending invoked.");
#endif

    ReadLock lock(dataLock_);

    pending = (expectedResourceTotal_ - currResourceTotal_);

    return true;
}


bool
AlpineQueryResults::numPeersResponding(ulong & numPeers)
{
#ifdef _VERBOSE
    Log::Debug("AlpineQueryResults::numPeersResponding invoked.");
#endif

    ReadLock lock(dataLock_);

    numPeers = numPeers_;

    return true;
}


bool
AlpineQueryResults::getPeerList(t_PeerIdList & peerList)
{
#ifdef _VERBOSE
    Log::Debug("AlpineQueryResults::getPeerList invoked.");
#endif

    ReadLock lock(dataLock_);

    peerList.clear();


    for (const auto & item : *replyIndex_) {
        peerList.push_back(item.first);
    }


    return true;
}


bool
AlpineQueryResults::getResourceList(ulong peerId, AlpineQueryPacket::t_ResourceDescList & resourceList)
{
#ifdef _VERBOSE
    Log::Debug("AlpineQueryResults::getResourceList invoked.  Peer ID: "s + std::to_string(peerId));
#endif

    ReadLock lock(dataLock_);


    // Locate list for this peer, if it exists.
    //
    auto iter = replyIndex_->find(peerId);

    if (iter == replyIndex_->end()) {
        Log::Error("Invalid peer ID passed in call to AlpineQueryResults::getResourceList!");
        return false;
    }
    // Get current list of returned resource descriptions
    //
    bool status;
    t_ReplyState * currState;
    currState = (*iter).second;

    status = currState->resourceDescSet->getResourceList(resourceList);

    if (!status) {
        Log::Error("Call to resourceDescSet->getResourceList failed in "
                   "AlpineQueryResults::getResourceList!");
        return false;
    }
    return true;
}


bool
AlpineQueryResults::processQueryOffer(ulong peerId, AlpineQueryPacket * offer, AlpineQueryPacket * request)
{
#ifdef _VERBOSE
    Log::Debug("AlpineQueryResults::processQueryOffer invoked.  Peer ID: "s + std::to_string(peerId));
#endif

    bool status;
    status = verifyQueryOffer(peerId, offer);

    if (!status) {
        // Verify will perform any clean up required.
        return false;
    }
    WriteLock lock(dataLock_);

    // Locate state for this request.
    //
    t_ReplyState * currState = nullptr;

    auto iter = replyIndex_->find(peerId);

    if (iter == replyIndex_->end()) {
        Log::Error("Cannot locate reply state in call to "
                   "AlpineQueryResults::processQueryOffer!");
        return false;
    }
    currState = (*iter).second;


    // Update our expected count according to reply size.  If the number of hits is more than
    // the configured max desc per peer, then use the configured max instead.
    //
    ulong replyHitCount;
    ulong maxDescPerPeer;

    offer->getNumHits(replyHitCount);
    queryOptions_.getMaxDescPerPeer(maxDescPerPeer);

    if (replyHitCount > maxDescPerPeer) {
#ifdef _VERBOSE
        Log::Debug("Reply hit count exceeds max configured per peer.  Setting to max."s + "\n - Reply Hit Count: "s +
                   std::to_string(replyHitCount) + "\n - Max per Peer: "s + std::to_string(maxDescPerPeer) + "\n");
#endif
        replyHitCount = maxDescPerPeer;
    }


    currState->expectedTotal = replyHitCount;
    expectedResourceTotal_ += replyHitCount;

    status = populateQueryRequest(currState, request);

    if (!status) {
        Log::Error("Populate query request failed in call to "
                   "AlpineQueryResults::processQueryOffer!");
        return false;
    }
    return true;
}


bool
AlpineQueryResults::processQueryReply(ulong peerId, AlpineQueryPacket * reply, AlpineQueryPacket * request)
{
#ifdef _VERBOSE
    Log::Debug("AlpineQueryResults::processQueryReply invoked.  Peer ID: "s + std::to_string(peerId));
#endif

    bool status;
    status = verifyQueryReply(peerId, reply);

    if (!status) {
        // Verify will perform any clean up required.
        return false;
    }
    WriteLock lock(dataLock_);

    // Locate state for this request.
    //
    t_ReplyState * currState = nullptr;

    auto iter = replyIndex_->find(peerId);

    if (iter == replyIndex_->end()) {
        Log::Error("Cannot locate reply state in call to "
                   "AlpineQueryResults::processQueryReply!");
        return false;
    }
    currState = (*iter).second;


    // Add reply resources to resource set and update state for this query.
    //
    status = currState->resourceDescSet->addResourceDataPacket(reply);

    if (!status) {
        Log::Error("Unable to add resource set from reply packet in call to "
                   "AlpineQueryResults::processQueryReply!");
        return false;
    }
    // Cross-peer dedup: count only resources not seen from other peers.
    // Hash by (primary locator + size).  Resources are still stored per-peer
    // for completeness, but currResourceTotal_ reflects unique count.
    ushort replySetSize;
    reply->getReplySetSize(replySetSize);

    AlpineQueryPacket::t_ResourceDescList replyResources;
    reply->getResourceDescList(replyResources);

    ushort uniqueCount = 0;
    for (auto & desc : replyResources) {
        AlpineResourceDesc::t_LocatorList locators;
        desc.getLocatorList(locators);

        string primaryLocator = locators.empty() ? ""s : locators[0];
        ulong resourceSize = desc.getSize();

        size_t h = std::hash<string>{}(primaryLocator) ^ (std::hash<ulong>{}(resourceSize) << 1);

        if (seenResources_.insert(h).second)
            ++uniqueCount;
    }

    currResourceTotal_ += uniqueCount;


    // Prepare reply packet
    //
    status = populateQueryRequest(currState, request);

    if (!status) {
        Log::Error("Populate query request failed in call to "
                   "AlpineQueryResults::processQueryReply!");
        return false;
    }
    // MRP_TEMP handle last reply packet???


    return true;
}


bool
AlpineQueryResults::verifyQueryOffer(ulong peerId, AlpineQueryPacket * offer)
{
#ifdef _VERBOSE
    Log::Debug("AlpineQueryResults::verifyQueryOffer invoked.  Peer ID: "s + std::to_string(peerId));
#endif

    // Initial verification of query offers consists of the following steps:
    //   1 - Verify packet type, must be query offer.
    //   2 - Verify packet query ID.  Must match our query ID.
    //   3 - Check duplicate packets.
    //
    if (offer->getPacketType() != AlpinePacket::t_PacketType::queryOffer) {
        Log::Error("Invalid packet type passed in call to AlpineQueryResults::verifyQueryOffer!");
        AlpinePeerMgr::badPacketReceived(peerId);

        return false;
    }
    bool status;
    ulong queryId;

    status = offer->getQueryId(queryId);

    if (!status) {
        Log::Error("Unable to get query ID in AlpineQueryResults::verifyQueryOffer!");
        AlpinePeerMgr::badPacketReceived(peerId);

        return false;
    }
    if (queryId != queryId_) {
#ifdef _VERBOSE
        Log::Debug("Received packet contains invalid query ID in "
                   "AlpineQueryResults::verifyQueryOffer!");
#endif
        AlpinePeerMgr::badPacketReceived(peerId);

        return false;
    }
    // Locate state for this request.  If it exists, verify sequence number.
    // If it does not, create reply state for this peer and update index.
    //
    t_ReplyState * currState = nullptr;

    WriteLock lock(dataLock_);  // this makes things easier

    auto iter = replyIndex_->find(peerId);

    if (iter != replyIndex_->end()) {
        currState = (*iter).second;
    }

    if (!currState) {
        // New reply, create state and initialize
        //
        currState = new t_ReplyState;
        currState->peerId = peerId;
        currState->expectedTotal = 0;
        currState->duplicateCount = 0;
        currState->resourceDescSet = new AlpineResourceDescSet;

        replyIndex_->emplace(peerId, currState);
        numPeers_++;

        // At this point, we consider validation successful
    } else {
        // This a reply to an existing request.  This should only occur if we get a
        // duplicate due to packet loss...
        //
        currState->duplicateCount++;

        if (currState->duplicateCount >= AlpineProtocol::maxDuplicateRecv_s) {
            // Too many duplicate packets!  Treat as maliscious attack.  If this is not,
            // the peer's client is severly fucked up, so no loss either way.
            //
#ifdef _VERBOSE
            Log::Debug("Too many duplicate packets from peer in call to "
                       "AlpineQueryResults::verifyQueryOffer.  Banning.");
#endif
            expectedResourceTotal_ -= currState->expectedTotal;

            delete currState->resourceDescSet;
            delete currState;

            replyIndex_->erase(peerId);

            status = AlpinePeerMgr::banPeer(peerId);

            if (!status) {
                Log::Error("Ban peer failed in call to AlpineQueryResults::verifyQueryOffer!");
                return false;
            }
            return false;
        }

        // In either case, this is not a valid packet.
        return false;
    }
    return true;
}


bool
AlpineQueryResults::verifyQueryReply(ulong peerId, AlpineQueryPacket * reply)
{
#ifdef _VERBOSE
    Log::Debug("AlpineQueryResults::verifyQueryReply invoked.  Peer ID: "s + std::to_string(peerId));
#endif

    // Initial verification of query replies consists of the following steps:
    //   1 - Verify packet type, must be query reply.
    //   2 - Verify packet query ID.  Must match our query ID.
    //   3 - Check sequence number and duplicate packets.
    //
    if (reply->getPacketType() != AlpinePacket::t_PacketType::queryReply) {
        Log::Error("Invalid packet type passed in call to AlpineQueryResults::verifyQueryReply!");
        AlpinePeerMgr::badPacketReceived(peerId);

        return false;
    }
    bool status;
    ulong queryId;

    status = reply->getQueryId(queryId);

    if (!status) {
        Log::Error("Unable to get query ID in AlpineQueryResults::verifyQueryReply!");
        AlpinePeerMgr::badPacketReceived(peerId);

        return false;
    }
    if (queryId != queryId_) {
#ifdef _VERBOSE
        Log::Debug("Received packet contains invalid query ID in "
                   "AlpineQueryResults::verifyQueryReply!");
#endif
        AlpinePeerMgr::badPacketReceived(peerId);

        return false;
    }
    // Locate state for this request.
    // If it does not, some kind of error occured.  Fubar'd request??
    //
    t_ReplyState * currState = nullptr;

    WriteLock lock(dataLock_);  // use write lock, we will probably modify something

    auto iter = replyIndex_->find(peerId);

    if (iter == replyIndex_->end()) {
#ifdef _VERBOSE
        Log::Debug("Invalid peer ID; cannot locate reply state in call to "
                   "AlpineQueryResults::verifyQueryReply!");
#endif
        AlpinePeerMgr::badPacketReceived(peerId);

        return false;
    }
    currState = (*iter).second;


    // Verify offset for this repsonse.
    //
    ulong currOffset;
    status = reply->getOffset(currOffset);

    if (!status) {
        Log::Error("Get offset from reply failed in call to "
                   "AlpineQueryResults::verifyQueryReply!");
        return false;
    }
    ulong expectedOffset;
    currState->resourceDescSet->getCurrOffset(expectedOffset);

    // If offset is more than expected, this is an invalid packet.
    // Otherwise, if the offset is less than expected, increment duplicate count.
    //
    if (currOffset > expectedOffset) {
#ifdef _VERBOSE
        Log::Debug("Invalid offset in reply packet in call to "
                   "AlpineQueryResults::verifyQueryReply!");
#endif
        AlpinePeerMgr::badPacketReceived(peerId);

        return false;
    }
    if (currOffset != expectedOffset) {

        // Consider this a duplicate packet, probably from a previous reply that is just now
        // reaching us.  Long route?
        //
        currState->duplicateCount++;

        if (currState->duplicateCount >= AlpineProtocol::maxDuplicateRecv_s) {
            // Too many duplicate packets!  Treat as maliscious attack.  If this is not,
            // the peer's client is severly fucked up, so no loss either way.
            //
#ifdef _VERBOSE
            Log::Debug("Too many duplicate packets from peer in call to "
                       "AlpineQueryResults::verifyQueryReply.  Banning.");
#endif
            expectedResourceTotal_ -= currState->expectedTotal;

            delete currState->resourceDescSet;
            delete currState;

            replyIndex_->erase(peerId);

            status = AlpinePeerMgr::banPeer(peerId);

            if (!status) {
                Log::Error("Ban peer failed in call to AlpineQueryResults::verifyQueryReply!");
                return false;
            }
            return false;
        }

        // In either case, this is not a valid packet.
        return false;
    }
    return true;
}


bool
AlpineQueryResults::populateQueryRequest(t_ReplyState * state, AlpineQueryPacket * request)
{
#ifdef _VERBOSE
    Log::Debug("AlpineQueryResults::populateQueryRequest invoked.");
#endif

    request->setPacketType(AlpinePacket::t_PacketType::queryRequest);
    request->setQueryId(queryId_);

    string queryString;
    queryOptions_.getQuery(queryString);
    request->setQueryString(queryString);

    bool status;
    ulong currOffset;

    status = state->resourceDescSet->getCurrOffset(currOffset);

    if (!status) {
        Log::Error("Unable to get current resource offset in call to "
                   "AlpineQueryResults::populateQueryRequest!");
        return false;
    }
    // Set the offset and desired reply set size according to the replies received so far.
    //
    request->setOffset(currOffset);

    ulong desiredSetSize;
    desiredSetSize = state->expectedTotal - currOffset;

    if (desiredSetSize > USHRT_MAX) {
        desiredSetSize = USHRT_MAX;
    }

    request->setReplySetSize((ushort)desiredSetSize);

    // MRP_TEMP for now query options are unused.
    //
    request->setOptionId(0);


    return true;
}
