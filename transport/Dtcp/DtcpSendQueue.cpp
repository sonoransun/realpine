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


#include <DtcpSendQueue.h>
#include <DataBuffer.h>
#include <DtcpBroadcastStates.h>
#include <DtcpBroadcastMgr.h>
#include <DtcpBaseUdpTransport.h>
#include <DtcpConnPacket.h>
#include <DtcpPacket.h>
#include <DtcpBaseConnTransport.h>
#include <Log.h>
#include <StringUtils.h>



DtcpSendQueue::DtcpSendQueue (DtcpBaseUdpTransport *  parentTransport,
                              ulong                   transferRate)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpSendQueue constructor invoked.");
#endif

    parentTransport_      = parentTransport;
    transferRate_         = transferRate;
    byteDelayValue_       = 0;
    nextSendTime_.tv_sec  = 0;
    nextSendTime_.tv_usec  = 0;
    priorityRequestList_  = new t_RequestList;
    normalRequestList_    = new t_RequestList;
    broadcastStates_      = nullptr;
}



DtcpSendQueue::~DtcpSendQueue ()
{
#ifdef _VERBOSE
    Log::Debug ("DtcpSendQueue destructor invoked.");
#endif

    delete priorityRequestList_;

    delete normalRequestList_;
}



bool  
DtcpSendQueue::addRequest (bool          priority,
                           const ulong   ipAddress,
                           const ushort  port,
                           DataBuffer *  dataBuffer)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpSendQueue::addRequest invoked.");
#endif

    if ( (!broadcastStates_) && !locateBroadcastStates () ) {
        // This should never happen...
        //
        Log::Error ("locateBroadcastStates failed in DtcpSendQueue::addRequest!");
        return false;
    }
    // Create state to hold this request until it can be satisfied.
    //
    t_UnicastRequest *  newRequest;
    newRequest = new t_UnicastRequest;

    newRequest->dataBuffer = dataBuffer;
    newRequest->ipAddress  = ipAddress;
    newRequest->port       = port;


    // Put this request in the correct queue depending on priority
    //
    if (priority) {
        priorityRequestList_->push_back (newRequest);
    }
    else {
        normalRequestList_->push_back (newRequest);
    }


    return true;
}



bool  
DtcpSendQueue::setTransferRate (ulong  rate)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpSendQueue::setTransferRate invoked.");
#endif

    // If the limiting rate is over a million bytes per second, just leave
    // throttling off.
    //
    if (rate > 1000000L) {
        transferRate_ = 0;
    }
    else {
        transferRate_ = rate;
    }


    return true;
}



bool  
DtcpSendQueue::queueIdle ()
{
#ifdef _VERBOSE
    Log::Debug ("DtcpSendQueue::queueIdle invoked.");
#endif

    if ( (!broadcastStates_) && !locateBroadcastStates () ) {
        // This should never happen...
        //
        Log::Error ("locateBroadcastStates failed in DtcpSendQueue::queueIdle!");
        return false;
    }
    // Check if we have any requests in any of the send queues
    //
    if (priorityRequestList_->size ()) {
        return false;
    }
    if (normalRequestList_->size ()) {
        return false;
    }
    return !broadcastStates_->requestsPending ();
}


bool  
DtcpSendQueue::requestsPending (ulong & numRequests)
{
    if ( (!broadcastStates_) && !locateBroadcastStates () ) {
        // This should never happen...
        //
        Log::Error ("locateBroadcastStates failed in DtcpSendQueue::requestsPending!");
        return false;
    }
    numRequests = 0;
    numRequests += priorityRequestList_->size ();
    numRequests += normalRequestList_->size ();
    numRequests += broadcastStates_->requestsPending ();


    return true;
}



