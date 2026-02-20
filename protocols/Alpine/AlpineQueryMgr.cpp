///////
///
///  Copyright (C) 2026  sonoransun
///
///  Permission is hereby granted, free of charge, to any person obtaining a copy
///  of this software and associated documentation files (the "Software"), to deal
///  in the Software without restriction, including without limitation the rights
///  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
///  copies of the Software, and to permit persons to whom the Software is
///  furnished to do so, subject to the following conditions:
///
///  The above copyright notice and this permission notice shall be included in all
///  copies or substantial portions of the Software.
///
///  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
///  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
///  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
///  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
///  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
///  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
///  SOFTWARE.
///
///////


#include <AlpineQueryMgr.h>
#include <AlpineQuery.h>
#include <AlpineQueryResults.h>
#include <AlpinePeerMgr.h>
#include <AlpineStack.h>
#include <WriteLock.h>
#include <ReadLock.h>
#include <Log.h>
#include <StringUtils.h>



bool                                       AlpineQueryMgr::initialized_s = false;
ulong                                      AlpineQueryMgr::maxConncurent_s = 0;
ulong                                      AlpineQueryMgr::currQueryId_s = 0;
AlpineQueryMgr::t_QueryStateIndex *        AlpineQueryMgr::activeQueryIndex_s = nullptr;
AlpineQueryMgr::t_QueryStateIndex *        AlpineQueryMgr::pastQueryIndex_s = nullptr;
AlpineQueryMgr::t_PeerQueryIndex *         AlpineQueryMgr::activePeerQueryIndex_s = nullptr;
AlpineQueryMgr::t_PeerQueryIndex *         AlpineQueryMgr::pastPeerQueryIndex_s = nullptr;
ReadWriteSem                               AlpineQueryMgr::dataLock_s;



bool  
AlpineQueryMgr::initialize (ulong maxConcurrent)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryMgr::initialize invoked.  MaxConcurrentQueries: "s +
                std::to_string (maxConcurrent));
#endif

    WriteLock  lock(dataLock_s);

    if (initialized_s) {
        Log::Error ("Attempt to reinitialize AlpineQueryMgr!");
        return false;
    }
    maxConncurent_s = maxConcurrent;

    activeQueryIndex_s     = new t_QueryStateIndex;
    pastQueryIndex_s       = new t_QueryStateIndex;
    activePeerQueryIndex_s = new t_PeerQueryIndex;
    pastPeerQueryIndex_s   = new t_PeerQueryIndex;

    initialized_s = true;


    return true;
}



bool  
AlpineQueryMgr::createQuery (AlpineQueryOptions &  options,
                             ulong &               queryId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryMgr::createQuery invoked.");
#endif

    WriteLock  lock(dataLock_s);

    if (!initialized_s) {
        Log::Error ("Call to AlpineQueryMgr::createQuery before initialization!");
        return false;
    }
    /////
    // The steps required to create query are:
    // 
    // - Get a query ID to identify this new query from here on.
    // - Create the AlpineQuery and AlpineQueryResults objects for this query.
    // - Kick off the query broadcast...
    /////

    queryId = currQueryId_s++;

    t_QueryState *  newState;
    newState = new t_QueryState;

    newState->queryId = queryId;
    newState->query   = new AlpineQuery (options);
    newState->results = new AlpineQueryResults (queryId, options);
    newState->pending = nullptr; // only allocate this if needed.


    // Get list of peers involved in this query for indexing purposes.
    //
    bool  status;
    AlpineQuery::t_PeerIdList   peerIdList;
    status = newState->query->getPeerIdList (peerIdList);

    if (!status) {
        Log::Error ("getPeerIdList for query failed in call to AlpineQueryMgr::createQuery!");

        delete newState->query;
        delete newState->results;

        return false;
    }
    // Activate query
    status = newState->query->startQuery ();

    if (!status) {
        // This is likely due to invalid options.
        //
        Log::Error ("startQuery failed in call to AlpineQueryMgr::createQuery!");
        
        delete newState->query;
        delete newState->results; 

        return false;
    }
    // Index state by query ID and member peer ID's
    //
    activeQueryIndex_s->emplace (queryId, newState);

    ulong  currPeerId;
    t_QueryIdList *  currIdList;

    for (auto& item : peerIdList) {

        currPeerId = item;

        // If there is an existing queryIdList, we simply add this query ID to the list.
        // Otherwise allocate a new list with this query ID as the sole member.
        //
        auto peerIter = activePeerQueryIndex_s->find (currPeerId);

        if (peerIter == activePeerQueryIndex_s->end ()) {
            currIdList = new t_QueryIdList;
            activePeerQueryIndex_s->emplace (currPeerId, currIdList);
        }
        else {
            currIdList = (*peerIter).second;
        }

        currIdList->push_back (queryId);
    }
        

    return true;
}



