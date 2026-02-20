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


#include <DtcpThreadTable.h>
#include <DtcpStackThread.h>
#include <DtcpIORecord.h>
#include <DataBuffer.h>
#include <ReadLock.h>
#include <WriteLock.h>
#include <Log.h>
#include <StringUtils.h>



static const int  defaultBufferSize = 5*1024;  // 5K



DtcpThreadTable::DtcpThreadTable (DtcpBaseUdpTransport * udpTransport,
                                  ushort                 maxThreads,
                                  uint                   maxRecords,
                                  uint                   maxBuffers)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpThreadTable constructor invoked."s +
                "\nMaxThreads: "s + std::to_string(maxThreads) +
                "\nMaxRecords: "s + std::to_string(maxRecords) +
                "\nMaxBuffers: "s + std::to_string(maxBuffers) +
                "\n");
#endif

    WriteLock  recordLock(recordLock_);
    WriteLock  threadLock(threadLock_);
    WriteLock  indexLock(indexLock_);
    WriteLock  bufferLock(bufferLock_);

    udpTransport_ = udpTransport;
    maxThreads_   = maxThreads;
    maxRecords_   = maxRecords;
    maxBuffers_   = maxBuffers;

    // allocate a DtcpIORecord and DataBuffer for each maxThread...
    // 
    uint i;
    DtcpIORecord * newRecord;

    for (i = 0; i < maxThreads; i++) {
        newRecord = new DtcpIORecord (defaultBufferSize);
        allRecordList_.push_back (newRecord);
        availableRecordList_.push_back (newRecord);
    }

    DataBuffer * newBuffer;

    for (i = 0; i < maxThreads; i++) {
        newBuffer = new DataBuffer (defaultBufferSize);
        allBufferList_.push_back (newBuffer);
        availableBufferList_.push_back (newBuffer);
    }
}



DtcpThreadTable::~DtcpThreadTable ()
{
#ifdef _VERBOSE
    Log::Debug ("DtcpThreadTable destructor invoked.");
#endif

    // cleanup all allocated threads and IO records...
    //
    for (const auto& record : allRecordList_) {
        delete record;
    }


    for (const auto& thread : allThreadList_) {
        delete thread;
    }
}



bool  
DtcpThreadTable::allocateRecord (DtcpIORecord *&  record)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpThreadTable::allocateRecord invoked.");
#endif

    record = nullptr;

    WriteLock  lock(recordLock_);

    if (!availableRecordList_.empty()) {
        record = availableRecordList_.front ();
        availableRecordList_.pop_front ();

        return true;

    }
    if (allRecordList_.size () < maxRecords_) {
        record = new DtcpIORecord (defaultBufferSize);
        allRecordList_.push_back (record);

        return true;
    }
    // reached our limit!
    //
    Log::Error ("Maximum number of DtcpIORecords allocated; none available.");


    return false;
}



void
DtcpThreadTable::returnRecord (DtcpIORecord * record)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpThreadTable::returnRecord invoked.");
#endif

    if (!record) {
        Log::Error ("Null record passed to DtcpThreadTable::returnRecord.  Ignoring.");
        return;
    }

    WriteLock  lock(recordLock_);

    availableRecordList_.push_back (record);
}



bool  
DtcpThreadTable::allocateBuffer (DataBuffer *&  buffer)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpThreadTable::allocateBuffer invoked.");
#endif

    buffer = nullptr;

    WriteLock  lock(bufferLock_);

    if (!availableBufferList_.empty()) {
        buffer = availableBufferList_.front ();
        availableBufferList_.pop_front ();

        return true;

    }
    if (allBufferList_.size () < maxBuffers_) {
        buffer = new DataBuffer (defaultBufferSize);
        allBufferList_.push_back (buffer);

        return true;
    }
    // reached our limit!
    //
    Log::Error ("Maximum number of DataBuffers allocated; none available.");


    return true;
}