bool  
DtcpSendQueue::processEvents (struct timeval &  currentDelay)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpSendQueue::processEvents invoked.");
#endif

    if ( (!broadcastStates_) && !locateBroadcastStates () ) {
        // This should never happen...
        //
        Log::Error ("locateBroadcastStates failed in DtcpSendQueue::processEvents!");
        return false;
    }
    // Verify that enough time has elapsed since we last sent a packet.
    //
    bool status;
    status = waitTimeRemaining (nextSendTime_, currentDelay);

    if (status) {
        // Still time remaining for send delay.  Return immediately with
        // the remaing time to delay stored in currentDelay...
        //
        return true;
    }
    // Locate data to send for this time slice.  Priority is as follows:
    // - Priority Data (usually reliable transfer packets)
    // - Normal Data
    // - Broadcast Data
    //
    t_UnicastRequest *  currRequest = nullptr;

    if (priorityRequestList_->size ()) {
        currRequest = priorityRequestList_->front ();
        priorityRequestList_->pop_front ();
    }
    else if (normalRequestList_->size ()) {
        currRequest = normalRequestList_->front ();
        normalRequestList_->pop_front ();
    }

    if ( (!currRequest) && (broadcastStates_->requestsPending () > 0) ) {
        // Nothing in our unicast send queues, check broadcast
        //
        status = getBroadcastRequest (currRequest);

        if (!status) {
            // nothing to send?
            Log::Error ("Broadcast states reporting pending requests, but "
                                 "getBroadcastRequest failed in DtcpSendQueue::processEvents!");
            return false;

        }
        return false;
    }


    // Send request through UDP port
    //
    DataBuffer * buffer;
    byte *       data;
    uint         dataLength;

    buffer = currRequest->dataBuffer;
    status = buffer->getData (data, dataLength);

    if (!status) {
        Log::Error ("getData failed in DtcpSendQueue::processEvents.");
        parentTransport_->releaseTempDataBuffer (buffer);
        return false;
    }
    status = parentTransport_->sendUdpPacket (currRequest->ipAddress, 
                                              currRequest->port,
                                              data,
                                              dataLength);

    struct timeval currTime;
    gettimeofday (&currTime, 0);

    parentTransport_->releaseTempDataBuffer (buffer);
    delete currRequest;


    // Calculate delay between now and next packet.
    //
    calculateDelay (dataLength, currentDelay);

    nextSendTime_ = currTime;
    nextSendTime_.tv_sec  += currentDelay.tv_sec;
    nextSendTime_.tv_usec += currentDelay.tv_usec;

    if (nextSendTime_.tv_usec >= 1000000L) {
        nextSendTime_.tv_usec -= 1000000L;
        nextSendTime_.tv_sec++;
    }


    return status;
}



bool  
DtcpSendQueue::getBroadcastRequest (t_UnicastRequest *&  request)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpSendQueue::getBroadcastRequest invoked.");
#endif

    // Locate an available unicast packet from a broadcast set,
    // prepare data for send.
    //
    if (broadcastStates_->requestsPending () == 0) {
#ifdef _VERBOSE
        Log::Debug ("No pending requests in call to DtcpSendQueue::getBroadcastRequest.");
#endif

        return false;
    }
    bool status;
    StackLinkInterface *     packet;
    DtcpBaseConnTransport *  destination;

    status = broadcastStates_->getCurrentRequest (packet, destination);

    if (!status) {
        Log::Error ("Call to BroadcastStates getCurrentRequest failed "
                             "in DtcpSendQueue::getBroadcastRequest!");

        return false;
    }
    // Verify packet type and settings
    //
    DtcpConnPacket *  connPacket;
    DtcpPacket *   dtcpPacket;
   
    connPacket = dynamic_cast<DtcpConnPacket *>(packet);

    if (!connPacket) {
        Log::Error ("Invalid packet (!DtcpConnPacket *) type passed "
                             "in DtcpSendQueue::getBroadcastRequest!");

        return false;
    }
    dtcpPacket = new DtcpPacket();
    dtcpPacket->setParent (connPacket);


    // Get destination location information
    //
    ulong   myId;
    ulong   ipAddress;
    ushort  port;

    status = destination->getMyId (myId);

    if (!status) {
        Log::Error ("Error retreiving my ID from destiation "
                             "in DtcpSendQueue::getBroadcastRequest!");
        delete dtcpPacket;
        return false;
    }
    status = destination->getPeerLocation (ipAddress, port);

    if (!status) {
        Log::Error ("Error retreiving peer location from destiation "
                             "in DtcpSendQueue::getBroadcastRequest!");
        delete dtcpPacket;
        return false;
    }
    // Configure packet links
    //
    // All we change during broadcasts is the MyID.  The rest
    // of the packet data should remain intact.
    //
    connPacket->setMyId (myId);


    // Allocate & populate new request state for use in processEvents.
    //
    t_UnicastRequest *  newRequest;
    DataBuffer *        buffer;
    newRequest = new t_UnicastRequest;

    status = parentTransport_->getTempDataBuffer (newRequest->dataBuffer);

    if (!status) {
        Log::Error ("Call to DtcpBaseUdpTransport getTempDataBuffer failed "
                             "in DtcpSendQueue::getBroadcastRequest!");
        delete newRequest;
        delete dtcpPacket;

        return false;
    }
    buffer = newRequest->dataBuffer;
    buffer->writeReset ();

    newRequest->ipAddress  = ipAddress;
    newRequest->port       = port;


    // Write packet link data into send buffer and hand back to caller
    //
    status = packet->writeData (buffer);
    delete dtcpPacket;

    if (!status) {
        Log::Debug ("writeData failed for packet "
                             "in DtcpSendQueue::getBroadcastRequest!");
        parentTransport_->releaseTempDataBuffer (buffer);
        delete newRequest;

        return false;
    }
    return true;
}