bool  
AlpineQueryMgr::getQueryStatus (ulong                queryId,
                                AlpineQueryStatus &  queryStatus)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryMgr::getQueryStatus invoked. Query ID: "s +
                std::to_string (queryId));
#endif

    ReadLock  lock(dataLock_s);

    if (!initialized_s) {
        Log::Error ("Call to AlpineQueryMgr::getQueryStatus before initialization!");
        return false;
    }
    // Attempt to locate query state for this ID
    //
    t_QueryState *  currState;

    auto iter = activeQueryIndex_s->find (queryId);

    if (iter == activeQueryIndex_s->end ()) {
        // This may be an inactive query, so check the inactive index before failing.
        //
        iter = pastQueryIndex_s->find (queryId);

        if (iter == pastQueryIndex_s->end ()) {
            // Invalid query ID
            Log::Error ("Invalid query ID passed in call to AlpineQueryMgr::getQueryStatus!");
            return false;
        }
        currState = (*iter).second;
    }
    else {
        currState = (*iter).second;
    }
   
    if (!currState->query) {
        // If the query value is null, this is a completed old query.
        // We can simply set the completed status to 100% and inactive flag.  
        //
        double percentage;
        percentage = 100.00;
        queryStatus.setPercentComplete (percentage);
        queryStatus.setIsActive (false);

        return true;
    }
    bool status;
    status = currState->query->getStatus (queryStatus);

    if (!status) {
        Log::Error ("AlpineQuery getStatus call failed in call to AlpineQueryMgr::getQueryStatus!");
        return false;
    }
    return true;
}



bool  
AlpineQueryMgr::exists (ulong  queryId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryMgr::exists invoked. Query ID: "s +
                std::to_string (queryId));
#endif

    ReadLock  lock(dataLock_s);

    if (!initialized_s) {
        Log::Error ("Call to AlpineQueryMgr::exists before initialization!");
        return false;
    }
    return (activeQueryIndex_s->find (queryId) != activeQueryIndex_s->end ()) ||
           (pastQueryIndex_s->find (queryId) != pastQueryIndex_s->end ());
}



bool  
AlpineQueryMgr::isActive (ulong  queryId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryMgr::isActive invoked. Query ID: "s +
                std::to_string (queryId));
#endif

    ReadLock  lock(dataLock_s);

    if (!initialized_s) {
        Log::Error ("Call to AlpineQueryMgr::isActive before initialization!");
        return false;
    }
    return activeQueryIndex_s->find (queryId) != activeQueryIndex_s->end ();
}



bool  
AlpineQueryMgr::inProgress (ulong  queryId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryMgr::inProgress invoked. Query ID: "s +
                std::to_string (queryId));
#endif

    ReadLock  lock(dataLock_s);

    if (!initialized_s) {
        Log::Error ("Call to AlpineQueryMgr::inProgress before initialization!");
        return false;
    }
    t_QueryState *  currState = nullptr;

    auto iter = activeQueryIndex_s->find (queryId);

    if (iter == activeQueryIndex_s->end ()) {
        Log::Error ("Invalid query ID passed in call to AlpineQueryMgr::inProgress!");
        return false;
    }
    currState = (*iter).second;

    return currState->query->inProgress ();
}



