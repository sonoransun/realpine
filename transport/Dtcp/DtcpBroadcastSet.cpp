/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <DtcpBroadcastSet.h>
#include <DtcpBaseConnTransport.h>
#include <DtcpStack.h>
#include <Log.h>
#include <StringUtils.h>



DtcpBroadcastSet::DtcpBroadcastSet ()
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBroadcastSet constructor invoked.");
#endif

    transportArray_ = new t_TransportArray;
    transportIndex_ = new t_TransportIndex;    
    reserve_    = 0;
}



DtcpBroadcastSet::DtcpBroadcastSet (t_TransportList &  transportList)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBroadcastSet transport constructor invoked.");
#endif

    transportArray_ = new t_TransportArray;
    transportIndex_ = new t_TransportIndex;     

    transportArray_->resize (transportList.size ());
    reserve_ = 0;

    // Iterate through each transport and insert into map & array
    //
    bool   status;
    ulong  currId;
    ulong  currIndex = 0;
    DtcpBaseConnTransport *  currTransport;

    for (const auto& item : transportList) {

        currTransport = item;
        status        = currTransport->getTransportId (currId);

        if (!status) {
            Log::Debug ("Could not get transport ID in DtcpBroadcastSet transport constructor!");
            continue;
        }

        (*transportArray_)[currIndex] = currTransport;
        transportIndex_->emplace (currId, currIndex);
        currIndex++;
    }
}



DtcpBroadcastSet::DtcpBroadcastSet (t_TransportIdList &  transportIdList)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBroadcastSet id constructor invoked.");
#endif

    transportArray_ = new t_TransportArray;
    transportIndex_ = new t_TransportIndex;     

    transportArray_->resize (transportIdList.size ());
    reserve_ = 0;

    // Iterate through each transport ID and insert into map & array
    //
    bool   status;
    ulong  currId;
    ulong  currIndex = 0;
    DtcpBaseConnTransport *  currTransport;

    for (auto& item : transportIdList) {

        currId = item;
        status = DtcpStack::locateTransport (currId, currTransport);

        if (!status) {
            Log::Debug ("Could not locate transport by ID in DtcpBroadcastSet id constructor!");
            continue;
        }

        (*transportArray_)[currIndex] = currTransport;
        transportIndex_->emplace (currId, currIndex);
        currIndex++;
    }
}



DtcpBroadcastSet::~DtcpBroadcastSet ()
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBroadcastSet destructor invoked.");
#endif

    delete transportArray_;

    delete transportIndex_;
}



ulong  
DtcpBroadcastSet::size ()
{
    return (transportIndex_->size ());
}



bool  
DtcpBroadcastSet::insert (DtcpBaseConnTransport *  transport)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBroadcastSet::insert transport invoked.");
#endif

    if (reserve_ == 0) {
        // extend array of transports
        //
        uint extent;
        extent = uint(transportArray_->size () / 5) + 1;

        extendTransportArray (extent);
    }

    // Get transport ID
    //
    bool   status;
    ulong  transportId;

    status = transport->getTransportId (transportId);    

    if (!status) {
        // This should never happen
        Log::Error ("Call to getTransportId failed in DtcpBroadcastSet::insert!");
        return false;
    }
    // Verify that this transport is not already a member of the set
    //
    auto iter = transportIndex_->find (transportId);

    if (iter != transportIndex_->end ()) {
        Log::Error ("Attempt to insert duplicate transport in DtcpBroadcastSet::insert!");
        return false;
    }
    // Insert transport into containers
    //
    ulong  currIndex;
    currIndex = transportArray_->size ();

    (*transportArray_)[currIndex] = transport;
    transportIndex_->emplace (transportId, currIndex);

    reserve_--;


    return true;
}



bool  
DtcpBroadcastSet::insert (ulong  transportId)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBroadcastSet::insert id invoked.");
#endif

    if (reserve_ == 0) {
        // extend array of transports
        //
        uint extent;
        extent = uint(transportArray_->size () / 5) + 1;

        extendTransportArray (extent);
    }


    // Verify that this transport is not already a member of the set
    //
    auto iter = transportIndex_->find (transportId);

    if (iter != transportIndex_->end ()) {
        Log::Error ("Attempt to insert duplicate transport in DtcpBroadcastSet::insert!");
        return false;
    }
    // Locate transport with given ID
    //
    bool  status;
    DtcpBaseConnTransport *  transport; 
    status = DtcpStack::locateTransport (transportId, transport);

    if (!status) { 
        // This should not occur
        Log::Debug ("Could not locate transport by ID in DtcpBroadcastSet::insert!");
        return false;
    }
    // Insert transport into containers
    //
    ulong  currIndex;
    currIndex = transportArray_->size ();

    (*transportArray_)[currIndex] = transport;
    transportIndex_->emplace (transportId, currIndex);

    reserve_--;


    return true;
}



bool  
DtcpBroadcastSet::exists (DtcpBaseConnTransport *  transport)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBroadcastSet::exists transport invoked.");
#endif

    bool   status;
    ulong  transportId;

    status = transport->getTransportId (transportId);    

    if (!status) {
        // This should never happen
        Log::Error ("Call to getTransportId failed in DtcpBroadcastSet::exists!");
        return false;
    }
    status = exists (transportId);


    return status;
}



bool  
DtcpBroadcastSet::exists (ulong  transportId)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBroadcastSet::exists id invoked.");
#endif

    return transportIndex_->find (transportId) == transportIndex_->end ();
}



bool  
DtcpBroadcastSet::getTransportList (t_TransportList &  transportList)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBroadcastSet::getTransportList invoked.");
#endif

    transportList.clear ();


    for (const auto& item : *transportArray_) {
        transportList.push_back (item);
    }


    return true;
}



bool  
DtcpBroadcastSet::getTransportIdList (t_TransportIdList &  transportIdList)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBroadcastSet::getTransportIdList invoked.");
#endif

    transportIdList.clear ();


    for (const auto& item : *transportIndex_) {
        transportIdList.push_back (item.first);
    }

    return true;
}



bool  
DtcpBroadcastSet::remove (DtcpBaseConnTransport *  transport)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBroadcastSet::remove transport invoked.");
#endif

    // MRP_TEMP not implemented

    return true;
}



bool  
DtcpBroadcastSet::remove (ulong  transportId)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBroadcastSet::remove id invoked.");
#endif

    // MRP_TEMP not implemented

    return true;
}



bool  
DtcpBroadcastSet::clear ()
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBroadcastSet::clear invoked.");
#endif

    transportArray_->clear ();
    transportIndex_->clear ();

    reserve_   = transportArray_->capacity ();


    return true;
}



bool  
DtcpBroadcastSet::extendTransportArray (uint  extent)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBroadcastSet::extendTransportArray invoked.");
#endif

    uint newSize = transportArray_->size () + extent;
    transportArray_->reserve (newSize);

    reserve_ = extent;


    return true;
}



