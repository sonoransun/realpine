/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <Configuration.h>
#include <DataBuffer.h>
#include <DtcpIORecord.h>
#include <DtcpStackThread.h>
#include <DtcpThreadTable.h>
#include <Log.h>
#include <ReadLock.h>
#include <StringUtils.h>
#include <WriteLock.h>
#include <algorithm>


static const int defaultBufferSize = 5 * 1024;  // 5K


DtcpThreadTable::DtcpThreadTable(DtcpBaseUdpTransport * udpTransport,
                                 ushort maxThreads,
                                 uint maxRecords,
                                 uint maxBuffers)
{
#ifdef _VERBOSE
    Log::Debug("DtcpThreadTable constructor invoked."s + "\nMaxThreads: "s + std::to_string(maxThreads) +
               "\nMaxRecords: "s + std::to_string(maxRecords) + "\nMaxBuffers: "s + std::to_string(maxBuffers) + "\n");
#endif

    WriteLock threadLock(threadLock_);
    WriteLock recordLock(recordLock_);

    udpTransport_ = udpTransport;
    maxThreads_ = maxThreads;
    maxBuffers_ = maxBuffers;

    // Check for configurable max pool size
    //
    string maxConnStr;
    if (Configuration::getValue("dtcp.maxConnections"s, maxConnStr)) {
        auto val = static_cast<uint>(std::stoul(maxConnStr));
        if (val > 0) {
            maxRecords = val;
        }
    }
    maxRecords_ = maxRecords;

    // Pre-allocate record pool with index-based free list
    //
    recordPool_.reserve(maxThreads);
    freeRecordIndices_.reserve(maxThreads);
    recordLastActivity_.reserve(maxThreads);

    for (uint i = 0; i < maxThreads; i++) {
        auto * newRecord = new DtcpIORecord(defaultBufferSize);
        recordPool_.push_back(newRecord);
        freeRecordIndices_.push_back(static_cast<int>(i));
        recordLastActivity_.push_back(t_Clock::now());
    }

    // Pre-allocate buffer pool with index-based free list
    //
    bufferPool_.reserve(maxThreads);
    freeBufferIndices_.reserve(maxThreads);

    for (uint i = 0; i < maxThreads; i++) {
        auto * newBuffer = new DataBuffer(defaultBufferSize);
        bufferPool_.push_back(newBuffer);
        freeBufferIndices_.push_back(static_cast<int>(i));
    }
}


DtcpThreadTable::~DtcpThreadTable()
{
#ifdef _VERBOSE
    Log::Debug("DtcpThreadTable destructor invoked.");
#endif

    // cleanup all allocated records
    //
    for (const auto & record : recordPool_) {
        delete record;
    }

    // cleanup all allocated buffers
    //
    for (const auto & buffer : bufferPool_) {
        delete buffer;
    }

    // cleanup all allocated threads
    //
    for (const auto & thread : allThreadList_) {
        delete thread;
    }
}


bool
DtcpThreadTable::allocateRecord(DtcpIORecord *& record)
{
#ifdef _VERBOSE
    Log::Debug("DtcpThreadTable::allocateRecord invoked.");
#endif

    record = nullptr;

    WriteLock lock(recordLock_);

    if (!freeRecordIndices_.empty()) {
        int idx = freeRecordIndices_.back();
        freeRecordIndices_.pop_back();
        record = recordPool_[idx];
        recordLastActivity_[idx] = t_Clock::now();

        return true;
    }

    if (recordPool_.size() < maxRecords_) {
        auto * newRecord = new DtcpIORecord(defaultBufferSize);
        int idx = static_cast<int>(recordPool_.size());
        recordPool_.push_back(newRecord);
        recordLastActivity_.push_back(t_Clock::now());
        record = newRecord;

        return true;
    }

    // Pool full — attempt LRU eviction of an idle record
    //
    int evicted = evictLruRecord();
    if (evicted >= 0) {
        record = recordPool_[evicted];
        recordLastActivity_[evicted] = t_Clock::now();
        return true;
    }

    // reached our limit, no idle records to evict
    //
    Log::Error("Maximum number of DtcpIORecords allocated; none available.");

    return false;
}


void
DtcpThreadTable::returnRecord(DtcpIORecord * record)
{
#ifdef _VERBOSE
    Log::Debug("DtcpThreadTable::returnRecord invoked.");
#endif

    if (!record) {
        Log::Error("Null record passed to DtcpThreadTable::returnRecord.  Ignoring.");
        return;
    }

    WriteLock lock(recordLock_);

    // Find index of record in pool
    //
    for (int i = 0; i < static_cast<int>(recordPool_.size()); i++) {
        if (recordPool_[i] == record) {
            freeRecordIndices_.push_back(i);
            recordLastActivity_[i] = t_Clock::now();
            return;
        }
    }

    Log::Error("Record not found in pool in DtcpThreadTable::returnRecord.");
}