bool
AlpineQueryMgr::pauseQuery (ulong  queryId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryMgr::pauseQuery invoked. Query ID: "s +
                std::to_string (queryId));
#endif

    ReadLock  lock(dataLock_s);

    if (!initialized_s) {
        Log::Error ("Call to AlpineQueryMgr::pauseQuery before initialization!");
        return false;
    }
    bool status;
    t_QueryState *  currState = nullptr;

    auto iter = activeQueryIndex_s->find (queryId);

    if (iter == activeQueryIndex_s->end ()) {
        Log::Error ("Invalid query ID passed in call to AlpineQueryMgr::pauseQuery!");
        return false;
    }
    currState = (*iter).second;

    if (!currState->query->inProgress ()) {
        // Query already halted, consider this a success
        return true;
    }
    status = currState->query->halt ();
    if (!status) {
        Log::Error ("Attempted query->halt failed in call to AlpineQueryMgr::pauseQuery!");
        return false;
    }
    return true;
}



bool  
AlpineQueryMgr::resumeQuery (ulong  queryId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryMgr::resumeQuery invoked. Query ID: "s +
                std::to_string (queryId));
#endif

    ReadLock  lock(dataLock_s);

    if (!initialized_s) {
        Log::Error ("Call to AlpineQueryMgr::resumeQuery before initialization!");
        return false;
    }
    bool status;
    t_QueryState *  currState = nullptr;

    auto iter = activeQueryIndex_s->find (queryId);

    if (iter == activeQueryIndex_s->end ()) {
        Log::Error ("Invalid query ID passed in call to AlpineQueryMgr::resumeQuery!");
        return false;
    }
    currState = (*iter).second;

    if (currState->query->inProgress ()) {
        // Query already in progress, consider this a success 
        return true;
    }
    status = currState->query->resume ();
    if (!status) {
        Log::Error ("Attempted query->resume failed in call to AlpineQueryMgr::resumeQuery!");
        return false;
    }
    return true;
}



bool  
AlpineQueryMgr::cancelQuery (ulong  queryId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryMgr::cancelQuery invoked. Query ID: "s +
                std::to_string (queryId));
#endif

    WriteLock  lock(dataLock_s);

    if (!initialized_s) {
        Log::Error ("Call to AlpineQueryMgr::cancelQuery before initialization!");
        return false;
    }
    bool status;
    t_QueryState *  currState = nullptr;

    auto queryIter = activeQueryIndex_s->find (queryId);

    if (queryIter == activeQueryIndex_s->end ()) {
        Log::Error ("Invalid query ID passed in call to AlpineQueryMgr::cancelQuery!");
        return false;
    }
    currState = (*queryIter).second;

    status = currState->query->cancel ();

    if (!status) {
        Log::Error ("Attempt to cancel query failed in AlpineQueryMgr::cancelQuery.  Ending query.");
        return false;
    }

    delete currState->query;
    currState->query = nullptr;

    // remove from the active index, and place state in the past index
    //
    activeQueryIndex_s->erase (queryId);
    pastQueryIndex_s->emplace (queryId, currState);


    // Remove peer ID indexes for this query from the active index, and
    // move them to the past index.
    //
    AlpineQuery::t_PeerIdList  peerIdList;
    status = currState->query->getPeerIdList (peerIdList);

    if (!status) {
        Log::Error ("Call to query getPeerIdList failed in AlpineQueryMgr::cancelQuery!");
        return false;
    }
    ulong currPeerId;
    t_QueryIdList *  currIdList;

    for (auto& item : peerIdList) {

        currPeerId = item;

        // We must remove the query ID from the peers active queryIdList and place it in the
        // past queryIdList.
        //
        auto peerQueryIter = activePeerQueryIndex_s->find (currPeerId);

        if (peerQueryIter == activePeerQueryIndex_s->end ()) {
            Log::Error ("Could not find queryIdList for peer in call to "
                                 "AlpineQueryMgr::cancelQuery!");
            continue;
        }

        currIdList = (*peerQueryIter).second;

        std::erase(*currIdList, queryId);

        peerQueryIter = pastPeerQueryIndex_s->find (currPeerId);

        if (peerQueryIter == activePeerQueryIndex_s->end ()) {
            currIdList =  new t_QueryIdList;
            pastPeerQueryIndex_s->emplace (currPeerId, currIdList);
        }
        else {
            currIdList = (*peerQueryIter).second;
        }

        currIdList->push_back (queryId);
    }


    return true;
}



