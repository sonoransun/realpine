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


#include <DtcpBroadcastStates.h>
#include <DtcpBroadcastMgr.h>
#include <DtcpBaseConnTransport.h>
#include <ReadLock.h>
#include <WriteLock.h>
#include <Log.h>
#include <StringUtils.h>



DtcpBroadcastStates::DtcpBroadcastStates ()
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBroadcastStates constructor invoked.");
#endif

    totalRequests_ = 0;
    requestList_.clear ();
}



DtcpBroadcastStates::~DtcpBroadcastStates ()
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBroadcastStates destructor invoked.");
#endif
}



bool 
DtcpBroadcastStates::addState (ulong                 requestId,
                               StackLinkInterface *  packet,
                               t_TransportList *     destinations)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBroadcastStates::addState invoked.");
#endif

    WriteLock  lock(dataLock_);

    if (destinations->size () <= 0) {
        // We must have at least one destination
        //
        Log::Error ("No destinations given in call to DtcpBroadcastStates::addState!");
        return false;
    }

    t_RequestData *  newRequest;
    newRequest = new t_RequestData;

    newRequest->requestId             = requestId;
    newRequest->packet                = packet;
    newRequest->remainingDestinations = destinations;

    requestList_.push_back (newRequest);

    totalRequests_ += destinations->size ();


    return true;
}



bool 
DtcpBroadcastStates::removeState (ulong  requestId)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBroadcastStates::removeState invoked.");
#endif

    WriteLock  lock(dataLock_);

    // Attempt to locate the state with this ID.
    //
    bool  found = false;
    t_RequestData *  currRequest;

    for (auto iter = requestList_.begin (); iter != requestList_.end (); ++iter) {
        currRequest = (*iter);

        if (currRequest->requestId == requestId) {
            requestList_.erase (iter);
            totalRequests_ -= currRequest->remainingDestinations->size ();
            found = true;
            break;
        }
    }


    return found;
}



ulong  
DtcpBroadcastStates::requestsPending ()
{
    return totalRequests_;
}



bool  
DtcpBroadcastStates::getCurrentRequest (StackLinkInterface *&     packet,
                                        DtcpBaseConnTransport *&  destination)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBroadcastStates::getCurrentRequest invoked.");
#endif

    ReadLock  lock(dataLock_);

    // Pop first available request from list.
    //
    if (requestList_.size () <= 0) {
        return false;
    }

    t_RequestData *  currRequest;
    ulong  requestId;

    currRequest = requestList_.front ();
    requestList_.pop_front ();

    packet      = currRequest->packet;
    requestId   = currRequest->requestId;
    destination = currRequest->remainingDestinations->front ();
    currRequest->remainingDestinations->pop_front ();

    if (currRequest->remainingDestinations->size () == 0) {
        // We have completed sending packets for this request,
        // remove this request from our list of active requests.
        //
        delete currRequest;
    }
    else {
        requestList_.push_back (currRequest);
    }

    ulong  transportId;
    destination->getTransportId (transportId);

    // Let the broadcast manager know that we have sent another packet
    //
    DtcpBroadcastMgr::packetSent (requestId, transportId);

    totalRequests_--;


    return true;
}



