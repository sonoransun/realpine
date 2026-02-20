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


#include <PeerNode.h>
#include <Connection.h>
#include <NetManager.h>
#include <StringUtils.h>
#include <Log.h>
#include <algorithm>



PeerNode::PeerNode (unsigned long id, unsigned long bandwidth, int initialConnections)
  : id_(id), maxBandwidth_(bandwidth), usedBandwidth_(0)
{
#ifdef VERBOSE
    Log::Debug ("PeerNode created."s +
                "\nNode ID ....: "s + std::to_string(id) +
                "\nBandwidth ..: "s + std::to_string(bandwidth) +
                "\nInit Conns .: "s + std::to_string(initialConnections) +
                "\n");
#endif

    connectionList_.clear();
    connectionIndex_.clear();
    recvMessages_.clear();

    loadFactor_ = 0.0;
    lostConnections_ = 0;
    closedConnections_ = 0;
    messagesReceived_ = 0;

    if (initialConnections > NetManager::numClients()) {
        initialConnections = NetManager::numClients();
    }

    int i;
    for (i = 0; i < initialConnections; i++) {
        createConnection (0);
    }
}

    
PeerNode::~PeerNode ()
{
#ifdef VERBOSE
    Log::Debug ("PeerNode::~PeerNode invoked.");
#endif

    connectionList_.clear();
    connectionIndex_.clear();
    recvMessages_.clear();
}


unsigned long 
PeerNode::getId ()
{
    return id_;
}


unsigned long 
PeerNode::getMaxBandwidth ()
{
    return maxBandwidth_;
}


unsigned long 
PeerNode::getUsedBandwidth ()
{
    return usedBandwidth_;
}


int 
PeerNode::getNumConnections ()
{
    return connectionList_.size();
}


bool 
PeerNode::createConnection (unsigned long peerId)
{
#ifdef VERBOSE
    Log::Debug ("PeerNode::createConnection invoked.  My peer ID: "s +
                std::to_string(id_) + "  target ID: " + std::to_string(peerId));
#endif

    PeerNode *  destNode;

    if (peerId == 0) {
        // select a random peer
        if (!NetManager::selectPeer (id_, destNode)) {
            Log::Error ("Unable to locate a peer for "s + std::to_string(id_));
            return false;

        }
        peerId = destNode->getId ();
#ifdef VERBOSE
        Log::Debug ("Random peer is: "s + std::to_string(peerId));
#endif
    }
    else {
        if (!NetManager::locatePeer (peerId, destNode)) {
            Log::Error ("Unable to locate peer "s + std::to_string(peerId));
            return false;
    }

    // make sure this isnt a duplicate
    if (connectionIndex_.find (peerId) != connectionIndex_.end ()) {
        Log::Error ("Duplicate peer selected.  Peer ID: "s + std::to_string(peerId));
        return false;
    }
    Connection *  newConnection;
    newConnection = new Connection (this);

    if (!destNode->acceptConnection (newConnection)) {
        Log::Debug ("Connection refused from Peer "s + std::to_string(peerId));
        delete newConnection;
        return false;
    }
    // Add this new connection to our containers
    connectionList_.push_back (newConnection);
    connectionIndex_.insert ( pair<unsigned long, Connection *>(peerId, newConnection) );

    return true;
}


bool 
PeerNode::acceptConnection (Connection * endpoint)
{
#ifdef VERBOSE
    Log::Debug ("PeerNode::acceptConnection invoked.  Requesting node: "s +
                std::to_string(endpoint->getNodeId ()) );
#endif

    unsigned long  peerId;
    Connection *   newConnection;

    newConnection = new Connection (this);
    peerId = endpoint->getNodeId ();

    // tie these connections together...
    //
    if (!endpoint->tie (newConnection)) {
        Log::Error ("Tie failed in PeerNode::acceptConnection!");
        return false;
    }
    // Add this new connection to our containers
    connectionList_.push_back (newConnection);
    connectionIndex_.insert ( pair<unsigned long, Connection *>(peerId, newConnection) );

    return true;
}


bool 
PeerNode::closeConnection (unsigned long peerId)
{
#ifdef VERBOSE
    Log::Debug ("PeerNode::closeConnection invoked.  Target node: "s +
                std::to_string(peerId));
#endif

    // If we closed this connection, dont do anything...
    Connection *  peerConnection;
    auto match = connectionIndex_.find (peerId);

    if (match == connectionIndex_.end ()) {
        // We closed this connection, ignore...
        return true;
    }
    // remove from map and delete...
    peerConnection = (*match).second;
    connectionIndex_.erase (peerId);
    connectionList_.erase (std::remove(connectionList_.begin(), connectionList_.end(), peerConnection), connectionList_.end());

    delete peerConnection;

    return true;
}


bool 
PeerNode::sendMessage (const Message & msg)
{
#ifdef VERBOSE
    Log::Debug ("PeerNode::sendMessage invoked for message ID: "s + std::to_string(msg.getId()) );
#endif

    // For each node we are connected to, forward message...
    //
    for (auto& item : connectionList_) {
        item->sendMsg (&msg);
    }

    return true;
}


void 
PeerNode::status (string & statusMsg)
{
    statusMsg = "OK";
}

   
unsigned long 
PeerNode::remainingBandwidth ()
{
    return maxBandwidth_ - usedBandwidth_;
}


bool 
PeerNode::consumeBandwidth (unsigned long amount)
{
#ifdef VERBOSE
    Log::Debug ("PeerNode::consumeBandwidth invoked for amount: "s + std::to_string(amount));
#endif

    if ( (usedBandwidth_ + amount) > maxBandwidth_) {
        return false;
    }
    usedBandwidth_ += amount;

    return true;
}


bool 
PeerNode::processMessage (const Message * msg)
{
#ifdef VERBOSE
    Log::Debug ("PeerNode::processMessage invoked for message ID: "s + std::to_string(msg->getId()) );
#endif

    messagesReceived_++;

    if (msg->getTtl () > 0) {

        // Forward on...
        Message * myCopy = new Message (*msg);
        myCopy->decTtl ();

        sendMessage (*myCopy);
        delete myCopy;
    }
    else {
#ifdef VERBOSE
        Log::Debug ("Dropped message, ttl expired.");
#endif
        return;
    }

    return true;
}


bool 
PeerNode::completeIteration (float & loadFactor,
                             long  & dataLoss,
                             long  & msgLoss,
                             int   & connectionLoss,
                             int   & connectionTerminate,
                             int   & messagesReceived)
{
#ifdef VERBOSE
    Log::Debug ("PeerNode::completeIteration invoked.");
#endif

    float bandMax = maxBandwidth_;
    float bandUsed = usedBandwidth_;

    loadFactor = loadFactor_ = (bandUsed / bandMax) * 100;
    connectionLoss = lostConnections_;
    connectionTerminate = closedConnections_;
    messagesReceived = messagesReceived_;

    dataLoss = 0;
    msgLoss = 0;
    for (auto& item : connectionList_) {
        dataLoss += item->dataLoss ();
        msgLoss += item->msgLoss ();
        item->reset ();
    }

    // reset
    usedBandwidth_ = 0;
    lostConnections_ = 0;
    closedConnections_ = 0;
    messagesReceived_ = 0;

    return true;
}
    