bool  
AlpineQueryMgr::getAllActiveQueryIds (t_QueryIdList  queryIdList)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryMgr::getAllActiveQueryIds invoked.");
#endif 

    ReadLock  lock(dataLock_s);
    
    if (!initialized_s) {
        Log::Error ("Call to AlpineQueryMgr::getAllActiveQueryIds before initialization!");
        return false;
    }
    queryIdList.clear ();


    for (const auto& item : *activeQueryIndex_s) {
        queryIdList.push_back (item.first);
    }


    return true;
}



bool  
AlpineQueryMgr::getAllPastQueryIds (t_QueryIdList  queryIdList)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryMgr::getAllPastQueryIds invoked.");
#endif

    ReadLock  lock(dataLock_s);

    if (!initialized_s) {
        Log::Error ("Call to AlpineQueryMgr::getQueryResults before initialization!");
        return false;
    }
    queryIdList.clear ();


    for (const auto& item : *pastQueryIndex_s) {
        queryIdList.push_back (item.first);
    }


    return true;
}



bool  
AlpineQueryMgr::getQueryResults (ulong                  queryId,
                                 AlpineQueryResults *&  results)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryMgr::getQueryResults invoked. Query ID: "s +
                std::to_string (queryId));
#endif

    ReadLock  lock(dataLock_s);

    if (!initialized_s) {
        Log::Error ("Call to AlpineQueryMgr::getQueryResults before initialization!");
        return false;
    }
    t_QueryState *  currState = nullptr;

    auto iter = activeQueryIndex_s->find (queryId);

    if (iter == activeQueryIndex_s->end ()) {
        Log::Error ("Invalid query ID passed in call to AlpineQueryMgr::getQueryResults!");
        return false;
    }
    currState = (*iter).second;

    results = currState->results;


    return true;
}



bool  
AlpineQueryMgr::getPeerActiveQueryList (ulong            peerId,
                                        t_QueryIdList &  queryIdList)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryMgr::getPeerActiveQueryList invoked. Peer ID: "s +
                std::to_string (peerId));
#endif

    ReadLock  lock(dataLock_s);

    if (!initialized_s) {
        Log::Error ("Call to AlpineQueryMgr::getPeerActiveQueryList before initialization!");
        return false;
    }
    queryIdList.clear ();

    auto iter = activePeerQueryIndex_s->find (peerId);

    if (iter == activePeerQueryIndex_s->end ()) {
        Log::Debug ("No queries active for peer ID passed in call to "
                             "AlpineQueryMgr::getPeerActiveQueryList.");
        return true;
    }
    t_QueryIdList *  peerQueryIdList;
    peerQueryIdList = (*iter).second;

    if (!peerQueryIdList) {
        // No active queries, return with empty list.
        return true;
    }
    queryIdList = *peerQueryIdList;


    return true;
}



bool  
AlpineQueryMgr::getPeerPastQueryList (ulong            peerId,
                                      t_QueryIdList &  queryIdList)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryMgr::getPeerPastQueryList invoked. Peer ID: "s +
                std::to_string (peerId));
