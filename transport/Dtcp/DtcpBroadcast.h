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
#include <ReadWriteSem.h>


class DtcpBroadcastSet;
class StackLinkInterface;
class DtcpBroadcastMgr;


class DtcpBroadcast
{
  public:

    DtcpBroadcast (DtcpBroadcastSet *  destinations);

    virtual ~DtcpBroadcast ();



    // packet must be a DtcpConnPacket or equivalent
    // MRP_TEMP refine requirements / verify
    //
    bool  sendPacket (StackLinkInterface * packet);

    bool  sending ();

    bool  numDestinations (ulong &  destinationCount);

    bool  packetsSent (ulong &  numSent);

    bool  percentComplete (double &  percentage);

    virtual bool  pause ();

    virtual bool  resume ();

    virtual bool  cancel ();

    bool  packetSendNotifications (bool  used);  // true == perform send notifications

    virtual bool  handlePacketSend (ulong  transportId) = 0;

    virtual bool  handleSendComplete (ulong             numSent,
                                      struct timeval &  duration) = 0;


  private:

    bool                  active_;
    DtcpBroadcastSet *    destinations_;
    ulong                 requestId_;
    struct timeval        startTime_;
    bool                  packetNotification_;
    ReadWriteSem          dataLock_;


    virtual bool  broadcastComplete (ulong  requestId,
                                     ulong  totalSent);


    friend class  DtcpBroadcastMgr;
};

