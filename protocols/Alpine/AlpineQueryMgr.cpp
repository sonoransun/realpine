/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AlpineQueryMgr.h>
#include <AlpineQuery.h>
#include <AlpineQueryResults.h>
#include <AlpinePeerMgr.h>
#include <AlpineStack.h>
#include <WriteLock.h>
#include <ReadLock.h>
#include <Log.h>
#include <StringUtils.h>

#include <algorithm>
#include <chrono>

#ifdef ALPINE_ENABLE_PERSISTENCE
#include <PersistenceStore.h>
#endif

// Helper: wake the event loop after state changes that need prompt processing.
static inline void  wakeEventLoop () { AlpineStack::notifyEvent(); }



bool                                                      AlpineQueryMgr::initialized_s = false;
ulong                                                     AlpineQueryMgr::maxConncurent_s = 0;
std::atomic<ulong>                                        AlpineQueryMgr::currQueryId_s = 0;
std::unique_ptr<AlpineQueryMgr::t_QueryStateIndex>        AlpineQueryMgr::activeQueryIndex_s;
std::unique_ptr<AlpineQueryMgr::t_QueryStateIndex>        AlpineQueryMgr::pastQueryIndex_s;
std::unique_ptr<AlpineQueryMgr::t_PeerQueryIndex>         AlpineQueryMgr::activePeerQueryIndex_s;
std::unique_ptr<AlpineQueryMgr::t_PeerQueryIndex>         AlpineQueryMgr::pastPeerQueryIndex_s;
ReadWriteSem                                              AlpineQueryMgr::activeQueryLock_s;
ReadWriteSem                                              AlpineQueryMgr::pastQueryLock_s;



bool
AlpineQueryMgr::initialize (ulong maxConcurrent)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryMgr::initialize invoked.  MaxConcurrentQueries: "s +
                std::to_string (maxConcurrent));
#endif

    WriteLock  activeLock(activeQueryLock_s);
    WriteLock  pastLock(pastQueryLock_s);

    if (initialized_s) {
        Log::Error ("Attempt to reinitialize AlpineQueryMgr!");
        return false;
    }
    maxConncurent_s = maxConcurrent;

    activeQueryIndex_s     = std::make_unique<t_QueryStateIndex>();
    pastQueryIndex_s       = std::make_unique<t_QueryStateIndex>();
    activePeerQueryIndex_s = std::make_unique<t_PeerQueryIndex>();
    pastPeerQueryIndex_s   = std::make_unique<t_PeerQueryIndex>();

    initialized_s = true;

#ifdef ALPINE_ENABLE_PERSISTENCE
    // WAL recovery: scan for stale query entries from a prior crash.
    // Any wal: prefixed entries indicate queries that were active when
    // the process terminated abnormally.
    if (PersistenceStore::isInitialized()) {
        int staleCount = 0;
        PersistenceStore::executeWithCallback(
            "SELECT query_string, timestamp FROM query_results WHERE query_string LIKE 'wal:%'"s,
            [&staleCount](int, char** values, char**) {
                if (values && values[0]) {
                    Log::Info("AlpineQueryMgr: WAL recovery — stale query entry: "s + values[0]);
                    ++staleCount;
                }
            });

        if (staleCount > 0) {
            Log::Info("AlpineQueryMgr: WAL recovery — clearing "s +
                      std::to_string(staleCount) + " stale entries."s);
            PersistenceStore::execute(
                "DELETE FROM query_results WHERE query_string LIKE 'wal:%'"s);
        }
    }
#endif


    return true;
}