#endif

    ReadLock  lock(dataLock_s);

    if (!initialized_s) {
        Log::Error ("Call to AlpineQueryMgr::getPeerPastQueryList before initialization!");
        return false;
    }
    queryIdList.clear ();

    auto iter = pastPeerQueryIndex_s->find (peerId);

    if (iter == pastPeerQueryIndex_s->end ()) {
        Log::Debug ("No queries active for peer ID passed in call to "
                             "AlpineQueryMgr::getPeerPastQueryList.");
        return true;
    }
    t_QueryIdList *  peerQueryIdList;
    peerQueryIdList = (*iter).second;

    if (!peerQueryIdList) {
        // No active queries, return with empty list.
        return true;
    }
    queryIdList = *peerQueryIdList;


    return true;
}



bool  
AlpineQueryMgr::handleQueryDiscover (ulong                peerId,
                                     AlpineQueryPacket *  discoverPacket)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryMgr::handleQueryDiscover invoked.  Peer ID: "s +
                std::to_string (peerId));
#endif

    WriteLock  lock(dataLock_s);

    if (!initialized_s) {
        Log::Error ("Call to AlpineQueryMgr::handleQueryDiscover before initialization!");
        return false;
    }
    return true;
}



bool  
AlpineQueryMgr::handleQueryOffer (ulong                peerId,
                                  AlpineQueryPacket *  offerPacket)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryMgr::handleQueryOffer invoked.  Peer ID: "s +
                std::to_string (peerId));
#endif

    // A query offer is considered a 'response' to a query (as far as quality metrics are concerned)
    // When an offer is received, we update the Profiles for the peer involved.  This is currently
    // the base profile administered by the AlpinePeerMgr, and the group profile, which is administered
    // by the AlpineGroup for that group.
    //

    WriteLock  lock(dataLock_s);

    if (!initialized_s) {
        Log::Error ("Call to AlpineQueryMgr::handleQueryOffer before initialization!");
        return false;
    }
    // Verify query ID, and locate state for this query.
    //
    ulong  queryId;
    offerPacket->getQueryId (queryId);

    t_QueryState *  currState = nullptr;

    auto iter = activeQueryIndex_s->find (queryId);

    if (iter == activeQueryIndex_s->end ()) {
#ifdef _VERBOSE
        Log::Error ("Invalid query ID passed in call to AlpineQueryMgr::handleQueryOffer!");
#endif
        AlpinePeerMgr::badPacketReceived (peerId);

        return false;
    }
    currState = (*iter).second;


    // Process this offer packet.  If correct, a response packet will be populated for transfer
    // back to peer.  This starts the actual transfer of resource locators.
    //   
    bool status;
    AlpinePacket  packet;
    AlpineQueryPacket  response;
    packet.setParent (&response);

    status = currState->results->processQueryOffer (peerId,
                                                    offerPacket,
                                                    &response);

    if (!status) {
#ifdef _VERBOSE
        Log::Error ("Process query offer packet failed in call to AlpineQueryMgr::handleQueryOffer!");
#endif
        AlpinePeerMgr::badPacketReceived (peerId);

        return false;
    }
    // Send response to peer; start locator transfer
    //
    ulong  requestId;
    status = AlpineStack::sendReliablePacket (peerId,
                                              &packet,
                                              requestId);

    if (!status) {
        Log::Error ("sendReliableData for response packet data failed in call to "
                             "AlpineQueryMgr::handleQueryOffer!");
        return false;
    }
    // Index this reliable request ID in case we need to acknowlege it manually
    // when a future packet is received.
    //
    if (!currState->pending) {
        currState->pending = new t_PendingReplyIndex;
    }

    currState->pending->emplace (peerId, requestId);


    return true;
}



bool  
AlpineQueryMgr::handleQueryRequest (ulong                peerId,
                                    AlpineQueryPacket *  requestPacket)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryMgr::handleQueryRequest invoked.  Peer ID: "s +
                std::to_string (peerId));
