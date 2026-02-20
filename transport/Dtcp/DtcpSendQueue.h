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

