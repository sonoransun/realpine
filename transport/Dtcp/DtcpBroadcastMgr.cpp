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


#include <DtcpBroadcastMgr.h>
#include <DtcpBaseConnTransport.h>
#include <DtcpBroadcast.h>
#include <DtcpBroadcastSet.h>
#include <DtcpBroadcastStates.h>
#include <ReadLock.h>
#include <WriteLock.h>
#include <Log.h>
#include <StringUtils.h>



bool                                             DtcpBroadcastMgr::initialized_s = false;
ulong                                            DtcpBroadcastMgr::currRequestId_s = 1;
DtcpBroadcastMgr::t_BroadcastStatesIndex *       DtcpBroadcastMgr::broadcastStatesIndex_s = nullptr;
DtcpBroadcastMgr::t_RequestIdIndex *             DtcpBroadcastMgr::requestIdIndex_s = nullptr;
ReadWriteSem                                     DtcpBroadcastMgr::dataLock_s;



// Ctor defaulted in header


// Dtor defaulted in header



bool 
DtcpBroadcastMgr::initialize ()
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBroadcastMgr::initialize invoked.");
#endif

    WriteLock  lock(dataLock_s);

    broadcastStatesIndex_s = new t_BroadcastStatesIndex;
    requestIdIndex_s       = new t_RequestIdIndex;

    initialized_s = true;


    return true;
}



bool  
DtcpBroadcastMgr::sendPacket (DtcpBroadcast *      requestor,
                              StackLinkInterface * packet,
                              DtcpBroadcastSet *   destinations,
                              ulong &              requestId)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBroadcastMgr::sendPacket invoked.");
#endif

    if ( (!requestor)     ||
         (!packet)        ||
         (!destinations)  ||
         (destinations->size () == 0) ) {

        // Invalid argument(s)
        //
        Log::Error ("Invalid parameter passed to DtcpBroadcastMgr::sendPacket!");
        return false;
    }


    t_BroadcastRequest *  newRequest;
    newRequest = new t_BroadcastRequest;

    // scope lock
    {
        ReadLock  lock(dataLock_s);
        requestId = currRequestId_s++;
    }

    newRequest->requestId         = requestId;
    newRequest->requestor         = requestor;
    newRequest->packet            = packet;
    newRequest->totalPackets      = destinations->size ();
    newRequest->packetsSent       = 0;
    newRequest->memberStatesList  = new t_BroadcastStatesList;

    t_UdpTransportIndex *  transportIndex;
    transportIndex = new t_UdpTransportIndex;

    bool status;
    status = populateTransportIndex (destinations, transportIndex);

    if (!status) {
        delete newRequest;

        // Cleanup transport index members
        //
        for (const auto& item : *transportIndex) {
            delete item.second;
        }

        delete transportIndex;

        return false;
    }


    // Populate the associated broadcast sets for each Udp transport parent
    //
    WriteLock  lock(dataLock_s);   // MRP_TEMP refine scope

    DtcpBaseUdpTransport *  currUdpTransport;
    DtcpBroadcastStates *   currStates;
    t_TransportList *       currDestinations;

    for (const auto& item : *transportIndex) {

        currUdpTransport = reinterpret_cast<DtcpBaseUdpTransport *>(item.first);
        currDestinations = item.second;
        auto stateIter = broadcastStatesIndex_s->find (static_cast<void *>(currUdpTransport));

        if (stateIter == broadcastStatesIndex_s->end ()) {
            // State does not exist for this UDP parent transport, create one
            //
            currStates = new DtcpBroadcastStates;
            broadcastStatesIndex_s->insert (
              t_BroadcastStatesIndexPair (static_cast<void *>(currUdpTransport), currStates));
        }
        else {
            currStates = (*stateIter).second;
        }

        // Add our list of destination transports to state for this UDP parent transport
        //
        status = currStates->addState (requestId,
                                       packet,
                                       currDestinations);

        if (!status) {
            // Adding requests failed??
            Log::Error ("Unable to add broadcast states for request!");
            break;
        }

        newRequest->memberStatesList->push_back (currStates);
    }


    // Cleanup temporary transport index members
    //
    for (const auto& item : *transportIndex) {
        delete item.second;
    }

    delete transportIndex;


    // Recover gracefully ;) from errors during request delegation
    //
    if (!status) {
        // Cleanup
        //
        for (const auto& item : *newRequest->memberStatesList) {
            currStates = item;
            currStates->removeState (requestId);
        }

        delete newRequest;

        return false;
    }


    // Everything complete, index this new request.
    //
    requestIdIndex_s->emplace (requestId,  newRequest);


    return true;
}



bool  
DtcpBroadcastMgr::populateTransportIndex (DtcpBroadcastSet *     destinations,
                                          t_UdpTransportIndex *  index)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBroadcastMgr::populateTransportIndex invoked.");