#endif

    WriteLock  lock(dataLock_s);

    if (!initialized_s) {
        Log::Error ("Call to AlpineQueryMgr::handleQueryRequest before initialization!");
        return false;
    }
    return true;
}



bool  
AlpineQueryMgr::handleQueryReply (ulong                peerId,
                                  AlpineQueryPacket *  replyPacket)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryMgr::handleQueryReply invoked.  Peer ID: "s +
                std::to_string (peerId));
#endif

    ReadLock  lock(dataLock_s);

    if (!initialized_s) {
        Log::Error ("Call to AlpineQueryMgr::handleQueryReply before initialization!");
        return false;
    }
    // Verify query ID, and locate state for this query.
    //
    ulong  queryId;
    replyPacket->getQueryId (queryId);

    t_QueryState *  currState = nullptr;

    auto iter = activeQueryIndex_s->find (queryId);

    if (iter == activeQueryIndex_s->end ()) {
#ifdef _VERBOSE
        Log::Error ("Invalid query ID passed in call to AlpineQueryMgr::handleQueryReply!");
#endif
        AlpinePeerMgr::badPacketReceived (peerId);

        return false;
    }
    currState = (*iter).second;


    // Process this reply packet.  If correct, a response packet will be populated for transfer
    // back to peer.  The locators contained in the response are then added to the current
    // response information for this query.
    //   
    bool status;
    AlpinePacket  packet;
    AlpineQueryPacket  response;
    packet.setParent (&response);

    status = currState->results->processQueryReply (peerId,
                                                    replyPacket,
                                                    &response);

    if (!status) {
#ifdef _VERBOSE
        Log::Error ("Process query reply packet failed in call to AlpineQueryMgr::handleQueryReply!");
#endif
        AlpinePeerMgr::badPacketReceived (peerId);

        return false;
    }
    // The packet sent prior to this was a reliable transfer.  Make sure that the reliable transfer state
    // is cleaned up before sending anything else to this peer.
    //
    ulong  requestId;
    auto pendingIter = currState->pending->find (peerId);

    if (pendingIter == currState->pending->end ()) {
        // This should be normal case, as the ack packet would have been received.
    }
    else {
        requestId = (*pendingIter).second;
        currState->pending->erase (peerId);

        AlpineStack::acknowledgeTransfer (peerId, requestId);
    }


    // Send response to peer
    //
    status = AlpineStack::sendReliablePacket (peerId,
                                              &packet,
                                              requestId);

    if (!status) {
        Log::Error ("sendReliableData for response packet data failed in call to "
                             "AlpineQueryMgr::handleQueryOffer!");
        return false;
    }
    // Index this reliable request ID in case we need to acknowlege it manually
    // when/if another reply packet is received.
    //
    currState->pending->emplace (peerId, requestId);


    return true;
}



bool
AlpineQueryMgr::handleSendReceived (ulong  peerId,
                                    ulong  requestId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryMgr::handleSendReceived invoked.  Parameters:"s +
                "\n Peer ID: "s + std::to_string (peerId) +
                "\n Request ID: "s + std::to_string (requestId) +
                "\n");
#endif

    ReadLock  lock(dataLock_s);

    if (!initialized_s) {
        Log::Error ("Call to AlpineQueryMgr::handleSendReceived before initialization!");
        return false;
    }
    bool status;
    status = removePendingRequest (peerId, requestId);


    return status;
}   
    


bool  
AlpineQueryMgr::handleSendFailure (ulong  peerId,
                                   ulong  requestId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryMgr::handleSendFailure invoked.  Parameters: "s +
                "\n Peer ID: "s + std::to_string (peerId) +
                "\n Request ID: "s + std::to_string (requestId) +
                "\n");
#endif

    ReadLock  lock(dataLock_s);

    if (!initialized_s) {
        Log::Error ("Call to AlpineQueryMgr::handleSendFailure before initialization!");
        return false;
    }
    // Modify status value due to failed transfer of reliable data.
    // MRP_TEMP - remove from queries & deactivate transport?
    //
    AlpinePeerMgr::reliableTransferFailed (peerId);

    bool status; 
    status = removePendingRequest (peerId, requestId);


    return status;
}



