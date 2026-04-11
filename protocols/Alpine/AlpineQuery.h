/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <AlpineQueryOptions.h>
#include <AlpineQueryStatus.h>
#include <Common.h>
#include <ReadWriteSem.h>


class AlpinePacket;
class AlpineQueryPacket;
class AlpineBroadcast;
class AlpineGroup;


class AlpineQuery
{
  public:
    ~AlpineQuery();


    using t_PeerIdList = vector<ulong>;


    bool startQuery();

    bool inProgress();

    bool getStatus(AlpineQueryStatus & queryStatus);

    bool getPeerIdList(t_PeerIdList & peerIdList);

    bool halt();

    bool resume();

    bool cancel();


  private:
    AlpineQueryOptions options_;
    bool queryActive_;
    AlpineGroup * group_;
    AlpineBroadcast * broadcast_;
    AlpinePacket * alpinePacket_;
    AlpineQueryPacket * queryPacket_;
    ReadWriteSem dataLock_;


    // Only the AlpineQueryMgr creates queries
    //
    AlpineQuery(AlpineQueryOptions & options);


    bool packetSent(ulong transportId);

    bool broadcastComplete(ulong numSent, struct timeval & duration);


    // Copy consturctor and assignement operator not implemented
    //
    AlpineQuery(const AlpineQuery & copy);
    AlpineQuery & operator=(const AlpineQuery & copy);


    friend class AlpineBroadcast;
    friend class AlpineQueryMgr;
};