bool
AlpineQueryMgr::createQuery (AlpineQueryOptions &  options,
                             ulong &               queryId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryMgr::createQuery invoked.");
#endif

    // Early initialization check under ReadLock
    {
        ReadLock  lock(activeQueryLock_s);

        if (!initialized_s) {
            Log::Error ("Call to AlpineQueryMgr::createQuery before initialization!");
            return false;
        }
    }

    /////
    // The steps required to create query are:
    //
    // - Get a query ID to identify this new query from here on.
    // - Create the AlpineQuery and AlpineQueryResults objects for this query.
    // - Kick off the query broadcast...
    /////

    // Atomic increment — no lock required.
    queryId = currQueryId_s.fetch_add(1, std::memory_order_relaxed);

    // Build the query state object outside the lock.
    auto newState = std::make_unique<t_QueryState>();

    newState->queryId = queryId;

    uint8_t priority = 128;
    options.getPriority (priority);
    newState->priority = priority;

    newState->query.reset(new AlpineQuery(options));
    newState->results.reset(new AlpineQueryResults(queryId, options));
    // pending only allocated if needed


    // Get list of peers involved in this query for indexing purposes.
    //
    bool  status;
    AlpineQuery::t_PeerIdList   peerIdList;
    status = newState->query->getPeerIdList (peerIdList);

    if (!status) {
        Log::Error ("getPeerIdList for query failed in call to AlpineQueryMgr::createQuery!");
        return false;
    }
    // Activate query — operates on the local newState, not yet in the index.
    status = newState->query->startQuery ();

    if (!status) {
        // This is likely due to invalid options.
        //
        Log::Error ("startQuery failed in call to AlpineQueryMgr::createQuery!");
        return false;
    }

    // Acquire WriteLock only for index mutations.
    {
        WriteLock  lock(activeQueryLock_s);

        // Index state by query ID and member peer ID's
        //
        activeQueryIndex_s->emplace (queryId, std::move(newState));

        ulong  currPeerId;

        for (auto& item : peerIdList) {

            currPeerId = item;

            // If there is an existing queryIdList, we simply add this query ID to the list.
            // Otherwise allocate a new list with this query ID as the sole member.
            //
            auto peerIter = activePeerQueryIndex_s->find (currPeerId);

            if (peerIter == activePeerQueryIndex_s->end ()) {
                auto idList = std::make_unique<t_QueryIdList>();
                idList->push_back (queryId);
                activePeerQueryIndex_s->emplace (currPeerId, std::move(idList));
            }
            else {
                peerIter->second->push_back (queryId);
            }
        }
    }

#ifdef ALPINE_ENABLE_PERSISTENCE
    // Write WAL entry for crash recovery (outside the lock)
    if (PersistenceStore::isInitialized()) {
        string queryString;
        options.getQuery(queryString);

        auto now = static_cast<ulong>(
            std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()).count());

        PersistenceStore::storeQueryResult(
            "wal:"s + std::to_string(queryId), queryString, now);
    }
#endif

    wakeEventLoop();

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

    if (!initialized_s) {
        Log::Error ("Call to AlpineQueryMgr::getQueryStatus before initialization!");
        return false;
    }

    // Check active queries first (hot path)
    {
        ReadLock  lock(activeQueryLock_s);

        auto iter = activeQueryIndex_s->find (queryId);

        if (iter != activeQueryIndex_s->end ()) {
            auto & currState = iter->second;
            std::shared_lock qlock(currState->queryMutex);

            if (!currState->query) {
                double percentage = 100.00;
                queryStatus.setPercentComplete (percentage);
                queryStatus.setIsActive (false);
                return true;
            }
            bool status = currState->query->getStatus (queryStatus);

            if (!status) {
                Log::Error ("AlpineQuery getStatus call failed in call to AlpineQueryMgr::getQueryStatus!");
                return false;
            }
            return true;
        }
    }

    // Fallback: check past queries (cold path)
    {
        ReadLock  lock(pastQueryLock_s);

        auto pastIter = pastQueryIndex_s->find (queryId);

        if (pastIter == pastQueryIndex_s->end ()) {
            Log::Error ("Invalid query ID passed in call to AlpineQueryMgr::getQueryStatus!");
            return false;
        }
        auto & currState = pastIter->second;
        std::shared_lock qlock(currState->queryMutex);

        double percentage = 100.00;
        queryStatus.setPercentComplete (percentage);
        queryStatus.setIsActive (false);

        return true;
    }
}