bool  
DtcpThreadTable::releaseBuffer (DataBuffer * buffer)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpThreadTable::releaseBuffer invoked.");
#endif

    if (!buffer) {
        Log::Error ("Null buffer passed to DtcpThreadTable::releaseBuffer!");
        return false;
    }
    WriteLock  lock(bufferLock_);
    availableBufferList_.push_back (buffer);


    return true;
}



bool  
DtcpThreadTable::processRecord (DtcpIORecord * record)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpThreadTable::processRecord invoked.");
#endif

    if (!record) {
        Log::Error ("Null record passed to DtcpThreadTable::processRecord.  Ignoring.");
        return false;
    }
    // throw record onto the queue...
    //
    {
        WriteLock  lock(recordLock_);
        recordQueue_.push_back (record);
    }


    // If an idle thread is available to process this request,
    // wake it up...
    //
    bool  resumeThread = false;
    {
        ReadLock  lock(threadLock_);

        if (!idleThreadList_.empty()) {
            resumeThread = true;
        }
    }

    if (resumeThread) {
        wakeThread ();
        return true;
    }
    // all threads busy, see if we can create one...
    //
    bool newThread = false;
    {
        ReadLock  lock(threadLock_);
    
        if (allThreadList_.size () < maxThreads_) {
            newThread = true;
        }
    }

    if (newThread) {
        createThread ();
        return true;
    }
    // All threads are busy, no more allowed.  Once a thread
    // becomes available it will process the pending records.
   
 
    return true;
}



bool
DtcpThreadTable::processComplete (t_ThreadId  id)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpThreadTable::processComplete invoked for thread ID: "s +
                threadIdToString (id));
#endif

    // Remove thread / record from active index and add to available queue.
    //
    DtcpIORecord * oldRecord = nullptr;

    {
        WriteLock  lock(indexLock_);

        auto iter = activeThreadIndex_.find (id);

        if (iter == activeThreadIndex_.end ()) {
            // serious error, this should not occur
            Log::Error ("Unable to locate active thread record "
                                 "in DtcpThreadTable::processComplete!");

            return false;
        }
        oldRecord = (*iter).second;

        if (!oldRecord) {
            // serious error, this should not occur
            Log::Error ("Active thread record is NULL "
                                 "in DtcpThreadTable::processComplete!");

            return false;
        }
        activeThreadIndex_.erase (id);
    }


    // Add IORecord to available list.
    //
    {
        WriteLock  lock(recordLock_);

        availableRecordList_.push_back (oldRecord);
    }


    return true;
}



bool  
DtcpThreadTable::locateRecord (t_ThreadId       threadId,
                               DtcpIORecord *&  record)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpThreadTable::locateRecord invoked.");
#endif

    // locate record associated with this thread...
    //

    record = nullptr;

    // scope lock
    {
        ReadLock  lock(indexLock_);

        auto iter = activeThreadIndex_.find (threadId);

        if (iter != activeThreadIndex_.end ()) {
            record = (*iter).second;
        }


        // if we did not find the record in the active thread index,
        // check the external index
        //
        if (!record) {
            iter = externalThreadIndex_.find (threadId);

            if (iter != externalThreadIndex_.end ()) {
                record = (*iter).second;
            }
        }
    }

    if (record) {
        // found a match
        return true;
    }
    // at this point we need to associate an available DtcpIORecord with
    // this external thread.
    //
    bool status;
    status = allocateRecord (record);

    if (!status) {
        // no more free records?
        Log::Error ("Unable to allocate DtcpIORecord for this thread.");

        return false;
}
#ifdef _VERBOSE
    Log::Debug ("Allocated new record for this external thread.");
#endif


    // Index new record with this thread.
    //
    WriteLock  lock(indexLock_);

    externalThreadIndex_.emplace (threadId, record);


    return true;
}


    
bool  
DtcpThreadTable::getNextRecord (t_ThreadId       threadId,
                                DtcpIORecord *&  record)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpThreadTable::getNextRecord invoked for thread ID: "s +
                threadIdToString (threadId));
