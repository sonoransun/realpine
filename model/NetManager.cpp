/// Copyright (C) 2026 sonoransun — see LICENCE.txt




#include <NetManager.h>
#include <PeerNode.h>
#include <Log.h>
#include <StringUtils.h>


NetManager::t_NodeList *        NetManager::nodeList_s = 0;
NetManager::t_NodeIndex *       NetManager::nodeIndex_s = 0;
unsigned long                   NetManager::currClientId_s = 1;
unsigned long                   NetManager::currMessageId_s = 1;
unsigned long                   NetManager::nodeCount_s = 0;
unsigned long                   NetManager::cycleMessageCount_s = 0;
unsigned long                   NetManager::cycleDataSize_s = 0;
unsigned long                   NetManager::connectionCount_s = 0;
struct timeval                  NetManager::cycleProcessTime_s;


NetManager::NetManager ()
{
}


NetManager::~NetManager ()
{
}


bool 
NetManager::initialize ()
{
    Log::Debug ("NetManager::initialize invoked.");

    if (nodeList_s) {
        // no reinitialize
        return false;
    }
    nodeList_s = new t_NodeList;
    nodeIndex_s = new t_NodeIndex;

    return true;
}


int  
NetManager::numClients ()
{
    if (nodeList_s) {
        return nodeList_s->size();
    else {
        return 0;
}


bool 
NetManager::createClients (int   numToCreate,
                           long  bandwidth)
{
    Log::Debug ("NetManager::createClients invoked.  Creating "s +
                std::to_string(numToCreate) + " clients on network.");

    nodeIndex_s->resize (nodeIndex_s->size() + 2 * numToCreate);

    PeerNode *  newNode;
    int i;

    for (i = 0; i < numToCreate; i++) {
        newNode = new PeerNode( currClientId_s, bandwidth, 3);
        nodeList_s->push_back (newNode);
        nodeIndex_s->insert ( pair<unsigned long, PeerNode *>(currClientId_s, newNode) );
        currClientId_s++;
    }

    return true;
}    


bool 
NetManager::removeClients (int numToRemove)
{
    Log::Debug ("NetManager::removeClients invoked.  Removing "s +
                std::to_string(numToRemove) + " clients from network.");

    return true;
}


bool 
NetManager::sendMessages (int numToSend,
                          int size,
                          int ttl)
{
    Log::Debug ("NetManager::sendMessages invoked.  Paramters:"s +
                "\nNum to Send .: "s + std::to_string(numToSend) +
                "\nSize ........: "s + std::to_string(size) +
                "\nTTL .........: "s + std::to_string(ttl) +
                "\n");

    int i;
    Message * newMessage;
    int  clientNumber;
    PeerNode * sendingNode;

    for (i = 0; i < numToSend; i++) {

        clientNumber = (int) (nodeList_s->size() * rand()/(RAND_MAX+1.0));

        auto iter = nodeList_s->begin();
        int j;
        for (j = 0; j < clientNumber; j++) {
            iter++;
        }

        sendingNode = (*iter);
        newMessage = new Message (size, ttl, currMessageId_s++);

        Log::Debug ("Sending message: "s + std::to_string(i));
        sendingNode->sendMessage (*newMessage);

        delete newMessage;
    }
 
    Log::Debug ("All messages sent.  Returning.");
       
    return true;
}


bool 
NetManager::selectPeer (unsigned long peerId,
                        PeerNode *&   peer)
{
    if ( (!nodeList_s) || (nodeList_s->size () == 0) ) {
        return false;
    }
    if (nodeList_s->size () <= 1) {
        // Only one node, cannot select.
        return false;
    }
    static unsigned int lastIndex = 0;

    if (lastIndex >= nodeList_s->size ()) {
        lastIndex = 0;
    }

    auto iter = nodeList_s->begin();
    unsigned int i;
    for (i = 0; i < lastIndex; i++) {
        iter++;
    }
    if ((*iter)->getId() == peerId) {
        iter++;
        if (iter == nodeList_s->end()) {
            iter = nodeList_s->begin();
        }
    }
    
    peer = (*iter);
    lastIndex++;

    return true;
}


bool 
NetManager::locatePeer (unsigned long peerId,
                        PeerNode *&   peer)
{
    auto match = nodeIndex_s->find (peerId);

    if (match == nodeIndex_s->end()) {
        return false;
    }
    peer = (*match).second;

    return true;
}


bool 
NetManager::processCycle ()
{
    Log::Debug ("NetManager::processCycle invoked.");

    // Status variables
    //
    unsigned long  id;
    unsigned long  maxBandwidth;
    unsigned long  usedBandwidth;
    int   numConnections;
    int   messagesReceived;
    float loadFactor;
    long  dataLoss;
    long  msgLoss;
    int   connectionLoss;
    int   connectionTerminate;

    for (auto& node : *nodeList_s) {

        // Gather stats for each node...
        //
        id = node->getId ();
        maxBandwidth = node->getMaxBandwidth ();
        usedBandwidth = node->getUsedBandwidth ();
        numConnections = node->getNumConnections ();


        node->completeIteration (loadFactor,
                                    dataLoss,
                                    msgLoss,
                                    connectionLoss,
                                    connectionTerminate,
                                    messagesReceived);

        Log::Debug ("Status for client ID: "s + std::to_string(id) +
                    "\nNum Peers ......: "s + std::to_string(numConnections) +
                    "\nMax Bandwidth ..: "s + std::to_string(maxBandwidth) +
                    "\nUsed Bandwidth .: "s + std::to_string(usedBandwidth) +
                    "\nLoad Factor ....: "s + std::to_string(loadFactor) +
                    "\nData Loss ......: "s + std::to_string(dataLoss) +
                    "\nMsg Loss .......: "s + std::to_string(msgLoss) +
                    "\nConn. Loss .....: "s + std::to_string(connectionLoss) +
                    "\nConn. Close ....: "s + std::to_string(connectionTerminate) +
                    "\nMsgs Received ..: "s + std::to_string(messagesReceived) +
                    "\n");
    }

    return true;
}


bool 
NetManager::status (string & statusMsg)
{
    statusMsg = "Num Peer Nodes: "s + std::to_string(nodeList_s->size());

    return true;
}




        
