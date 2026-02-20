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


#include <DtcpPendingAckMap.h>
#include <AgedQueue.h>
#include <DtcpBaseUdpTransport.h>
#include <DtcpBaseConnTransport.h>
#include <DataBlock.h>
#include <Log.h>
#include <StringUtils.h>
#include <Platform.h>


const uint  lowInterval  = 500;     // 500 ms
const uint  midInterval  = 2000;    // 2 seconds
const uint  highInterval = 20000;   // 20 seconds



DtcpPendingAckMap::DtcpPendingAckMap (DtcpBaseUdpTransport * udpTransport)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpPendingAckMap constructor invoked.");
#endif

    udpTransport_ = udpTransport;

    recordIndex_ = new t_RecordIndex;
    currId_ = 1;

    lowQueue_  = new AgedQueue;
    midQueue_  = new AgedQueue;
    highQueue_ = new AgedQueue;
}



DtcpPendingAckMap::~DtcpPendingAckMap ()
{
#ifdef _VERBOSE
    Log::Debug ("DtcpPendingAckMap destructor invoked.");
#endif

    delete recordIndex_;

    delete lowQueue_;

    delete highQueue_;
}



bool  
DtcpPendingAckMap::add (DtcpBaseConnTransport * requestor,
                        DataBlock *             data,
                        ulong                   destIpAddress,
                        ushort                  destPort,
                        ulong &                 id)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpPendingAckMap::add invoked.");
#endif


    // assign this request an ID, not a problem if it
    // fails, as we have plenty to choose from.
    //
    id = currId_++;

    gettimeofday (&currTime_, nullptr);


    // Create new record for this reliable transfer
    //
    t_PendingRecord *  newRecord;
    newRecord = new t_PendingRecord;

    newRecord->id                 = id;
    newRecord->requestor          = requestor;
    newRecord->data               = data;
    newRecord->destIpAddress      = destIpAddress;
    newRecord->destPort           = destPort;
    newRecord->sendTime.tv_sec    = currTime_.tv_sec;
    newRecord->sendTime.tv_usec   = currTime_.tv_usec;
    newRecord->resendCount        = 0;
    newRecord->currQueue          = lowQueue_;


    // Place in low interval queue and index by ID.
    //
    bool status;
    status = lowQueue_->add (reinterpret_cast<void *>(newRecord));

    if (!status) {
        Log::Error ("Could not add new record to aged queue in DtcpPendingAckMap::add!");
        return false;
    }
    recordIndex_->emplace (id, newRecord);


    return true;
}



bool  
DtcpPendingAckMap::remove (ulong  id)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpPendingAckMap::remove invoked.");
#endif

    // Locate record for this ID
    //
    auto iter = recordIndex_->find (id);

    if (iter == recordIndex_->end ()) {
        // not found?
        //
#ifdef _VERBOSE
        Log::Debug ("Error locating record ID: "s + std::to_string (id) +
                    " in DtcpPendingAckMap::remove.");
#endif

        return false;
    }
    t_PendingRecord * record;
    record = (*iter).second;


    // Free data block and remove from aged queue
    //
    delete record->data;

    bool status;
    status = record->currQueue->remove (reinterpret_cast<void *>(record));

    if (!status) {
        // This should never happen...
        //
        Log::Error ("Unable to remove record from aged queue in DtcpPendingAckMap::remove.");
        return false;
    }
    // Erase from index, and return success
    //
    recordIndex_->erase (id);
     

    return true;
}



