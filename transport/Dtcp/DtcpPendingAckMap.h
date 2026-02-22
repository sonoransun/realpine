/// Copyright (C) 2026 sonoransun — see LICENCE.txt


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

