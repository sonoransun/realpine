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
#include <Platform.h>
#include <OptHash.h>


class AgedQueue;
class DataBlock;
class DtcpBaseUdpTransport;
class DtcpBaseConnTransport;


class DtcpPendingAckMap
{
  public:

    DtcpPendingAckMap (DtcpBaseUdpTransport *  udpTransport);

    ~DtcpPendingAckMap ();



    bool  add (DtcpBaseConnTransport * requestor,
               DataBlock *             data,
               ulong                   destIpAddress,
               ushort                  destPort,
               ulong &                 id);

    bool  remove (ulong  id);

    bool  processTimers ();



    // Types
    //
    using t_SysTime = struct timeval;

    struct t_PendingRecord {
        ulong                    id;
        DtcpBaseConnTransport *  requestor;
        DataBlock *              data;
        ulong                    destIpAddress;
        ushort                   destPort;
        t_SysTime                sendTime;
        ushort                   resendCount;
        AgedQueue *              currQueue;
    };

    using t_RecordIndex = std::unordered_map < ulong,
                       t_PendingRecord *,
                       OptHash<ulong>,
                       equal_to<ulong> >;

    using t_RecordIndexPair = std::pair <ulong, t_PendingRecord *>;
    
    
        
  private:

    DtcpBaseUdpTransport *  udpTransport_;

    t_RecordIndex * recordIndex_;
    ulong           currId_;
    t_SysTime       currTime_;

    AgedQueue *     lowQueue_;
    AgedQueue *     midQueue_;
    AgedQueue *     highQueue_;


    static ulong  msecTimeDiff (const t_SysTime &  beginTime,
                                const t_SysTime &  endTime);

};