bool  
DtcpPendingAckMap::processTimers ()
{
#ifdef _VERY_VERBOSE
    Log::Debug ("DtcpPendingAckMap::processTimers invoked.");
#endif

    ////
    //
    // The basic process here is to check the oldest records
    // in the low, mid, and high queues.  If the record has
    // a resend timeout, we resend the packet, and increment
    // its resendCount.  If the resend count is max for the
    // queue in which it resides, it is then moved to the next
    // queue.  If a record has maxed out its resend count in
    // the high queue, it has completely expired.  The
    // requesting transport is then notified of the send failure
    // to perform any recover needed.
    //

    gettimeofday (&currTime_, nullptr);
   
 
    // Check queue sizes before processing timeouts to prevent
    // chasing timeout records needlessly.
    //
    bool  checkLowQueue  = false;
    bool  checkMidQueue  = false;
    bool  checkHighQueue = false;

    if (lowQueue_->size ())
        checkLowQueue = true;

    if (midQueue_->size ())
        checkMidQueue = true;

    if (highQueue_->size ())
        checkHighQueue = true;


    // used for processing timed queues
    //
    t_PendingRecord * currRecord  = nullptr;
    t_PendingRecord * firstRecord = nullptr; // prevent extra time checks
    bool   status;
    bool   finished;
    ulong  timeDiff;


    // Check low queue first. 
    // This queue has one resend, then sent to mid queue.  resend count unused.
    //

    if (checkLowQueue) {

#ifdef _VERY_VERBOSE
        Log::Debug ("- Processing low timer queue...");
#endif
       
        firstRecord = nullptr;
        finished = false;

        while (!finished) {

            void *  recordAddr;
            status = lowQueue_->oldest (recordAddr);
            currRecord = reinterpret_cast<t_PendingRecord *>(recordAddr);
 

            if (!status) {
                // This should never happen.  Serious error?
                //
                Log::Error ("Unable to retreive low oldest record in DtcpPendingAckMap::processTimers.");
                return false;

            }
            if (!currRecord) {
                Log::Error ("Null record in pending queue!!!");
                return false;
            }
            if (!firstRecord) {
                // set first record to catch wraparound condition
                firstRecord = currRecord;
            }
            else if (firstRecord == currRecord) {
                // wrap around
                finished = true;
                continue;
            }

            // Check for resend timeout
            //
            timeDiff = msecTimeDiff (currRecord->sendTime, currTime_);

            if (timeDiff > lowInterval) {

                // Timeout, resend packet and place in midQueue_.
                //
#ifdef _VERBOSE
                Log::Debug ("Low timeout for pending packet in DtcpPendingAckMap.  Resending...");
#endif

                udpTransport_->sendData (currRecord->destIpAddress,
                                         currRecord->destPort,
                                         currRecord->data->buffer_,
                                         currRecord->data->length_);

                currRecord->sendTime.tv_sec  = currTime_.tv_sec;
                currRecord->sendTime.tv_usec = currTime_.tv_usec;
                currRecord->resendCount = 0;

                lowQueue_->remove (reinterpret_cast<void *>(currRecord));
                midQueue_->add (reinterpret_cast<void *>(currRecord));

                currRecord->currQueue = midQueue_;

                // if we removed the only entry in the queue, we are finished.
                //
                if (lowQueue_->size () == 0) {
                    finished = true;
                }
            }
            else {
                // stop checking at this point, all remaining records will be newer
                finished = true;
            }
        }

#ifdef _VERY_VERBOSE
        Log::Debug ("- low timer complete...");
#endif
    }

    gettimeofday (&currTime_, nullptr);

    // Check mid interval queue (1 second interval queue)
    // This queue has two resends, then sent to high queue.
    //

    if (checkMidQueue) {
       
#ifdef _VERY_VERBOSE
        Log::Debug ("- Processing mid timer queue...");
#endif

        firstRecord = nullptr;
        finished = false;

        while (!finished) { 

            void *  recordAddr;
            status = midQueue_->oldest (recordAddr);
            currRecord = reinterpret_cast<t_PendingRecord *>(recordAddr);

            if (!status) {
                // This should never happen.  Serious error?
                //
                Log::Error ("Unable to retreive mid oldest record in DtcpPendingAckMap::processTimers.");
                return false;
            }
            if (!firstRecord) {
                // set first record to catch wraparound condition
                firstRecord = currRecord;
            }
            else if (firstRecord == currRecord) {
                // wrap around
                finished = true;
                continue;
            }

            // Check for resend timeout
            //
            timeDiff = msecTimeDiff (currRecord->sendTime, currTime_);

            if (timeDiff > midInterval) {

                // Timeout, resend packet and update or place in highQueue
                //
#ifdef _VERBOSE
                Log::Debug ("Mid timeout for pending packet in DtcpPendingAckMap.  Resending...");
#endif

                udpTransport_->sendData (currRecord->destIpAddress,
                                         currRecord->destPort,
                                         currRecord->data->buffer_,
                                         currRecord->data->length_);

                currRecord->sendTime.tv_sec  = currTime_.tv_sec;
                currRecord->sendTime.tv_usec = currTime_.tv_usec;

                if (currRecord->resendCount >= 1) {
                    // Second timeout, place in high queue...
                    //
                    currRecord->resendCount = 0;

                    midQueue_->remove (reinterpret_cast<void *>(currRecord));
                    highQueue_->add (reinterpret_cast<void *>(currRecord));

                    currRecord->currQueue = highQueue_;

                    // if we removed the only entry in the queue, we are finished.
                    //
                    if (midQueue_->size () == 0) {
                        finished = true;
                    }
                }
                else {
                    // first timeout, touch aged queue location and update next timeout.
                    //
                    currRecord->resendCount++;

                    midQueue_->touch (reinterpret_cast<void *>(currRecord));
                }
            }
            else {
                // stop checking at this point, all remaining records will be newer
                finished = true;
            }
        }

#ifdef _VERY_VERBOSE
        Log::Debug ("- mid timer complete...");
#endif
    }

    gettimeofday (&currTime_, nullptr);

    // Check high interval queue (10 second interval queue)
    // This queue has two resends, then request is failed.
    //

    if (checkHighQueue) {
       
#ifdef _VERY_VERBOSE
        Log::Debug ("- Processing high timer queue...");
#endif

        firstRecord = nullptr;
        finished = false;

        while (!finished) { 

            void *  recordAddr;
            status = highQueue_->oldest (recordAddr);
            currRecord = reinterpret_cast<t_PendingRecord *>(recordAddr);

            if (!status) {
                // This should never happen.  Serious error?
                //
                Log::Error ("Unable to retreive high oldest record in DtcpPendingAckMap::processTimers.");
                return false;
            }
            if (!firstRecord) {
                // set first record to catch wraparound condition
                firstRecord = currRecord;
            }
            else if (firstRecord == currRecord) {
                // wrap around
                finished = true;
                continue;
            }

            // Check for resend timeout
            //
            timeDiff = msecTimeDiff (currRecord->sendTime, currTime_);

            if (timeDiff > highInterval) {

                if (currRecord->resendCount >= 1) {

                    // Second timeout, error
                    //
                    highQueue_->remove (reinterpret_cast<void *>(currRecord));
                    recordIndex_->erase (currRecord->id);

                    // if connTransport requestor is null, this was sent by the udpTransport.
                    //
                    if (currRecord->requestor) {
                        currRecord->requestor->handleSendFailure (currRecord->id);
                    }
                    else {
                        udpTransport_->handleSendFailure (currRecord->id);
                    }

                    delete currRecord->data;
                    delete currRecord;

                    // if we removed the only entry in the queue, we are finished.
                    //
                    if (highQueue_->size () == 0) {
                        finished = true;
                    }
                }
                else {
                    // Timeout, resend packet and update or timeout
                    //
#ifdef _VERBOSE
                    Log::Debug ("High timeout for pending packet in DtcpPendingAckMap.  Resending...");
#endif

                    udpTransport_->sendData (currRecord->destIpAddress,
                                             currRecord->destPort,
                                             currRecord->data->buffer_,
                                             currRecord->data->length_);

                    // first timeout, touch aged queue location and update next timeout.
                    //
                    currRecord->sendTime.tv_sec  = currTime_.tv_sec;
                    currRecord->sendTime.tv_usec = currTime_.tv_usec;
                    currRecord->resendCount++;

                    highQueue_->touch (reinterpret_cast<void *>(currRecord));
                }
            }
            else {
                // stop checking at this point, all remaining records will be newer
                finished = true;
            }
        }

#ifdef _VERY_VERBOSE
        Log::Debug ("- high timer complete...");
#endif
    }

#ifdef _VERY_VERBOSE
    Log::Debug ("processTimers complete...");
#endif


    return true;
}



ulong  
DtcpPendingAckMap::msecTimeDiff (const t_SysTime &  beginTime,
                                 const t_SysTime &  endTime)
{
#ifdef _VERY_VERBOSE
    Log::Debug ("DtcpPendingAckMap::msecTimeDiff invoked.");
#endif

    ulong diff = 0;
    ulong usec;

    diff = endTime.tv_sec - beginTime.tv_sec;
    diff *= 1000;  // convert to milliseconds

    if (endTime.tv_usec > beginTime.tv_usec) {
        usec = endTime.tv_usec - beginTime.tv_usec;
        usec = usec / 1000;
        diff += usec / 1000;
    }
    else {
        usec = (1000000 - beginTime.tv_usec) + endTime.tv_usec;
        usec = usec / 1000;
        diff -= 1000;
        diff += usec;
    }
       

    return diff;
}


