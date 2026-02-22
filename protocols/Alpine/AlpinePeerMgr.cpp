/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AlpinePeerMgr.h>
#include <AlpineDtcpConnTransport.h>
#include <AlpinePeerProfile.h>
#include <AlpinePeerProfileIndex.h>
#include <AlpineGroup.h>
#include <AlpineGroupMgr.h>
#include <DtcpStack.h>
#include <AlpineDtcpConnTransport.h>
#include <Log.h>
#include <StringUtils.h>
#include <WriteLock.h>
#include <ReadLock.h>
#include <memory>



bool                                     AlpinePeerMgr::initialized_s = false;
AlpinePeerProfileIndex *                 AlpinePeerMgr::baseProfileIndex_s = nullptr;
ReadWriteSem                             AlpinePeerMgr::dataLock_s;
AlpinePeerMgr::t_PeerInfoIndex *         AlpinePeerMgr::peerInfoIndex_s = nullptr;
ReadWriteSem                             AlpinePeerMgr::peerInfoLock_s;



// Ctor defaulted in header


// Dtor defaulted in header


  
bool  
AlpinePeerMgr::initialize ()
{
#ifdef _VERBOSE
    Log::Debug ("AlpinePeerMgr::initialize invoked.");
#endif

    WriteLock  dataLock(dataLock_s);

    if (initialized_s) {
        Log::Error ("Attempt to reinitialize AlpinePeerMgr!");
        return false;
    }

    WriteLock  peerInfoLock(peerInfoLock_s);

    peerInfoIndex_s    = new t_PeerInfoIndex;
    baseProfileIndex_s = new AlpinePeerProfileIndex;
    initialized_s      = true;


    return true;
}


  
bool  
AlpinePeerMgr::getAlias (ulong     peerId,
                         string &  alias)
{
#ifdef _VERBOSE
    Log::Debug ("AlpinePeerMgr::getAlias invoked.");
#endif

    ReadLock  dataLock(dataLock_s);

    if (!initialized_s) {
        Log::Error ("Call to AlpinePeerMgr::getAlias before initialization!");
        return false;
    }

    ReadLock  lock(peerInfoLock_s);

    auto iter = peerInfoIndex_s->find (peerId);

    if (iter == peerInfoIndex_s->end ()) {
        Log::Error ("Invalid peer ID passed to AlpinePeerMgr::getAlias!");
        return false;
    }

    alias = "";
    t_PeerInfo *  peerInfo = iter->second.get();

    if (peerInfo->alias.has_value()) {
        alias = peerInfo->alias.value();
    }


    return true;
}



bool  
AlpinePeerMgr::setAlias (ulong           peerId,
                         const string &  alias)
{
#ifdef _VERBOSE
    Log::Debug ("AlpinePeerMgr::setAlias invoked.  Values: "s +
                "\n Peer ID: "s + std::to_string (peerId) +
                "\n Alias: "s + alias +
                "\n");
#endif

    ReadLock  dataLock(dataLock_s);

    if (!initialized_s) {
        Log::Error ("Call to AlpinePeerMgr::setAlias before initialization!");
        return false;
    }

    ReadLock  lock(peerInfoLock_s);

    auto iter = peerInfoIndex_s->find (peerId);

    if (iter == peerInfoIndex_s->end ()) {
        Log::Error ("Invalid peer ID passed to AlpinePeerMgr::setAlias!");
        return false;
    }

    t_PeerInfo *  peerInfo = iter->second.get();

    peerInfo->alias = alias;


    return true;
}



bool  
AlpinePeerMgr::getProfile (ulong                peerId,
                           AlpinePeerProfile &  profile)
{
#ifdef _VERBOSE
    Log::Debug ("AlpinePeerMgr::getProfile invoked.");
#endif

    ReadLock  dataLock(dataLock_s);

    if (!initialized_s) {
        Log::Error ("Call to AlpinePeerMgr::getProfile before initialization!");
        return false;
    }

    bool status;
    AlpinePeerProfile *  currProfile;

    status = baseProfileIndex_s->locate (peerId, currProfile);

    if (!status) {
        Log::Error ("Locating base profile failed in AlpinePeerMgr::getProfile!");
        return false;
    }

    profile = *currProfile;


    return true;
}