bool  
AlpineQueryMgr::cancelAll (ulong  peerId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryMgr::cancelAll invoked.  Peer ID: "s +
                std::to_string (peerId));
#endif

    // scope lock
    {
        ReadLock  lock(dataLock_s);

        if (!initialized_s) {
            Log::Error ("Call to AlpineQueryMgr::cancelAll before initialization!");
            return false;
        }
    }

    bool  status;
    t_QueryIdList  queryIdList;

    status = getAllActiveQueryIds (queryIdList);

    if (!status) {
        Log::Error ("getAllActiveQueryIds failed in call to AlpineQueryMgr::cancelAll!");
        return false;
    }
    for (auto& item : queryIdList) {
        cancelQuery (item);
    }


    return true;
}



void  
AlpineQueryMgr::processTimedEvents ()
{   
#ifdef _VERY_VERBOSE  
    Log::Debug ("AlpineQueryMgr::processTimedEvents invoked.");
#endif

    ReadLock  lock(dataLock_s);

    if (!initialized_s) {
        Log::Error ("Call to AlpineQueryMgr::processTimedEvents before initialization!");
    }
}



void  
AlpineQueryMgr::cleanUp ()
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryMgr::cleanUp invoked.");
#endif

    WriteLock  lock(dataLock_s);

    if (!initialized_s) {
        Log::Error ("Call to AlpineQueryMgr::cleanUp before initialization!");
        return;
    }

    if (activeQueryIndex_s) {

        for (const auto& item : *activeQueryIndex_s) {
            delete item.second;
        }

        delete activeQueryIndex_s;
        activeQueryIndex_s = nullptr;
        return;
    }

    if (pastQueryIndex_s) {

        for (const auto& item : *pastQueryIndex_s) {
            delete item.second;
        }

        delete pastQueryIndex_s;
        pastQueryIndex_s = nullptr;
        return;
    }

    if (activePeerQueryIndex_s) {

        for (const auto& item : *activePeerQueryIndex_s) {
            delete item.second;
        }

        delete activePeerQueryIndex_s;
        activePeerQueryIndex_s = nullptr;
        return;
    }

    if (pastPeerQueryIndex_s) {

        for (const auto& item : *pastPeerQueryIndex_s) {
            delete item.second;
        }

        delete pastPeerQueryIndex_s;
        pastPeerQueryIndex_s = nullptr;
        return;
    }

    initialized_s = false;
}



bool  
AlpineQueryMgr::removePendingRequest (ulong  peerId,
                                      ulong  requestId)
{
    // NOTE: Caller must synchronize!

    // Locate all queries this peer is a member of and see if this request applies.
    //
    auto peerIter = activePeerQueryIndex_s->find (peerId);

    if (peerIter == activePeerQueryIndex_s->end ()) {
        // query canceled?
#ifdef _VERBOSE
        Log::Debug ("No active queries for peer ID passed in call to "
                             "AlpineQueryMgr::removePendingRequest.  Ignoring.");
#endif

        return true;
    }
    t_QueryIdList *  queryList;
    queryList = (*peerIter).second;

    t_QueryState *  currState;

    for (auto& queryItem : *queryList) {
        // Locate query state for this query ID
        auto stateIter = activeQueryIndex_s->find (queryItem);

        if (stateIter == activeQueryIndex_s->end ()) {
            Log::Error ("Unable to locate query state returned in ID list in call to "
                                 "AlpineQueryMgr::removePendingRequest!");
            return false;
        }
        currState = (*stateIter).second;

        // If the request belongs to this query, remove it from the pending index.
        if ( (currState->pending) &&
             (currState->pending->find (peerId) != currState->pending->end ()) ) {

            // check that both request ID's match?
            currState->pending->erase (peerId);
            break;
        }
    }


    return true;
}



