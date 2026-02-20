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
        