bool  
AlpinePeerMgr::getGroupProfile (ulong                peerId,
                                ulong                groupId,
                                AlpinePeerProfile &  profile)
{
#ifdef _VERBOSE
    Log::Debug ("AlpinePeerMgr::getGroupProfile invoked.");
#endif

    ReadLock  dataLock(dataLock_s);

    if (!initialized_s) {
        Log::Error ("Call to AlpinePeerMgr::getGroupProfile before initialization!");
        return false;
    }

    bool status;
    AlpineGroup *  group;
    status = AlpineGroupMgr::locateGroup (groupId, group);

    if (!status) {
        Log::Error ("Could not locate group in AlpinePeerMgr::getGroupProfile!");
        return false;
    }

    AlpinePeerProfile *  currProfile;
    status = group->getPeerProfile (peerId, currProfile);

    if (!status) {
        Log::Error ("Locating peer profile failed in AlpinePeerMgr::getGroupProfile!");
        return false;
    }

    profile = *currProfile;


    return true;
}



bool  
AlpinePeerMgr::getTransport (ulong                       peerId,
                             AlpineDtcpConnTransport *&  transport)
{
#ifdef _VERBOSE
    Log::Debug ("AlpinePeerMgr::getTransport invoked.");
#endif

    ReadLock  dataLock(dataLock_s);

    if (!initialized_s) {
        Log::Error ("Call to AlpinePeerMgr::getTransport before initialization!");
        return false;
    }

    bool status;
    DtcpBaseConnTransport *  baseTransport;

    status = DtcpStack::locateTransport (peerId, baseTransport);

    if (!status) {
        Log::Error ("DTCP Locate transport failed in AlpinePeerMgr::getTransport!");
        return false;
    }

    transport = dynamic_cast<AlpineDtcpConnTransport *>(baseTransport);

    if (!transport) {
        Log::Error ("Invalid transport type returned in AlpinePeerMgr::getTransport!");
        return false;
    }


    return true;
}



bool  
AlpinePeerMgr::querySent (ulong  peerId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpinePeerMgr::querySent invoked.  Peer ID: "s +
                std::to_string (peerId));
#endif

    ReadLock  dataLock(dataLock_s);

    if (!initialized_s) {
        Log::Error ("Call to AlpinePeerMgr::querySent before initialization!");
        return false;
    }
    
    // Update the base profile for this peer
    //
    bool  status;
    status = baseProfileIndex_s->querySent (peerId);

    if (!status) {
        Log::Error ("Updating base profile for peer failed in AlpinePeerMgr::responseReceived!");
        return false;
    }


    return true;
}



bool  
AlpinePeerMgr::responseReceived (ulong  peerId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpinePeerMgr::responseReceived invoked.  Peer ID: "s +
                std::to_string (peerId));
#endif

    ReadLock  dataLock(dataLock_s);

    if (!initialized_s) {
        Log::Error ("Call to AlpinePeerMgr::responseReceived before initialization!");
        return false;
    }

    // Update the base profile for this peer
    //
    bool  status;
    status = baseProfileIndex_s->responseReceived (peerId);

    if (!status) {
        Log::Error ("Updating base profile for peer failed in AlpinePeerMgr::responseReceived!");
        return false;
    }


    return true;
}



