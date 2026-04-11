/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AlpineBroadcast.h>
#include <AlpineGroup.h>
#include <AlpineGroupMgr.h>
#include <AlpinePacket.h>
#include <AlpineQuery.h>
#include <AlpineQueryPacket.h>
#include <DtcpBroadcastSet.h>
#include <Log.h>
#include <ReadLock.h>
#include <StringUtils.h>
#include <WriteLock.h>


AlpineQuery::AlpineQuery(AlpineQueryOptions & options)
    : options_(options)
{
#ifdef _VERBOSE
    Log::Debug("AlpineQuery constructor invoked.");
#endif

    WriteLock lock(dataLock_);

    queryActive_ = false;

    string groupName;
    options_.getGroup(groupName);

    bool status;
    group_ = nullptr;
    status = AlpineGroupMgr::locateGroup(groupName, group_);

    if (!status) {
        Log::Error("Error locating group: "s + groupName + " in AlpineQuery constructor!");

        // as workaround, get default group.
        //
        AlpineGroupMgr::getDefaultGroup(group_);
        return;
    }

    broadcast_ = nullptr;
    alpinePacket_ = new AlpinePacket;
    queryPacket_ = new AlpineQueryPacket;

    alpinePacket_->setParent(queryPacket_);

    // MRP_TEMP prepare query packet from provided options
}


AlpineQuery::~AlpineQuery()
{
#ifdef _VERBOSE
    Log::Debug("AlpineQuery destructor invoked.");
#endif

    cancel();

    WriteLock lock(dataLock_);

    delete alpinePacket_;

    delete queryPacket_;
}


bool
AlpineQuery::startQuery()
{
#ifdef _VERBOSE
    Log::Debug("AlpineQuery::startQuery invoked.");
#endif

    WriteLock lock(dataLock_);

    if (queryActive_) {
        Log::Error("Query already active in call to AlpineQuery::startQuery!");
        return false;
    }
    if (!group_) {
        Log::Error("No group identified in AlpineQuery::startQuery!");
        return false;
    }
    // Get the broadcast list ordered by quality preference.
    //
    bool status;
    AlpineGroup::t_PeerIdList peerList;

    status = group_->createPeerList(peerList);

    if (!status) {
        Log::Error("Unable to create peer list for broadcast in AlpineQuery::startQuery!");
        return false;
    }
    // Create broadcast set from peer list, use for creation of AlpineBroadcast to begin query
    //
    DtcpBroadcastSet * broadcastSet;
    broadcastSet = new DtcpBroadcastSet(peerList);

    broadcast_ = new AlpineBroadcast(broadcastSet);
    delete broadcastSet;

    status = group_->queryStart(this);

    if (!status) {
        Log::Error("Call to AlpineGroup queryStart failed in AlpineQuery::startQuery!");
        return false;
    }
    // Everything is ready, kick off broadcast.
    //
    broadcast_->packetSendNotifications(true);
    status = broadcast_->sendPacket(alpinePacket_);

    if (!status) {
        Log::Error("Broadcast sendPacket failed in AlpineQuery::startQuery!");
        return false;
    }
    queryActive_ = true;


    return true;
}


bool
AlpineQuery::inProgress()
{
#ifdef _VERBOSE
    Log::Debug("AlpineQuery::inProgress invoked.");
#endif

    ReadLock lock(dataLock_);


    return true;
}


bool
AlpineQuery::getStatus(AlpineQueryStatus & queryStatus)
{
#ifdef _VERBOSE
    Log::Debug("AlpineQuery::getStatus invoked.");
#endif

    ReadLock lock(dataLock_);


    return true;
}


bool
AlpineQuery::getPeerIdList(t_PeerIdList & peerIdList)
{
#ifdef _VERBOSE
    Log::Debug("AlpineQuery::getPeerIdList invoked.");
#endif

    ReadLock lock(dataLock_);


    return true;
}


bool
AlpineQuery::halt()
{
#ifdef _VERBOSE
    Log::Debug("AlpineQuery::halt invoked.");
#endif

    WriteLock lock(dataLock_);


    return true;
}


bool
AlpineQuery::resume()
{
#ifdef _VERBOSE
    Log::Debug("AlpineQuery::resume invoked.");
#endif

    WriteLock lock(dataLock_);


    return true;
}


bool
AlpineQuery::cancel()
{
#ifdef _VERBOSE
    Log::Debug("AlpineQuery::cancel invoked.");
#endif

    WriteLock lock(dataLock_);


    return true;
}


bool
AlpineQuery::packetSent(ulong transportId)
{
#ifdef _VERBOSE
    Log::Debug("AlpineQuery::packetSent invoked.  Peer ID: "s + std::to_string(transportId));
#endif

    WriteLock lock(dataLock_);


    return true;
}


bool
AlpineQuery::broadcastComplete(ulong numSent, struct timeval & duration)
{
#ifdef _VERBOSE
    Log::Debug("AlpineQuery::broadcastComplete invoked.  Total Sent: "s + std::to_string(numSent));
#endif

    WriteLock lock(dataLock_);


    return true;
}