bool
AlpineQueryMgr::exists (ulong  queryId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryMgr::exists invoked. Query ID: "s +
                std::to_string (queryId));
#endif

    if (!initialized_s) {
        Log::Error ("Call to AlpineQueryMgr::exists before initialization!");
        return false;
    }
    {
        ReadLock  lock(activeQueryLock_s);
        if (activeQueryIndex_s->find (queryId) != activeQueryIndex_s->end ())
            return true;
    }
    {
        ReadLock  lock(pastQueryLock_s);
        return pastQueryIndex_s->find (queryId) != pastQueryIndex_s->end ();
    }
}



bool
AlpineQueryMgr::isActive (ulong  queryId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryMgr::isActive invoked. Query ID: "s +
                std::to_string (queryId));
#endif

    ReadLock  lock(activeQueryLock_s);

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

    ReadLock  lock(activeQueryLock_s);

    if (!initialized_s) {
        Log::Error ("Call to AlpineQueryMgr::inProgress before initialization!");
        return false;
    }
    auto iter = activeQueryIndex_s->find (queryId);

    if (iter == activeQueryIndex_s->end ()) {
        Log::Error ("Invalid query ID passed in call to AlpineQueryMgr::inProgress!");
        return false;
    }

    return iter->second->query->inProgress ();
}



bool
AlpineQueryMgr::pauseQuery (ulong  queryId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryMgr::pauseQuery invoked. Query ID: "s +
                std::to_string (queryId));
#endif

    ReadLock  lock(activeQueryLock_s);

    if (!initialized_s) {
        Log::Error ("Call to AlpineQueryMgr::pauseQuery before initialization!");
        return false;
    }
    bool status;

    auto iter = activeQueryIndex_s->find (queryId);

    if (iter == activeQueryIndex_s->end ()) {
        Log::Error ("Invalid query ID passed in call to AlpineQueryMgr::pauseQuery!");
        return false;
    }
    auto & currState = iter->second;

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

    ReadLock  lock(activeQueryLock_s);

    if (!initialized_s) {
        Log::Error ("Call to AlpineQueryMgr::resumeQuery before initialization!");
        return false;
    }
    bool status;

    auto iter = activeQueryIndex_s->find (queryId);

    if (iter == activeQueryIndex_s->end ()) {
        Log::Error ("Invalid query ID passed in call to AlpineQueryMgr::resumeQuery!");
        return false;
    }
    auto & currState = iter->second;

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

    WriteLock  activeLock(activeQueryLock_s);
    WriteLock  pastLock(pastQueryLock_s);

    if (!initialized_s) {
        Log::Error ("Call to AlpineQueryMgr::cancelQuery before initialization!");
        return false;
    }
    bool status;

    auto queryIter = activeQueryIndex_s->find (queryId);

    if (queryIter == activeQueryIndex_s->end ()) {
        Log::Error ("Invalid query ID passed in call to AlpineQueryMgr::cancelQuery!");
        return false;
    }
    auto & currState = queryIter->second;

    status = currState->query->cancel ();

    if (!status) {
        Log::Error ("Attempt to cancel query failed in AlpineQueryMgr::cancelQuery.  Ending query.");
        return false;
    }

    // Get peer list before releasing query
    AlpineQuery::t_PeerIdList  peerIdList;
    status = currState->query->getPeerIdList (peerIdList);

    if (!status) {
        Log::Error ("Call to query getPeerIdList failed in AlpineQueryMgr::cancelQuery!");
        return false;
    }

    currState->query.reset();

    // remove from the active index, and place state in the past index
    //
    auto statePtr = std::move(queryIter->second);
    activeQueryIndex_s->erase (queryIter);
    pastQueryIndex_s->emplace (queryId, std::move(statePtr));

