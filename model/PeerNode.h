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


#ifndef __PeerNode_h__
#define __PeerNode_h__


#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <Message.h>


class Connection;
class NetManager;


class PeerNode
{
  public:

    PeerNode (unsigned long id, unsigned long bandwidth, int initialConnections);
    ~PeerNode ();


    unsigned long getId ();

    unsigned long getMaxBandwidth ();

    unsigned long getUsedBandwidth ();

    int getNumConnections ();


    bool createConnection (unsigned long peerId = 0);

    bool acceptConnection (Connection * endpoint);

    bool closeConnection (unsigned long peerId);

    bool sendMessage (const Message & msg);

    bool processMessage (const Message * msg);

    void status (string & statusMsg);


   
    using t_ConnectionList = vector<Connection *>;

    using t_ConnectionIndex = std::unordered_map< unsigned long,
                      Connection *,
                      std::hash<unsigned long>,
                      equal_to<unsigned long> >;

    using t_MessageSet = std::unordered_set< unsigned long,
                      std::hash<unsigned long>,
                      equal_to<unsigned long> >;
                      
    
  private:

    unsigned long      id_;
    unsigned long      maxBandwidth_;
    unsigned long      usedBandwidth_;
    
    t_ConnectionList   connectionList_;
    t_ConnectionIndex  connectionIndex_;
    t_MessageSet       recvMessages_;
    float              loadFactor_;
    int                lostConnections_;
    int                closedConnections_;
    int                messagesReceived_;
    


    friend class Connection;
    friend class NetManager;

    unsigned long remainingBandwidth ();

    bool consumeBandwidth (unsigned long amount);

    bool processMessage (Connection * activeConnection);

    bool completeIteration (float & loadFactor,
                            long  & dataLoss,
                            long  & msgLoss,
                            int   & connectionLoss,
                            int   & connectionTerminate,
                            int   & messagesReceived);
    

};


#endif
