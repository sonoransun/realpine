/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <ReadWriteSem.h>


class AlpinePeerProfileIndex;
class AlpinePeerProfile;
class AlpineQuery;


class AlpineGroup
{
  public:
    ~AlpineGroup();


    bool getId(ulong & id);

    bool getName(string & name);

    bool getDescription(string & description);

    ulong size();

    ulong totalQueries();

    ulong totalResponses();

    bool addPeer(ulong peerId);

    bool removePeer(ulong peerId);

    bool getPeerProfile(ulong peerId, AlpinePeerProfile *& profile);


    // Internal Types
    //
    using t_PeerIdList = vector<ulong>;

    using t_QueryList = list<AlpineQuery *>;


  private:
    ulong groupId_;
    string name_;
    string description_;
    AlpinePeerProfileIndex * profileIndex_;
    t_QueryList * queryList_;
    ulong numQueries_;
    ulong numResponses_;
    ReadWriteSem dataLock_;


    AlpineGroup(ulong groupId, const string & name, const string & description);

    AlpineGroup(AlpineGroup * copy, ulong groupId, const string & name, const string & description);


    bool createPeerList(t_PeerIdList & peerList);

    bool adjustPeerQuality(int delta);

    bool queryStart(AlpineQuery * query);

    bool queryEnd(AlpineQuery * query);

    void querySent(ulong peerId);

    bool cancelAll();

    // Profile operations specific to this group
    //
    bool responseReceived(ulong peerId);

    bool badPacketReceived(ulong peerId);

    bool adjustQuality(ulong peerId, short delta);


    // Copy constructor and assignment operator not implemented
    //
    AlpineGroup(const AlpineGroup & copy);
    AlpineGroup & operator=(const AlpineGroup & copy);


    friend class AlpineQuery;
    friend class AlpineGroupMgr;
};
