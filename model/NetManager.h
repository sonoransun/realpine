/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#ifndef __NetManager_h__
#define __NetManager_h__


#include <sys/time.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>


class PeerNode;


class NetManager
{
  public:

    NetManager ();
    ~NetManager ();


    static bool initialize (); 

    static bool createClients (int  numToCreate,
                               long bandwidth);

    static bool sendMessages (int numToSend,
                              int size,
                              int ttl);

    static int  numClients ();

    static bool removeClients (int numToRemove);

    static bool processCycle ();

    static bool status (string & statusMsg);

    static bool selectPeer (unsigned long peerId,
                            PeerNode *&   peer);

    static bool locatePeer (unsigned long peerId,
                            PeerNode *&   peer);



    using t_NodeList = vector<PeerNode *>;

    using t_NodeIndex = std::unordered_map< unsigned long,
                      PeerNode *,
                      std::hash<unsigned long>,
                      equal_to<unsigned long> >;


  private:

    static t_NodeList *    nodeList_s;
    static t_NodeIndex *   nodeIndex_s;

    static unsigned long   currClientId_s;
    static unsigned long   currMessageId_s;
    static unsigned long   nodeCount_s;
    static unsigned long   cycleMessageCount_s;
    static unsigned long   cycleDataSize_s;
    static unsigned long   connectionCount_s;
    static struct timeval  cycleProcessTime_s;

};

#endif
        