bool  
AlpinePeerMgr::registerTransport (ulong                      peerId,
                                  AlpineDtcpConnTransport *  transport)
{
#ifdef _VERBOSE
    Log::Debug ("AlpinePeerMgr::registerTransport invoked.  Peer ID: "s +
                std::to_string (peerId));
#endif

    ReadLock  dataLock(dataLock_s);

    if (!initialized_s) {
        Log::Error ("Call to AlpinePeerMgr::registerTransport before initialization!");
        return false;
    }


    // Create peer information
    //
    auto  peerInfo = std::make_unique<t_PeerInfo>();

    peerInfo->peerId     = peerId;
    peerInfo->transport  = transport;
    peerInfo->alias      = std::nullopt;

    // Register peer with the default group.
    //
    bool status;
    AlpineGroup *  defaultGroup;

    status = AlpineGroupMgr::getDefaultGroup (defaultGroup);

    if (!status) {
        Log::Error ("Locating default peer group failed in AlpinePeerMgr::registerTransport!");

        return false;
    }

    status = defaultGroup->addPeer (peerId);

    if (!status) {
        Log::Error ("Add peer to default group failed in AlpinePeerMgr::registerTransport!");

        return false;
    }

    ulong  groupId;
    defaultGroup->getId (groupId);

    peerInfo->groupList  = std::make_unique<t_GroupIdList>();
    peerInfo->groupList->push_back (groupId);


    // Add peer info to index
    //
    {
        WriteLock  lock(peerInfoLock_s);

        peerInfoIndex_s->emplace (peerId, std::move(peerInfo));
    }

    // Add peer to base profile
    //
    status = baseProfileIndex_s->create (peerId);

    if (!status) {
        Log::Error ("Adding peer to base profile failed in AlpinePeerMgr::registerTransport!");
        return false;
    }


    return true;
}



bool  
AlpinePeerMgr::deactivatePeer (ulong  peerId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpinePeerMgr::deactivatePeer invoked.  Peer ID: "s +
                std::to_string (peerId));
#endif

    // MRP_TEMP implement!

    return true;
}



bool  
AlpinePeerMgr::deletePeer (ulong  peerId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpinePeerMgr::deletePeer invoked.  Peer ID: "s +
                std::to_string (peerId));
#endif

    // MRP_TEMP implement!

    return true;
}



bool  
AlpinePeerMgr::banPeer (ulong  peerId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpinePeerMgr::banPeer invoked.  Peer ID: "s +
                std::to_string (peerId));
#endif

    // MRP_TEMP implement!

    return true;
}
                                 


bool  
AlpinePeerMgr::badPacketReceived (ulong  peerId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpinePeerMgr::badPacketReceived invoked.  Peer ID: "s +
                std::to_string (peerId));
#endif

    ReadLock  dataLock(dataLock_s);

    if (!initialized_s) {
        Log::Error ("Call to AlpinePeerMgr::badPacketReceived before initialization!");
        return false;
    }

    // Update the base profile for this peer.  Bad packets are ONLY tracked here, and not per group
    // like most other affinity/quality stats.
    //
    bool  status;
    status = baseProfileIndex_s->badPacketReceived (peerId);

    if (!status) {
        Log::Error ("Updating base profile for peer failed in AlpinePeerMgr::badPacketReceived!");
        return false;
    }


    return true;
}



bool  
AlpinePeerMgr::reliableTransferFailed (ulong  peerId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpinePeerMgr::reliableTransferFailed invoked.  Peer ID: "s +
                std::to_string (peerId));
#endif

    ReadLock  dataLock(dataLock_s);

    if (!initialized_s) {
        Log::Error ("Call to AlpinePeerMgr::reliableTransferFailed before initialization!");
        return false;
    }

    // Update the base profile for this peer.  Transfer failures only tracked at global level.
    //
    bool  status;
    status = baseProfileIndex_s->reliableTransferFailed (peerId);

    if (!status) {
        Log::Error ("Updating base profile for peer failed in AlpinePeerMgr::badPacketReceived!");
        return false;
    }


    return true;
}



void  
AlpinePeerMgr::processTimedEvents ()
{
#ifdef _VERY_VERBOSE
    Log::Debug ("AlpinePeerMgr::processTimedEvents invoked.");
#endif
}