bool
DtcpThreadTable::allocateBuffer(DataBuffer *& buffer)
{
#ifdef _VERBOSE
    Log::Debug("DtcpThreadTable::allocateBuffer invoked.");
#endif

    buffer = nullptr;

    WriteLock lock(recordLock_);

    if (!freeBufferIndices_.empty()) {
        int idx = freeBufferIndices_.back();
        freeBufferIndices_.pop_back();
        buffer = bufferPool_[idx];

        return true;
    }

    if (bufferPool_.size() < maxBuffers_) {
        auto * newBuffer = new DataBuffer(defaultBufferSize);
        bufferPool_.push_back(newBuffer);
        freeBufferIndices_.push_back(static_cast<int>(bufferPool_.size()) - 1);
        freeBufferIndices_.pop_back();
        buffer = newBuffer;

        return true;
    }

    // reached our limit!
    //
    Log::Error("Maximum number of DataBuffers allocated; none available.");

    return false;
}


bool
DtcpThreadTable::releaseBuffer(DataBuffer * buffer)
{
#ifdef _VERBOSE
    Log::Debug("DtcpThreadTable::releaseBuffer invoked.");
#endif

    if (!buffer) {
        Log::Error("Null buffer passed to DtcpThreadTable::releaseBuffer!");
        return false;
    }

    WriteLock lock(recordLock_);

    for (int i = 0; i < static_cast<int>(bufferPool_.size()); i++) {
        if (bufferPool_[i] == buffer) {
            freeBufferIndices_.push_back(i);
            return true;
        }
    }

    Log::Error("Buffer not found in pool in DtcpThreadTable::releaseBuffer.");
    return false;
}


bool
DtcpThreadTable::processRecord(DtcpIORecord * record)
{
#ifdef _VERBOSE
    Log::Debug("DtcpThreadTable::processRecord invoked.");
#endif

    if (!record) {
        Log::Error("Null record passed to DtcpThreadTable::processRecord.  Ignoring.");
        return false;
    }

    // Find record index and enqueue
    //
    {
        WriteLock lock(recordLock_);

        int recIdx = -1;
        for (int i = 0; i < static_cast<int>(recordPool_.size()); i++) {
            if (recordPool_[i] == record) {
                recIdx = i;
                break;
            }
        }
        if (recIdx >= 0) {
            recordLastActivity_[recIdx] = t_Clock::now();
        }
        recordQueue_.push_back(recIdx);
    }


    // If an idle thread is available to process this request,
    // wake it up...
    //
    bool resumeThread = false;
    {
        ReadLock lock(threadLock_);

        if (!idleThreadList_.empty()) {
            resumeThread = true;
        }
    }

    if (resumeThread) {
        wakeThread();
        return true;
    }
    // all threads busy, see if we can create one...
    //
    bool newThread = false;
    {
        ReadLock lock(threadLock_);

        if (allThreadList_.size() < maxThreads_) {
            newThread = true;
        }
    }

    if (newThread) {
        createThread();
        return true;
    }
    // All threads are busy, no more allowed.  Once a thread
    // becomes available it will process the pending records.


    return true;
}


bool
DtcpThreadTable::processComplete(t_ThreadId id)
{
#ifdef _VERBOSE
    Log::Debug("DtcpThreadTable::processComplete invoked for thread ID: "s + threadIdToString(id));
#endif

    // Remove thread / record from active index and add to available queue.
    // recordLock_ protects both activeThreadIndex_ and freeRecordIndices_.
    //
    {
        WriteLock lock(recordLock_);

        auto iter = activeThreadIndex_.find(id);

        if (iter == activeThreadIndex_.end()) {
            Log::Error("Unable to locate active thread record "
                       "in DtcpThreadTable::processComplete!");
            return false;
        }

        int recIdx = iter->second;
        activeThreadIndex_.erase(id);

        freeRecordIndices_.push_back(recIdx);
        recordLastActivity_[recIdx] = t_Clock::now();
    }

    return true;
}


bool
DtcpThreadTable::locateRecord(t_ThreadId threadId, DtcpIORecord *& record)
{
#ifdef _VERBOSE
    Log::Debug("DtcpThreadTable::locateRecord invoked.");
#endif

    record = nullptr;

    // scope lock — recordLock_ protects all index maps
    {
        ReadLock lock(recordLock_);

        auto iter = activeThreadIndex_.find(threadId);

        if (iter != activeThreadIndex_.end()) {
            record = recordPool_[iter->second];
        }

        // if we did not find the record in the active thread index,
        // check the external index
        //
        if (!record) {
            iter = externalThreadIndex_.find(threadId);

            if (iter != externalThreadIndex_.end()) {
                record = recordPool_[iter->second];
            }
        }
    }

    if (record) {
        return true;
    }

    // at this point we need to associate an available DtcpIORecord with
    // this external thread.
    //
    bool status;
    status = allocateRecord(record);

    if (!status) {
        Log::Error("Unable to allocate DtcpIORecord for this thread.");
        return false;
    }

#ifdef _VERBOSE
    Log::Debug("Allocated new record for this external thread.");
#endif

    // Find index of the allocated record and add to external index
    //
    WriteLock lock(recordLock_);

    for (int i = 0; i < static_cast<int>(recordPool_.size()); i++) {
        if (recordPool_[i] == record) {
            externalThreadIndex_.emplace(threadId, i);
            break;
        }
    }

    return true;
}