#ifdef ALPINE_ENABLE_PERSISTENCE
    // Remove WAL entry — query completed or cancelled
    if (PersistenceStore::isInitialized()) {
        PersistenceStore::execute(
            "DELETE FROM query_results WHERE query_string = 'wal:"s +
            std::to_string(queryId) + "'"s);
    }
#endif

    // Remove peer ID indexes for this query from the active index, and
    // move them to the past index.
    //
    ulong currPeerId;

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

        std::erase(*peerQueryIter->second, queryId);

        auto pastPeerIter = pastPeerQueryIndex_s->find (currPeerId);

        if (pastPeerIter == pastPeerQueryIndex_s->end ()) {
            auto idList = std::make_unique<t_QueryIdList>();
            idList->push_back (queryId);
            pastPeerQueryIndex_s->emplace (currPeerId, std::move(idList));
        }
        else {
            pastPeerIter->second->push_back (queryId);
        }
    }


    return true;
}



bool
AlpineQueryMgr::getAllActiveQueryIds (t_QueryIdList  queryIdList)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryMgr::getAllActiveQueryIds invoked.");
#endif

    ReadLock  lock(activeQueryLock_s);

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

    ReadLock  lock(pastQueryLock_s);

    if (!initialized_s) {
        Log::Error ("Call to AlpineQueryMgr::getAllPastQueryIds before initialization!");
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

    ReadLock  lock(activeQueryLock_s);

    if (!initialized_s) {
        Log::Error ("Call to AlpineQueryMgr::getQueryResults before initialization!");
        return false;
    }
    auto iter = activeQueryIndex_s->find (queryId);

    if (iter == activeQueryIndex_s->end ()) {
        Log::Error ("Invalid query ID passed in call to AlpineQueryMgr::getQueryResults!");
        return false;
    }

    results = iter->second->results.get();


    return true;
}



bool
AlpineQueryMgr::registerResultCallback (ulong                queryId,
                                        QueryResultCallback  callback)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryMgr::registerResultCallback invoked. Query ID: "s +
                std::to_string (queryId));
#endif

    ReadLock  lock(activeQueryLock_s);

    if (!initialized_s) {
        Log::Error ("Call to AlpineQueryMgr::registerResultCallback before initialization!");
        return false;
    }

    auto iter = activeQueryIndex_s->find (queryId);

    if (iter == activeQueryIndex_s->end ()) {
        Log::Error ("Invalid query ID passed in call to AlpineQueryMgr::registerResultCallback!");
        return false;
    }

    auto & currState = iter->second;
    std::unique_lock qlock(currState->queryMutex);
    currState->onResultCallback = std::move(callback);

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

    ReadLock  lock(activeQueryLock_s);

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
    auto & peerQueryIdList = iter->second;

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

    ReadLock  lock(pastQueryLock_s);

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
    auto & peerQueryIdList = iter->second;

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

    ReadLock  lock(activeQueryLock_s);

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

    ReadLock  lock(activeQueryLock_s);

    if (!initialized_s) {
        Log::Error ("Call to AlpineQueryMgr::handleQueryOffer before initialization!");
        return false;
    }
    // Verify query ID, and locate state for this query.
    //
    ulong  queryId;
    offerPacket->getQueryId (queryId);

    auto iter = activeQueryIndex_s->find (queryId);

    if (iter == activeQueryIndex_s->end ()) {
#ifdef _VERBOSE
        Log::Error ("Invalid query ID passed in call to AlpineQueryMgr::handleQueryOffer!");
#endif
        AlpinePeerMgr::badPacketReceived (peerId);

        return false;
    }
    auto & currState = iter->second;

    // Acquire per-query mutex for mutation of per-query state.
    std::unique_lock qlock(currState->queryMutex);

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
        currState->pending = std::make_unique<t_PendingReplyIndex>();
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

    ReadLock  lock(activeQueryLock_s);

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

    ReadLock  lock(activeQueryLock_s);

    if (!initialized_s) {
        Log::Error ("Call to AlpineQueryMgr::handleQueryReply before initialization!");
        return false;
    }
    // Verify query ID, and locate state for this query.
    //
    ulong  queryId;
    replyPacket->getQueryId (queryId);

    auto iter = activeQueryIndex_s->find (queryId);

    if (iter == activeQueryIndex_s->end ()) {
#ifdef _VERBOSE
        Log::Error ("Invalid query ID passed in call to AlpineQueryMgr::handleQueryReply!");
#endif
        AlpinePeerMgr::badPacketReceived (peerId);

        return false;
    }
    auto & currState = iter->second;


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

    // Invoke result callback if registered (supports SSE streaming)
    //
    QueryResultCallback callback;
    {
        std::shared_lock qlock(currState->queryMutex);
        callback = currState->onResultCallback;
    }
    if (callback) {
        callback(queryId, peerId);
    }

    wakeEventLoop();

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

    ReadLock  lock(activeQueryLock_s);

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

    ReadLock  lock(activeQueryLock_s);

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
        ReadLock  lock(activeQueryLock_s);

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