bool
DtcpSendQueue::locateBroadcastStates ()
{
    bool status;

    if (!broadcastStates_) {
        // Initialize states from Broadcast Manager
        //
        status = DtcpBroadcastMgr::getBroadcastStates (parentTransport_,
                                                       broadcastStates_);

        if (!status) {
            // This should never happen...
            //
            Log::Error ("getBroadcastStates failed in DtcpSendQueue::locateBroadcastStates!");
            broadcastStates_ = nullptr;

            return false;

        }
        return false;
    }


    return true;
}



bool  
DtcpSendQueue::calculateDelay (ulong             dataSize,
                               struct timeval &  delay)
{
    if (dataSize > 400000L) {
        // too large, something is wrong here
        return false;
    }
    delay.tv_sec  = 0;
    delay.tv_usec = 0;

    // If the transfer rate is zero, no throttling...
    //
    if (transferRate_ == 0) {
        return true;
    }
    // Determine the delay in microseconds for each byte at the
    // specified throughput rate.  Use this to determine delay
    // value.  The calculation below will try and keep things
    // accurate to the microsecond if data is < 4k, otherwise
    // accurate to a few hundreds of milliseconds.
    //
    ulong  usDelay;
    ulong  firstMultiplier;
    ulong  secondMultiplier;

    if (dataSize < 4294) {
         firstMultiplier = 1000000;
         secondMultiplier = 1;
    }
    else {
         firstMultiplier = 10000;
         secondMultiplier = 100;
    }

    dataSize *= firstMultiplier;
    usDelay = dataSize / transferRate_;
    usDelay *= secondMultiplier;

    delay.tv_sec = 0;
    delay.tv_usec = usDelay;

    while (delay.tv_usec >= 1000000L) {
        delay.tv_sec++;
        delay.tv_usec -= 1000000L;
    }
 

    return true;
}



bool  
DtcpSendQueue::waitTimeRemaining (struct timeval &  endTime,
                                  struct timeval &  remaining)
{
    struct timeval currTime;
    gettimeofday (&currTime, 0);

    if ( (currTime.tv_sec < endTime.tv_sec) ||
         ((currTime.tv_sec == endTime.tv_sec) && (currTime.tv_usec < endTime.tv_usec)) ) {

        // Still time remaining...
        //
        if (endTime.tv_usec < currTime.tv_usec) {
            remaining.tv_sec = (endTime.tv_sec - currTime.tv_sec) - 1;
            remaining.tv_usec = (endTime.tv_usec + 1000000L) - currTime.tv_usec;
        }
        else {
            remaining.tv_sec = endTime.tv_sec - currTime.tv_sec;
            remaining.tv_usec = endTime.tv_usec - currTime.tv_usec;
        }

        return true;
    }
    return false;
}