#endif

    // If there is anything in the queue, pull it now.
    // otherwise, return and let thread go to idle.
    //
    record = nullptr;

    {
        WriteLock  lock(recordLock_);

        if (!recordQueue_.empty()) {
            record = recordQueue_.front ();
            recordQueue_.pop_front ();
        }
    }

    if (!record) {
        // nothing in the queue...
        return false;
    }
    // perform required setup to associate this record with this thread,
    // hand back to calling thread to process.
    //
    DtcpIORecord *   oldRecord = nullptr;

    {
        WriteLock  lock(indexLock_);

        auto iter = activeThreadIndex_.find (threadId);

        if (iter != activeThreadIndex_.end ()) {
            oldRecord = (*iter).second;
            activeThreadIndex_.erase (threadId);
        }

        // index new record with this thread.
        //
        activeThreadIndex_.emplace (threadId, record);
    }

    if (oldRecord) {
        // add old record back on available list.
        //
        {
            WriteLock  lock(recordLock_);

            availableRecordList_.push_back (oldRecord);
        }
    }


    return true;
}



bool  
DtcpThreadTable::createThread ()
{
#ifdef _VERBOSE
    Log::Debug ("DtcpThreadTable::createThread invoked.");
#endif

    DtcpStackThread * newThread;
    bool status;

    newThread = new DtcpStackThread (udpTransport_);

    status = newThread->create ();

    if (!status) {
        Log::Error ("Create thread failed in DtcpThreadTable::createThread.");
        return false;
    }
    // Add thread to allThreadList for cleanup or search later.
    //
    {
        WriteLock  lock(threadLock_);

        allThreadList_.push_back (newThread);
    }

    // Add thread ID to thread object mapping in threadObjectIndex
    // for use in threadIdle method.
    //
    t_ThreadId  id;
    newThread->getThreadId (id);
    {
        WriteLock  lock(indexLock_);

        threadObjectIndex_.emplace(id,newThread);
    }

#ifdef _VERBOSE
    Log::Debug ("-DtcpStackThread created with ID: "s + threadIdToString (id));
#endif

    // start thread, it will then receive a pending buffer to process.
    //
    newThread->resume ();


    return true;
}



bool  
DtcpThreadTable::wakeThread ()
{
#ifdef _VERBOSE
    Log::Debug ("DtcpThreadTable::wakeThread invoked.");
#endif

    // attempt to start idle thread...
    //

    DtcpStackThread *  idleThread = nullptr;

    {
        WriteLock  lock(threadLock_);
    
        // If there is a thread in the thread list, wake it.
        // otherwise, someone else must have woken the thread
        // before we got to it...  ignore.
        //    
        if (!idleThreadList_.empty()) {
            idleThread = idleThreadList_.front ();
            idleThreadList_.pop_front ();
        }
    }

    if (idleThread) {
        idleThread->resume ();
        return true;
    }


    return true;
}



bool  
DtcpThreadTable::threadIdle (t_ThreadId  id)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpThreadTable::threadIdle invoked for thread ID: "s + threadIdToString (id));
#endif

    DtcpStackThread * threadObj = nullptr;

    // Add thread to idle list.
    //
    {
        ReadLock  lock(indexLock_);

        auto iter = threadObjectIndex_.find (id);

        if (iter == threadObjectIndex_.end ()) {
            // serious error, this should not occur
            Log::Error ("Unable to locate thread object in DtcpThreadTable::threadIdle!");

            return false;
        }
        threadObj = (*iter).second;

        if (!threadObj) {
            // serious error, this should not occur
            Log::Error ("NULL thread object indexed in DtcpThreadTable::threadIdle!");

            return false;
        }
    }

    {
        WriteLock  lock(threadLock_);
 
        idleThreadList_.push_back (threadObj);
    }


    return true;
}



