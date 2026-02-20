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


#include <AlpineGroup.h>
#include <AlpinePeerProfileIndex.h>
#include <AlpineQuery.h>
#include <Log.h>
#include <StringUtils.h>
#include <WriteLock.h>
#include <ReadLock.h>



AlpineGroup::AlpineGroup (ulong           groupId,
                          const string &  name,
                          const string &  description)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineGroup constructor invoked.");
#endif

    WriteLock  lock(dataLock_);

    groupId_       = groupId;
    name_          = name;
    description_   = description;
    profileIndex_  = new AlpinePeerProfileIndex ();
    queryList_     = new t_QueryList;
    numQueries_    = 0;
    numResponses_  = 0;
}



AlpineGroup::AlpineGroup (AlpineGroup *   copy,
                          ulong           groupId, 
                          const string &  name, 
                          const string &  description)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineGroup copy constructor invoked.");
#endif

    WriteLock  lock(dataLock_);

    groupId_       = groupId;
    name_          = name;
    description_   = description;
    profileIndex_  = new AlpinePeerProfileIndex ();
    queryList_     = new t_QueryList;
    numQueries_    = 0;
    numResponses_  = 0;

    // Copy peers from source group into our local profile index.
    //
    AlpinePeerProfileIndex::t_PeerIdList  idList;

    {
        ReadLock  copyLock(copy->dataLock_);
        copy->profileIndex_->getAllPeers (idList);
    }

    for (auto& item : idList) {
        profileIndex_->create (item);
    }
}



AlpineGroup::~AlpineGroup ()
{
#ifdef _VERBOSE
    Log::Debug ("AlpineGroup destructor invoked.");
#endif

    cancelAll ();

    WriteLock  lock(dataLock_);

    delete profileIndex_;

    delete queryList_;
}



bool  
AlpineGroup::getId (ulong &  id)
{
    ReadLock  lock(dataLock_);

    id = groupId_;

    return true;
}



bool  
AlpineGroup::getName (string &  name)
{
    ReadLock  lock(dataLock_);

    name = name_;

    return true;
}



bool  
AlpineGroup::getDescription (string & description)
{
    ReadLock  lock(dataLock_);

    description = description_;

    return true;
}



ulong  
AlpineGroup::size ()
{
    ReadLock  lock(dataLock_);

    ulong  size;
    size = profileIndex_->size ();

    return size;
}



ulong  
AlpineGroup::totalQueries ()
{
    ReadLock  lock(dataLock_);

    return numQueries_;
}



ulong  
AlpineGroup::totalResponses ()
{
    ReadLock  lock(dataLock_);

    return numResponses_;
}



bool  
AlpineGroup::addPeer (ulong peerId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineGroup::addPeer invoked.");
#endif


    return true;
}



bool  
AlpineGroup::removePeer (ulong  peerId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineGroup::removePeer invoked.");
#endif


    return true;
}



bool  
AlpineGroup::getPeerProfile (ulong                 peerId,
                             AlpinePeerProfile *&  profile)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineGroup::getPeerProfile invoked.");
#endif


    return true;
}



bool  
AlpineGroup::createPeerList (t_PeerIdList &  peerList)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineGroup::createPeerList invoked.");
#endif


    return true;
}



bool  
AlpineGroup::adjustPeerQuality (int  delta)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineGroup::adjustPeerQuality invoked.");
#endif


    return true;
}



bool  
AlpineGroup::queryStart (AlpineQuery *  query)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineGroup::queryStart invoked.");
#endif


    return true;
}



bool  
AlpineGroup::queryEnd (AlpineQuery *  query)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineGroup::queryEnd invoked.");
#endif


    return true;
}



void  
AlpineGroup::querySent (ulong  peerId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineGroup::queryEnd invoked.");
#endif
}



bool  
AlpineGroup::cancelAll ()
{
#ifdef _VERBOSE
    Log::Debug ("AlpineGroup::cancelAll invoked.");
#endif


    return true;
}



bool  
AlpineGroup::responseReceived (ulong  peerId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineGroup::responseReceived invoked.  Peer ID: "s +
                std::to_string (peerId));
#endif


    return true;
}



bool  
AlpineGroup::badPacketReceived (ulong  peerId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineGroup::badPacketReceived invoked.  Peer ID: "s +
                std::to_string (peerId));
#endif


    return true;
}



bool  
AlpineGroup::adjustQuality (ulong  peerId,
                            short  delta)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineGroup::adjustQuality invoked.  Peer ID: "s +
                std::to_string (peerId));
#endif


    return true;
}