#endif

    // For each destination transport specified in the destinations
    // add the request to its states queue.
    //
    t_TransportList   transportList;
    t_TransportList *       currTransportList;
    DtcpBaseConnTransport * currTransport;
    DtcpBaseUdpTransport *  currUdpTransport;

    bool status;
    status = destinations->getTransportList (transportList);

    if (!status) {
        Log::Error ("Unable to get transport list in DtcpBroadcastMgr::populateTransportIndex!");
        return false;
    }


    for (const auto& transItem : transportList) {

        // Obtain UDP transport parent
        //
        currTransport = transItem;
        status = currTransport->getParent (currUdpTransport);

        if (!status) {
            Log::Error ("Unable to get UDP parent transport in "
                                 "DtcpBroadcastMgr::populateTransportIndex!");
            return false;
        }

        // Try to locate Connection transport list for this UDP parent
        //
        auto udpIter = index->find (static_cast<void *>(currUdpTransport));

        if (udpIter == index->end ()) {
            // List does not exist for this UDP parent, create one.
            //
            currTransportList = new t_TransportList;   
            currTransportList->push_back (currTransport);

            index->emplace (static_cast<void *>(currUdpTransport),
                                                    currTransportList);
        }
        else {
            currTransportList = (*udpIter).second;
            currTransportList->push_back (currTransport);
        }
    }


    return true;
}



bool  
DtcpBroadcastMgr::getStatus (ulong    requestId,
                             ulong &  packetsSent)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBroadcastMgr::getStatus invoked.");
#endif

    // Try to locate this request
    //
    ReadLock  lock(dataLock_s);

    auto iter = requestIdIndex_s->find (requestId);

    if (iter == requestIdIndex_s->end ()) {
        // Invalid request ID
        return false;
    }

    t_BroadcastRequest *  request;
    request = (*iter).second;

    packetsSent = request->packetsSent;


    return true;
}

           

bool  
DtcpBroadcastMgr::cancel (ulong  requestId)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBroadcastMgr::cancel invoked.");
#endif

    // Try to locate this request
    //
    WriteLock  lock(dataLock_s);

    auto iter = requestIdIndex_s->find (requestId);

    if (iter == requestIdIndex_s->end ()) {
        // Invalid request ID
        return false;
    }

    t_BroadcastRequest *  request;
    request = (*iter).second;


    // Cancel this request in all assocaited BroadcastStates containers
    //
    t_BroadcastStatesList *          memberStatesList;
    DtcpBroadcastStates *            currStates;

    memberStatesList = request->memberStatesList;

    for (const auto& stateItem : *memberStatesList) {
        currStates = stateItem;
        currStates->removeState (requestId);
    }

    delete memberStatesList;
    delete request;

    requestIdIndex_s->erase (requestId);


    return true;
}



bool  
DtcpBroadcastMgr::getBroadcastStates (DtcpBaseUdpTransport *  transport,
                                      DtcpBroadcastStates *&  broadcastStates)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBroadcastMgr::getBroadcastStates invoked.");
#endif

    // Locate broadcast states object for this parent UDP transport
    //
    WriteLock  lock(dataLock_s);  // use write lock in case we need to insert into index

    auto iter = broadcastStatesIndex_s->find (static_cast<void *>(transport));

    if (iter == broadcastStatesIndex_s->end ()) {
        // No DtcpBroadcastStates object assigned to this transport,
        // create an empty states container and return to caller.
        //
        broadcastStates = new DtcpBroadcastStates;

        broadcastStatesIndex_s->emplace (
                static_cast<void *>(transport), broadcastStates);
    }
    else {
        broadcastStates = (*iter).second;
    }
        

    return true;
}



bool  
DtcpBroadcastMgr::packetSent (ulong  requestId,
                              ulong  transportId)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBroadcastMgr::packetSent invoked.");
#endif

    // Try to locate this request
    //
    WriteLock  lock(dataLock_s);

    auto iter = requestIdIndex_s->find (requestId);

    if (iter == requestIdIndex_s->end ()) {
        // Invalid request ID
        return false;
    }

    t_BroadcastRequest *  request;
    DtcpBroadcast *       requestor;

    request   = (*iter).second;
    requestor = request->requestor;

    request->packetsSent++;


    // If requestor wants packet send notifications, perform handler invocation
    //
    if (requestor->packetNotification_) {
        requestor->handlePacketSend (transportId);
    }


    // If this is the last packet sent for the entire request, notify
    // requestor and cleanup state.
    //
    if (request->packetsSent == request->totalPackets) {
        requestor->broadcastComplete (requestId, request->packetsSent);

        delete request->memberStatesList;
        delete request;

        requestIdIndex_s->erase (requestId);
    }


    return true;
}



