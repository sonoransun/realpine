/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <AutoThread.h>
#include <ReadWriteSem.h>
#include <chrono>
#include <vector>


class DtcpBaseUdpTransport;
class DtcpStackThread;
class DtcpIORecord;
class DataBuffer;


class DtcpThreadTable
{
  public:

    DtcpThreadTable (DtcpBaseUdpTransport *  udpTransport,
                     ushort                  maxThreads = 25,
                     uint                    maxRecords = 100,
                     uint                    maxBuffers = 10000);

    ~DtcpThreadTable ();



    // obtain a free record for receive of data
    //
    bool  allocateRecord (DtcpIORecord *&  record);


    // Allocate a data buffer (allocateRecord is preferred)
    //
    bool  allocateBuffer (DataBuffer *&  buffer);

    bool  releaseBuffer (DataBuffer * buffer);


    // return a received data record for processing
    // by first available thread.
    //
    bool  processRecord (DtcpIORecord * record);


    // return processed record back to queue.
    //
    bool  processComplete (t_ThreadId  id);


    // return an allocated record to the free store.
    // Do not process.  (allocated, but not used)
    //
    void  returnRecord (DtcpIORecord * record);


    // locate record assigned to this thread
    //
    bool  locateRecord (t_ThreadId       threadId,
                        DtcpIORecord *&  record);



    // types
    //
    using t_Clock = std::chrono::steady_clock;
    using t_TimePoint = t_Clock::time_point;

    using t_ThreadList = list <DtcpStackThread *>;

    // std::thread::id has built-in std::hash and operator==
    // specializations, so no custom hasher is needed.
    //
    using t_ThreadObjIndex = std::unordered_map <t_ThreadId, DtcpStackThread *>;

    using t_ThreadObjIndexPair = std::pair <t_ThreadId, DtcpStackThread *>;

    using t_ThreadIdIndex = std::unordered_map <t_ThreadId, int>;

    using t_ThreadIdIndexPair = std::pair <t_ThreadId, int>;


  private:

    DtcpBaseUdpTransport *  udpTransport_;

    ushort            maxThreads_;
    uint              maxRecords_;
    uint              maxBuffers_;

    // Record pool: contiguous vector with index-based free list.
    // Each record tracks last-activity time for LRU eviction.
    //
    std::vector<DtcpIORecord *>  recordPool_;
    std::vector<int>             freeRecordIndices_;
    std::vector<t_TimePoint>     recordLastActivity_;
    std::vector<int>             recordQueue_;

    // Buffer pool: contiguous vector with index-based free list.
    //
    std::vector<DataBuffer *>    bufferPool_;
    std::vector<int>             freeBufferIndices_;

    // Consolidated lock: protects records, buffers, and index mappings.
    // Lock ordering: always threadLock_ before recordLock_.
    //
    ReadWriteSem      recordLock_;

    // Thread indices (merged from former indexLock_ into recordLock_)
    //
    t_ThreadObjIndex  threadObjectIndex_;
    t_ThreadIdIndex   activeThreadIndex_;     // maps thread ID -> record index
    t_ThreadIdIndex   externalThreadIndex_;   // maps thread ID -> record index

    // Thread lifecycle lock
    //
    t_ThreadList      allThreadList_;
    t_ThreadList      idleThreadList_;
    ReadWriteSem      threadLock_;



    // called by DtcpStackThreads to get next
    // buffer if queued requests are pending.
    //
    bool  getNextRecord (t_ThreadId       threadId,
                         DtcpIORecord *&  record);

    bool  createThread ();

    bool  wakeThread ();

    bool  threadIdle (t_ThreadId  id);

    // LRU eviction: find the idle record with oldest activity timestamp
    //
    int   evictLruRecord ();



    friend class  DtcpStackThread;
    friend class  DtcpBaseUdpTransport;



    DtcpThreadTable (const DtcpThreadTable & copy);
    DtcpThreadTable & operator = (const DtcpThreadTable & copy);

};

