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


#pragma once
#include <Common.h>
#include <AutoThread.h>
#include <ReadWriteSem.h>
#include <list>


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
    using t_DataBufferList = list <DataBuffer *>;

    using t_IORecordList = list <DtcpIORecord *>;

    using t_ThreadList = list <DtcpStackThread *>;

    // std::thread::id has built-in std::hash and operator==
    // specializations, so no custom hasher is needed.
    //
    using t_ThreadObjIndex = std::unordered_map <t_ThreadId, DtcpStackThread *>;

    using t_ThreadObjIndexPair = std::pair <t_ThreadId, DtcpStackThread *>;

    using t_ThreadIdIndex = std::unordered_map <t_ThreadId, DtcpIORecord *>;

    using t_ThreadIdIndexPair = std::pair <t_ThreadId, DtcpIORecord *>;


  private:

    DtcpBaseUdpTransport *  udpTransport_;

    ushort            maxThreads_;
    uint              maxRecords_;
    uint              maxBuffers_;

    t_IORecordList    allRecordList_;
    t_IORecordList    availableRecordList_;
    t_IORecordList    recordQueue_;
    ReadWriteSem      recordLock_;

    t_ThreadList      allThreadList_;
    t_ThreadList      idleThreadList_;
    ReadWriteSem      threadLock_;

    t_ThreadObjIndex  threadObjectIndex_;
    t_ThreadIdIndex   activeThreadIndex_;
    t_ThreadIdIndex   externalThreadIndex_;
    ReadWriteSem      indexLock_;

    t_DataBufferList  allBufferList_;
    t_DataBufferList  availableBufferList_;
    ReadWriteSem      bufferLock_;
    


    // called by DtcpStackThreads to get next
    // buffer if queued requests are pending.
    //
    bool  getNextRecord (t_ThreadId       threadId,
                         DtcpIORecord *&  record);

    bool  createThread ();

    bool  wakeThread ();

    bool  threadIdle (t_ThreadId  id);



    friend class  DtcpStackThread;
    friend class  DtcpBaseUdpTransport;



    DtcpThreadTable (const DtcpThreadTable & copy);
    DtcpThreadTable & operator = (const DtcpThreadTable & copy);

};