bool
DtcpThreadTable::getNextRecord(t_ThreadId threadId, DtcpIORecord *& record)
{
#ifdef _VERBOSE
    Log::Debug("DtcpThreadTable::getNextRecord invoked for thread ID: "s + threadIdToString(threadId));
#endif

    // If there is anything in the queue, pull it now.
    // otherwise, return and let thread go to idle.
    //
    record = nullptr;
    int recIdx = -1;
    int oldRecIdx = -1;

    {
        WriteLock lock(recordLock_);

        if (!recordQueue_.empty()) {
            recIdx = recordQueue_.front();
            recordQueue_.erase(recordQueue_.begin());
        }
    }

    if (recIdx < 0) {
        return false;
    }

    // perform required setup to associate this record with this thread,
    // hand back to calling thread to process.
    //
    {
        WriteLock lock(recordLock_);

        auto iter = activeThreadIndex_.find(threadId);

        if (iter != activeThreadIndex_.end()) {
            oldRecIdx = iter->second;
            activeThreadIndex_.erase(threadId);
        }

        // index new record with this thread.
        //
        activeThreadIndex_.emplace(threadId, recIdx);
        recordLastActivity_[recIdx] = t_Clock::now();
    }

    record = recordPool_[recIdx];

    if (oldRecIdx >= 0) {
        // add old record back on available list.
        //
        WriteLock lock(recordLock_);
        freeRecordIndices_.push_back(oldRecIdx);
        recordLastActivity_[oldRecIdx] = t_Clock::now();
    }

    return true;
}


bool
DtcpThreadTable::createThread()
{
#ifdef _VERBOSE
    Log::Debug("DtcpThreadTable::createThread invoked.");
#endif

    auto * newThread = new DtcpStackThread(udpTransport_);

    bool status = newThread->create();

    if (!status) {
        Log::Error("Create thread failed in DtcpThreadTable::createThread.");
        delete newThread;
        return false;
    }

    // Add thread to allThreadList for cleanup or search later.
    // Lock ordering: threadLock_ before recordLock_
    //
    {
        WriteLock lock(threadLock_);
        allThreadList_.push_back(newThread);
    }

    // Add thread ID to thread object mapping in threadObjectIndex
    //
    t_ThreadId id;
    newThread->getThreadId(id);
    {
        WriteLock lock(recordLock_);
        threadObjectIndex_.emplace(id, newThread);
    }

#ifdef _VERBOSE
    Log::Debug("-DtcpStackThread created with ID: "s + threadIdToString(id));
#endif

    // start thread, it will then receive a pending buffer to process.
    //
    newThread->resume();

    return true;
}


bool
DtcpThreadTable::wakeThread()
{
#ifdef _VERBOSE
    Log::Debug("DtcpThreadTable::wakeThread invoked.");
#endif

    // attempt to start idle thread...
    //
    DtcpStackThread * idleThread = nullptr;

    {
        WriteLock lock(threadLock_);

        if (!idleThreadList_.empty()) {
            idleThread = idleThreadList_.front();
            idleThreadList_.pop_front();
        }
    }

    if (idleThread) {
        idleThread->resume();
    }

    return true;
}


bool
DtcpThreadTable::threadIdle(t_ThreadId id)
{
#ifdef _VERBOSE
    Log::Debug("DtcpThreadTable::threadIdle invoked for thread ID: "s + threadIdToString(id));
#endif

    DtcpStackThread * threadObj = nullptr;

    // Lookup thread object — threadObjectIndex_ is under recordLock_
    //
    {
        ReadLock lock(recordLock_);

        auto iter = threadObjectIndex_.find(id);

        if (iter == threadObjectIndex_.end()) {
            Log::Error("Unable to locate thread object in DtcpThreadTable::threadIdle!");
            return false;
        }
        threadObj = iter->second;

        if (!threadObj) {
            Log::Error("NULL thread object indexed in DtcpThreadTable::threadIdle!");
            return false;
        }
    }

    // Add thread to idle list
    //
    {
        WriteLock lock(threadLock_);
        idleThreadList_.push_back(threadObj);
    }

    return true;
}


int
DtcpThreadTable::evictLruRecord()
{
    // Caller must hold recordLock_ in write mode.
    // Find the free record index with the oldest last-activity timestamp.
    // If no free records exist, scan for an idle record not in active use.
    //

    if (!freeRecordIndices_.empty()) {
        // Find the LRU among free records
        //
        auto oldestTime = t_TimePoint::max();
        int oldestPos = -1;

        for (int i = 0; i < static_cast<int>(freeRecordIndices_.size()); i++) {
            int idx = freeRecordIndices_[i];
            if (recordLastActivity_[idx] < oldestTime) {
                oldestTime = recordLastActivity_[idx];
                oldestPos = i;
            }
        }

        if (oldestPos >= 0) {
            int idx = freeRecordIndices_[oldestPos];
            // Swap-and-pop for O(1) removal
            freeRecordIndices_[oldestPos] = freeRecordIndices_.back();
            freeRecordIndices_.pop_back();
            return idx;
        }
    }

    return -1;
}