std::vector<ulong>
AlpineQueryMgr::processTimedEvents ()
{
#ifdef _VERY_VERBOSE
    Log::Debug ("AlpineQueryMgr::processTimedEvents invoked.");
#endif

    ReadLock  lock(activeQueryLock_s);

    if (!initialized_s) {
        Log::Error ("Call to AlpineQueryMgr::processTimedEvents before initialization!");
        return {};
    }

    // Build a priority-sorted snapshot of active query IDs so that
    // higher-priority queries are processed first each cycle.
    //
    struct PrioritizedQuery {
        ulong    queryId;
        uint8_t  priority;
    };

    std::vector<PrioritizedQuery> sortedQueries;
    sortedQueries.reserve (activeQueryIndex_s->size ());

    for (const auto& [id, state] : *activeQueryIndex_s) {
        sortedQueries.push_back ({id, state->priority});
    }

    // Sort descending by priority (255 = highest, processed first)
    std::sort (sortedQueries.begin (), sortedQueries.end (),
               [](const PrioritizedQuery & a, const PrioritizedQuery & b) {
                   return a.priority > b.priority;
               });

    // Check for completed queries that have async callbacks registered.
    // A query is considered complete when it is no longer in progress.
    //
    std::vector<ulong> completedIds;

    for (const auto& pq : sortedQueries) {
        auto it = activeQueryIndex_s->find (pq.queryId);

        if (it == activeQueryIndex_s->end ())
            continue;

        const auto& state = it->second;
        if (state->query && !state->query->inProgress()) {
            completedIds.push_back(pq.queryId);
        }
    }

    return completedIds;
}



void
AlpineQueryMgr::cleanUp ()
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryMgr::cleanUp invoked.");
#endif

    WriteLock  activeLock(activeQueryLock_s);
    WriteLock  pastLock(pastQueryLock_s);

    if (!initialized_s) {
        Log::Error ("Call to AlpineQueryMgr::cleanUp before initialization!");
        return;
    }

    // unique_ptr containers automatically clean up their elements
    activeQueryIndex_s.reset();
    pastQueryIndex_s.reset();
    activePeerQueryIndex_s.reset();
    pastPeerQueryIndex_s.reset();

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
    auto & queryList = peerIter->second;

    for (auto& queryItem : *queryList) {
        // Locate query state for this query ID
        auto stateIter = activeQueryIndex_s->find (queryItem);

        if (stateIter == activeQueryIndex_s->end ()) {
            Log::Error ("Unable to locate query state returned in ID list in call to "
                                 "AlpineQueryMgr::removePendingRequest!");
            return false;
        }
        auto & currState = stateIter->second;

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
