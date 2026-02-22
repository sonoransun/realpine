/// Copyright (C) 2026 sonoransun — see LICENCE.txt


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

