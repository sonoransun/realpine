/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <DtcpBroadcast.h>
#include <DtcpBroadcastMgr.h>
#include <DtcpBroadcastSet.h>
#include <StackLinkInterface.h>
#include <Log.h>
#include <StringUtils.h>
#include <WriteLock.h>
#include <ReadLock.h>



DtcpBroadcast::DtcpBroadcast (DtcpBroadcastSet *  destinations)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBroadcast constructor invoked.");
#endif

    destinations_       = destinations;
    active_             = false;
    requestId_          = 0;
    startTime_.tv_sec   = 0;
    startTime_.tv_usec  = 0;
    packetNotification_ = false;
}

    

DtcpBroadcast::~DtcpBroadcast ()
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBroadcast destructor invoked.");
#endif

    cancel ();
}



bool  
DtcpBroadcast::sendPacket (StackLinkInterface * packet)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBroadcast::sendPacket invoked.");
#endif

    WriteLock  lock(dataLock_);
 
    if (active_) {
        // Cannot perform multiple broadcast requests!
        //
        Log::Error ("Attempt to perform multiple broadcasts in DtcpBroadcast::sendPacket!");
        return false;
    }
    if (!destinations_) {
        // Initialized with null destination???
        //
        Log::Error ("Attempt to perform broadcast with NULL destination in DtcpBroadcast::sendPacket!");
        return false;
    }
    bool status;
    status = DtcpBroadcastMgr::sendPacket (this,
                                           packet, 
                                           destinations_,
                                           requestId_);

    if (!status) {
        Log::Debug ("Broadcast Manager sendPacket request failed in DtcpBroadcast::sendPacket!");
        return false;
    }
    // Request is in progress, update state
    // 
    active_ = true;
    gettimeofday (&startTime_, 0);


    return true;
}



bool  
DtcpBroadcast::sending ()
{
    return active_;
}



bool  
DtcpBroadcast::numDestinations (ulong &  destinationCount)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBroadcast::numDestinations invoked.");
#endif

    ReadLock  lock(dataLock_);
 
    if (!destinations_) {
        // Initialized with null destination???
        //
        Log::Error ("Call to DtcpBroadcast::numDestinations with NULL destination!");
        return false;
    }
    destinationCount = destinations_->size (); 


    return true;
}



bool  
DtcpBroadcast::packetsSent (ulong &  numSent)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBroadcast::packetsSent invoked.");
#endif

    ReadLock  lock(dataLock_);
 
    if (!active_) {
        // Must have a broadcast in progress
        //
        Log::Error ("Call to DtcpBroadcast::packetsSent without broadcast in progress!");
        return false;
    }
    bool status;
    status = DtcpBroadcastMgr::getStatus (requestId_, numSent);

    return status;
}



bool  
DtcpBroadcast::percentComplete (double &  percentage)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBroadcast::percentComplete invoked.");
#endif

    ReadLock  lock(dataLock_);
 
    if (!active_) {
        // Must have a broadcast in progress
        //
        Log::Error ("Call to DtcpBroadcast::percentComplete without broadcast in progress!");
        return false;
    }
    bool status;
    double  totalPackets;
    double  packetsSent;
    ulong   numSent;

    status = DtcpBroadcastMgr::getStatus (requestId_, numSent);

    if (!status) {
        // This should not occur
        //
        Log::Error ("Unable to get status information in DtcpBroadcast::percentComplete!");
        return false;
    }
    totalPackets = destinations_->size (); 
    packetsSent  = numSent;
    percentage   = packetsSent / totalPackets;


    return true;
}



bool  
DtcpBroadcast::pause ()
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBroadcast::pause invoked.");
#endif 


    return true;
}



bool  
DtcpBroadcast::resume ()
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBroadcast::resume invoked.");
#endif


    return true;
}



bool  
DtcpBroadcast::cancel ()
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBroadcast::cancel invoked.");
#endif

    WriteLock  lock(dataLock_);

    if (!active_) {
        // Must have a broadcast in progress to cancel
        //
        Log::Error ("Call to DtcpBroadcast::cancel without broadcast in progress!");
        return false;
    }
    bool status;
    status = DtcpBroadcastMgr::cancel (requestId_);

    active_ = false;

    if (!status) {
        // This should never happen
        //
        Log::Error ("Call to BroadcastMgr::cancel failed in DtcpBroadcast!");
        return false;
    }
    return true;
}



bool  
DtcpBroadcast::packetSendNotifications (bool  used)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBroadcast::packetSendNotifications invoked.");
#endif

    WriteLock  lock(dataLock_);

    packetNotification_ = used;

    return true;
}



bool  
DtcpBroadcast::broadcastComplete (ulong  requestId,
                                  ulong  totalSent)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBroadcast::broadcastComplete invoked.");
#endif

    WriteLock  lock(dataLock_);

    // Calculate duration and invoke derived handler to notify of completion.
    //
    struct timeval  duration; 
    struct timeval  now;
    gettimeofday (&now, 0);

    duration.tv_sec  = now.tv_sec  - startTime_.tv_sec;

    if (now.tv_usec > startTime_.tv_usec) {
        duration.tv_usec = now.tv_usec - startTime_.tv_usec;
    }
    else {
        // carry microseconds
        duration.tv_sec--;
        duration.tv_usec = (now.tv_usec + 1000000L) - startTime_.tv_usec;
    }
    
    handleSendComplete (totalSent,
                        duration);
  

    return true;
}



