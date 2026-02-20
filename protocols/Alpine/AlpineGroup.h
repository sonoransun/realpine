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
#include <ReadWriteSem.h>


class AlpinePeerProfileIndex;
class AlpinePeerProfile;
class AlpineQuery;


class AlpineGroup
{
  public:

    ~AlpineGroup ();



    bool  getId (ulong &  id);

    bool  getName (string &  name);

    bool  getDescription (string & description);

    ulong  size ();

    ulong  totalQueries ();

    ulong  totalResponses ();

    bool  addPeer (ulong peerId);

    bool  removePeer (ulong  peerId);

    bool  getPeerProfile (ulong                 peerId,
                          AlpinePeerProfile *&  profile);



    // Internal Types
    //
    using t_PeerIdList = vector<ulong>;

    using t_QueryList = list<AlpineQuery *>;


  private:

    ulong                     groupId_;
    string                    name_;
    string                    description_;
    AlpinePeerProfileIndex *  profileIndex_;
    t_QueryList *             queryList_;
    ulong                     numQueries_;
    ulong                     numResponses_;
    ReadWriteSem              dataLock_;


    AlpineGroup (ulong           groupId,
                 const string &  name,
                 const string &  description);

    AlpineGroup (AlpineGroup *   copy,
                 ulong           groupId, 
                 const string &  name, 
                 const string &  description);


    bool  createPeerList (t_PeerIdList &  peerList);

    bool  adjustPeerQuality (int  delta);

    bool  queryStart (AlpineQuery *  query);

    bool  queryEnd (AlpineQuery *  query);

    void  querySent (ulong  peerId);

    bool  cancelAll ();

    // Profile operations specific to this group
    //
    bool  responseReceived (ulong  peerId);

    bool  badPacketReceived (ulong  peerId);

    bool  adjustQuality (ulong  peerId,
                         short  delta);


    // Copy constructor and assignment operator not implemented
    //
    AlpineGroup (const AlpineGroup & copy);
    AlpineGroup & operator = (const AlpineGroup & copy);


    friend class AlpineQuery;
    friend class AlpineGroupMgr;
};


