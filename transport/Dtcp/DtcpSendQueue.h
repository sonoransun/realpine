/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <list>
#include <Platform.h>


class DataBuffer;
class DtcpBroadcastStates;
class DtcpBaseUdpTransport;


class DtcpSendQueue
{
  public:

    DtcpSendQueue (DtcpBaseUdpTransport *  parentTransport,
                   ulong                   transferRate = 0);  // by default, do not use a rate

    ~DtcpSendQueue ();



    // Queue request for sending when available.  This
    // provides controlled use of outgoing bandwidth on
    // the UDP transport.
    //
    bool  addRequest (bool          priority,  // high priority packets sent first
                      const ulong   ipAddress,
                      const ushort  port,
                      DataBuffer *  dataBuffer);



    // Public types
    //
    struct t_UnicastRequest {
        DataBuffer *  dataBuffer;
        ulong         ipAddress;
        ushort        port;
    };

    using t_RequestList = list<t_UnicastRequest *>;

   
  private:

    DtcpBaseUdpTransport *   parentTransport_;
    ulong                    transferRate_;
    ulong                    byteDelayValue_;
    struct timeval           nextSendTime_;
    t_RequestList *          priorityRequestList_; // usually for reliable transfers
    t_RequestList *          normalRequestList_;
    DtcpBroadcastStates *    broadcastStates_;



    bool  setTransferRate (ulong  rate);

    bool  queueIdle ();

    bool  requestsPending (ulong & numRequests);

    bool  processEvents (struct timeval &  currentDelay); // for use in UDP poll

    bool  getBroadcastRequest (t_UnicastRequest *&  request);

    bool  locateBroadcastStates ();

    bool  calculateDelay (ulong             dataSize,
                          struct timeval &  delay);

    static bool  waitTimeRemaining (struct timeval &  endTime,
                                    struct timeval &  remaining);



    friend class DtcpBaseUdpTransport;
};

