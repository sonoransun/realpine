/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>


class AlpineQueryStatus
{
  public:

    AlpineQueryStatus () = default;

    AlpineQueryStatus (const AlpineQueryStatus & copy);

    ~AlpineQueryStatus () = default;

    const AlpineQueryStatus & operator = (const AlpineQueryStatus & copy);



    ulong  totalPackets ();

    ulong  numPacketsSent ();

    ulong  numRepliesReceived ();

    double percentComplete ();

    bool   isActive ();



  private:

    ulong        totalPackets_;
    ulong        packetsSent_;
    ulong        repliesReceived_;
    double       percentComplete_;
    bool         isActive_;

  
    void  setTotalPackets (ulong  totalPackets);
 
    void  setPacketsSent (ulong  numSent);

    void  setRepliesReceived (ulong  numReceived);

    void  setPercentComplete (const double &  percentage);

    void  setIsActive (bool  value);


    friend class AlpineQueryMgr;
    friend class AlpineQuery;
};


