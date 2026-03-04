/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AlpinePeerProfileIndex.h>
#include <AlpinePeerProfile.h>
#include <AlpineRatingEngine.h>
#include <Log.h>
#include <StringUtils.h>
#include <ReadLock.h>
#include <WriteLock.h>
#include <algorithm>
#include <cmath>
#include <queue>



static const long  linkEnd = -1L;
static const long  minReserve = 128;
static const long  defaultReserve = 1024;



AlpinePeerProfileIndex::AlpinePeerProfileIndex (long  reserve)
{
#ifdef _VERBOSE
    Log::Debug ("AlpinePeerProfileIndex constructor invoked.  Reserve: "s +
                std::to_string (reserve));
#endif

    WriteLock   lock(dataLock_);

    if (reserve < minReserve) {  // invalid value
        reserve = defaultReserve;
    }

    size_                 = reserve;
    numRecords_           = 0;
    highestQualityIndex_  = linkEnd;
    lowestQualityIndex_   = linkEnd;
    baseIndex_            = linkEnd;


    // Allocate and initialize profile containers
    //
    profileArray_   = nullptr;
    freeIndexList_  = nullptr;
    profileIndex_   = nullptr;

    profileArray_ = new t_ProfileRecordArray;
    profileArray_->clear ();
    profileArray_->resize (size_);

    freeIndexList_ = new t_FreeIndexList;
    freeIndexList_->clear ();

    profileIndex_ = new t_ProfileIndex;
    profileIndex_->clear ();


    long  i;
    for (i = 0; i < size_; i++) {
        freeIndexList_->push_back (i);
        (*profileArray_)[i].profile = nullptr;
        (*profileArray_)[i].next = linkEnd;
        (*profileArray_)[i].prev = linkEnd;
    }
}



AlpinePeerProfileIndex::~AlpinePeerProfileIndex ()
{
#ifdef _VERBOSE
    Log::Debug ("AlpinePeerProfileIndex destructor invoked.");
#endif

    delete profileArray_;

    delete freeIndexList_;

    delete profileIndex_;
}



bool  
AlpinePeerProfileIndex::create (ulong  peerId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpinePeerProfileIndex::create invoked.  Peer ID: "s +
                std::to_string (peerId));
#endif

    // Make sure we dont aready have this peer ID
    //
    {
        ReadLock  lock(dataLock_);

        if (profileIndex_->find (peerId) != profileIndex_->end ()) {
            Log::Error ("Attempt to create duplicate profile for peer in "
                                 "AlpinePeerProfileIndex::create!");

            return false;
        }
    }

    WriteLock   lock(dataLock_);

    // Resize if needed...
    //
    if (numRecords_ >= size_) {
        resize (size_ * 2);
    }

    long currIndex = freeIndexList_->front ();
    freeIndexList_->pop_front ();

    t_ProfileRecord * newRecord;
    newRecord = &( (*profileArray_)[currIndex] );

    newRecord->profile = new AlpinePeerProfile (peerId);


    // Add to ID index
    //
    profileIndex_->emplace (peerId, currIndex);

    if (numRecords_ == 0) {

        // First profile, initialize some pointers
        //
        highestQualityIndex_ = currIndex;
        lowestQualityIndex_  = currIndex;
        baseIndex_           = currIndex;

        newRecord->next = linkEnd;
        newRecord->prev = linkEnd;

        numRecords_++;

        return true;
    }
    // Not first profile, update links as required
    //
    t_ProfileRecord *  baseRecord;
    short              quality;

    baseRecord = &( (*profileArray_)[baseIndex_] );
    baseRecord->profile->getRelativeQuality (quality);

    // We want to place this record at the base quality level, so if our current base
    // has been reduced, we place it before, otherwise after.
    //
    if (quality < 0) {
        newRecord->prev = baseIndex_;
        newRecord->next = baseRecord->next;

        if (baseRecord->next != linkEnd) {
            t_ProfileRecord *  nextRecord;
            nextRecord = &( (*profileArray_)[baseRecord->next] );
            nextRecord->prev = currIndex;
        }
        
        baseRecord->next = currIndex;

        if (highestQualityIndex_ == baseIndex_) {
            highestQualityIndex_ = currIndex;
        }
    }
    else {
        newRecord->next = baseIndex_;
        newRecord->prev = baseRecord->prev;

        if (baseRecord->prev != linkEnd) {
            t_ProfileRecord *  prevRecord;
            prevRecord = &( (*profileArray_)[baseRecord->prev] );
            prevRecord->next = currIndex;
        }

        baseRecord->prev = currIndex;

        if (lowestQualityIndex_ == baseIndex_) {
            lowestQualityIndex_ = currIndex;
        }
    }

    baseIndex_ = currIndex;
    numRecords_++;


    return true;
}



bool
AlpinePeerProfileIndex::erase (ulong  peerId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpinePeerProfileIndex::erase invoked.  Peer ID: "s +
                std::to_string (peerId));
#endif

    if (numRecords_ == 0) {
        return false;
    }
    WriteLock   lock(dataLock_);

    auto indexIter = profileIndex_->find (peerId);

    if (indexIter == profileIndex_->end ()) {
        Log::Error ("Attempt to erase non existant peer ID profile in "
                             "AlpinePeerProfileIndex::erase!");

        return false;
    }
    long currIndex = (*indexIter).second;

    profileIndex_->erase (peerId);

    t_ProfileRecord *  currRecord;
    currRecord  = &( (*profileArray_)[currIndex] );
    t_ProfileRecord *  prevRecord;
    t_ProfileRecord *  nextRecord;


    // Update quality indexes if required
    //
    highestQualityIndex_  = linkEnd;
    lowestQualityIndex_   = linkEnd;
    
    if (highestQualityIndex_ == currIndex) {
        highestQualityIndex_ = currRecord->prev;
    }
    if (lowestQualityIndex_ == currIndex) {
        lowestQualityIndex_ = currRecord->next;
    }
    if (baseIndex_ == currIndex) {
        if (currRecord->prev != linkEnd) {
            baseIndex_ = currRecord->prev;
        }
        else {
            baseIndex_ = currRecord->next;
        }
    }


    // Pull from doubly linked profile list
    //
    if (currRecord->prev != linkEnd) {
        prevRecord = &( (*profileArray_)[currRecord->prev] );
        prevRecord->next = currRecord->next;
    }
    if (currRecord->next != linkEnd) {
        nextRecord = &( (*profileArray_)[currRecord->next] );
        nextRecord->prev = currRecord->prev;
    }

    freeIndexList_->push_back (currIndex);
    numRecords_--;


    return true;
}



long  
AlpinePeerProfileIndex::size ()
{
    return numRecords_;
}



bool  
AlpinePeerProfileIndex::exists (ulong  peerId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpinePeerProfileIndex::exists invoked.  Peer ID: "s +
                std::to_string (peerId));
#endif

    ReadLock   lock(dataLock_);

    if (profileIndex_->find (peerId) == profileIndex_->end ()) {
        return false;
    }
    return true;
}



bool  
AlpinePeerProfileIndex::locate (ulong                 peerId,
                                AlpinePeerProfile *&  profile)
{
#ifdef _VERBOSE
    Log::Debug ("AlpinePeerProfileIndex::locate invoked.  Peer ID: "s +
                std::to_string (peerId));
#endif

    ReadLock   lock(dataLock_);

    auto indexIter = profileIndex_->find (peerId);

    if (indexIter == profileIndex_->end ()) {
        Log::Error ("Attempt to locate non existant peer ID profile in "
                             "AlpinePeerProfileIndex::locate!");

        return false;
    }
    long currIndex = (*indexIter).second;
    t_ProfileRecord *  currRecord;
    currRecord = &( (*profileArray_)[currIndex] );

    profile = currRecord->profile;


    return true;
}



bool  
AlpinePeerProfileIndex::adjustQuality (ulong  peerId,
                                       short  delta)
{
#ifdef _VERBOSE
    Log::Debug ("AlpinePeerProfileIndex::adjustQuality invoked.  Values:"s +
                "\n Peer ID: "s + std::to_string (peerId) +
                "\n Q Delta: "s + std::to_string (delta) +
                "\n");
#endif

    WriteLock   lock(dataLock_);

    auto indexIter = profileIndex_->find (peerId);

    if (indexIter == profileIndex_->end ()) {
        Log::Error ("Attempt to adjust non existant peer ID profile in "
                             "AlpinePeerProfileIndex::adjustQuality!");

        return false;
    }
    long currIndex = (*indexIter).second;
    t_ProfileRecord *  currRecord;
    currRecord = &( (*profileArray_)[currIndex] );

    AlpinePeerProfile *  profile;
    profile = currRecord->profile;

    short value;
    profile->modifyRelativeQuality (delta);
    profile->getRelativeQuality (value);

    relocate (currIndex, currRecord, value);


    return true;
}



bool  
AlpinePeerProfileIndex::querySent (ulong  peerId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpinePeerProfileIndex::querySent invoked.  Peer ID: "s +
                std::to_string (peerId));
#endif

    WriteLock   lock(dataLock_);

    auto indexIter = profileIndex_->find (peerId);

    if (indexIter == profileIndex_->end ()) {
        Log::Error ("Attempt to update non existant peer ID profile in "
                             "AlpinePeerProfileIndex::querySent!");

        return false;
    }
    long currIndex = (*indexIter).second;
    t_ProfileRecord *  currRecord;
    currRecord = &( (*profileArray_)[currIndex] );

    AlpinePeerProfile *  profile;
    profile = currRecord->profile;
    profile->incrTotalQueries ();


    return true;
}



bool  
AlpinePeerProfileIndex::responseReceived (ulong  peerId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpinePeerProfileIndex::responseReceived invoked.  Peer ID: "s +
                std::to_string (peerId));
#endif

    WriteLock   lock(dataLock_);

    auto indexIter = profileIndex_->find (peerId);

    if (indexIter == profileIndex_->end ()) {
        Log::Error ("Attempt to update non existant peer ID profile in "
                             "AlpinePeerProfileIndex::responseReceived!");

        return false;
    }
    long currIndex = (*indexIter).second;
    t_ProfileRecord *  currRecord;
    currRecord = &( (*profileArray_)[currIndex] );

    AlpinePeerProfile *  profile;
    profile = currRecord->profile;
    profile->incrTotalQueries ();

    bool   status;
    short  delta;
    status = AlpineRatingEngine::queryResponseEvent (profile, delta);

    if (!status) {
        Log::Error ("Computing quality delta for query response event failed in "
                             "AlpinePeerProfileIndex::responseReceived!");
        return false;
    }
    short value;
    profile->modifyRelativeQuality (delta);
    profile->getRelativeQuality (value);

    relocate (currIndex, currRecord, value);


    return true;
}


 
bool  
AlpinePeerProfileIndex::badPacketReceived (ulong  peerId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpinePeerProfileIndex::badPacketReceived invoked.  Peer ID: "s +
                std::to_string (peerId));
#endif

    WriteLock   lock(dataLock_);

    auto indexIter = profileIndex_->find (peerId);

    if (indexIter == profileIndex_->end ()) {
        Log::Error ("Attempt to update non existant peer ID profile in "
                             "AlpinePeerProfileIndex::badPacketReceived!");

        return false;
    }
    long currIndex = (*indexIter).second;
    t_ProfileRecord *  currRecord;
    currRecord = &( (*profileArray_)[currIndex] );

    AlpinePeerProfile *  profile;
    profile = currRecord->profile;

    bool   status;
    short  delta;
    status = AlpineRatingEngine::badPacketEvent (profile, delta);

    if (!status) {
        Log::Error ("Computing quality delta for bad packet event failed in "
                             "AlpinePeerProfileIndex::badPacketReceived!");
        return false;
    }
    short value;
    profile->modifyRelativeQuality (delta);
    profile->getRelativeQuality (value);

    relocate (currIndex, currRecord, value);


    return true;
}


 
bool  
AlpinePeerProfileIndex::reliableTransferFailed (ulong  peerId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpinePeerProfileIndex::reliableTransferFailed invoked.  Peer ID: "s +
                std::to_string (peerId));
#endif

    WriteLock   lock(dataLock_);

    auto indexIter = profileIndex_->find (peerId);

    if (indexIter == profileIndex_->end ()) {
        Log::Error ("Attempt to update non existant peer ID profile in "
                             "AlpinePeerProfileIndex::reliableTransferFailed!");

        return false;
    }
    long currIndex = (*indexIter).second;
    t_ProfileRecord *  currRecord;
    currRecord = &( (*profileArray_)[currIndex] );

    AlpinePeerProfile *  profile;
    profile = currRecord->profile;

    bool   status;
    short  delta;
    status = AlpineRatingEngine::transferFailureEvent (profile, delta);

    if (!status) {
        Log::Error ("Computing quality delta for transfer failuere event failed in "
                             "AlpinePeerProfileIndex::reliableTransferFailed!");
        return false;
    }
    short value;
    profile->modifyRelativeQuality (delta);
    profile->getRelativeQuality (value);

    relocate (currIndex, currRecord, value);


    return true;
}



bool  
AlpinePeerProfileIndex::highestQuality (ulong &  peerId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpinePeerProfileIndex::highestQuality invoked.");
#endif

    WriteLock   lock(dataLock_);

    if (numRecords_ == 0) {
        return false;
    }
    t_ProfileRecord *  currRecord;
    currRecord = &( (*profileArray_)[highestQualityIndex_] );

    currRecord->profile->getPeerId (peerId);


    return true;
}



bool  
AlpinePeerProfileIndex::lowestQuality (ulong &  peerId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpinePeerProfileIndex::lowestQuality invoked.");
#endif

    WriteLock   lock(dataLock_);

    if (numRecords_ == 0) {
        return false; 
    }
    t_ProfileRecord *  currRecord;
    currRecord = &( (*profileArray_)[lowestQualityIndex_] );

    currRecord->profile->getPeerId (peerId);


    return true;
}



bool  
AlpinePeerProfileIndex::getAllPeers (t_PeerIdList &  peerIdList)
{
#ifdef _VERBOSE
    Log::Debug ("AlpinePeerProfileIndex::getAllPeers invoked.");
#endif

    ReadLock  lock(dataLock_);

    peerIdList.clear ();

    if (numRecords_ == 0) {
        return true;
    }
    ulong currId;

    for (const auto& item : *profileIndex_) {
        currId = item.first;
        peerIdList.push_back (currId);
    }


    return true;
}



bool
AlpinePeerProfileIndex::getWeightedPeerList (t_PeerIdList &  peerIdList,
                                             ulong           limit)
{
#ifdef _VERBOSE
    Log::Debug ("AlpinePeerProfileIndex::getWeightedPeerList invoked.");
#endif

    peerIdList.clear();

    ReadLock lock(dataLock_);

    if (numRecords_ == 0)
        return true;

    // Collect peer IDs and their weights from the rating engine
    // Weight = score^2 (amplifies good peers, suppresses bad)
    //
    static constexpr double  kBanThreshold     = 0.1;
    static constexpr double  kUnscoredWeight   = 0.25;

    struct t_WeightedPeer {
        ulong   peerId;
        double  weight;
    };

    vector<t_WeightedPeer>  scoredPeers;
    vector<ulong>           unscoredPeers;

    for (const auto & [peerId, arrayIndex] : *profileIndex_) {
        double score = AlpineRatingEngine::getScore(peerId);

        // Default score (0.5) means unscored — treat as unscored fallback
        if (score == 0.5) {
            unscoredPeers.push_back(peerId);
            continue;
        }
        if (score < kBanThreshold)
            continue;

        double weight = score * score;
        scoredPeers.push_back({peerId, weight});
    }

    // Efraimidis-Spirakis weighted reservoir sampling
    // Key = pow(random(0,1), 1/weight) — higher weight = higher expected key
    //
    ulong selectCount = limit;
    if (selectCount == 0)
        selectCount = static_cast<ulong>(scoredPeers.size() + unscoredPeers.size());

    // Min-heap of (key, peerId) — keeps the top-N highest keys
    //
    using t_KeyPeer = std::pair<double, ulong>;
    std::priority_queue<t_KeyPeer, vector<t_KeyPeer>, std::greater<t_KeyPeer>>  reservoir;

    thread_local std::mt19937  rng(std::random_device{}());
    std::uniform_real_distribution<double>  dist(0.0001, 1.0);

    for (const auto & peer : scoredPeers) {
        double key = std::pow(dist(rng), 1.0 / peer.weight);

        if (reservoir.size() < selectCount) {
            reservoir.push({key, peer.peerId});
        }
        else if (key > reservoir.top().first) {
            reservoir.pop();
            reservoir.push({key, peer.peerId});
        }
    }

    // Fallback: if we have fewer than limit scored peers, include unscored at weight 0.25
    //
    if (reservoir.size() < selectCount) {
        for (ulong peerId : unscoredPeers) {
            double key = std::pow(dist(rng), 1.0 / kUnscoredWeight);

            if (reservoir.size() < selectCount) {
                reservoir.push({key, peerId});
            }
            else if (key > reservoir.top().first) {
                reservoir.pop();
                reservoir.push({key, peerId});
            }
        }
    }

    // Extract results — highest weight first
    //
    peerIdList.resize(reservoir.size());
    for (auto i = static_cast<long>(peerIdList.size()) - 1; i >= 0; --i) {
        peerIdList[static_cast<size_t>(i)] = reservoir.top().second;
        reservoir.pop();
    }


    return true;
}



bool  
AlpinePeerProfileIndex::resize (long size)
{
#ifdef _VERBOSE
    Log::Debug ("AlpinePeerProfileIndex::resize invoked.");
#endif

    // Allocate new larger containers.  MUST be called only when all records in use (full)
    //
    t_ProfileRecordArray *  newProfileRecordArray;
    newProfileRecordArray = new t_ProfileRecordArray;

    newProfileRecordArray->clear ();
    newProfileRecordArray->resize (size);

    // There should not be anything in here anyway...
    freeIndexList_->clear ();

    // Copy current profile records
    //
    long i;
    for (i = 0; i < size_; i++) {
        (*newProfileRecordArray)[i].profile = (*profileArray_)[i].profile;
        (*newProfileRecordArray)[i].prev = (*profileArray_)[i].prev;
        (*newProfileRecordArray)[i].next = (*profileArray_)[i].next;
    }

    // Initialize remaining records.
    //
    for (i = size_; i < size; i++) {
        freeIndexList_->push_back (i);
        (*newProfileRecordArray)[i].profile = nullptr;
        (*newProfileRecordArray)[i].prev = linkEnd;
        (*newProfileRecordArray)[i].next = linkEnd;
    }

    delete profileArray_;
    size_         = size;
    profileArray_ = newProfileRecordArray;


    return true;
}



bool  
AlpinePeerProfileIndex::relocate (long               currIndex,
                                  t_ProfileRecord *  currRecord,
                                  short              currValue)
{
#ifdef _VERBOSE
    Log::Debug ("AlpinePeerProfileIndex::relocate invoked.");
#endif

    t_ProfileRecord *  prevRecord = nullptr;
    t_ProfileRecord *  nextRecord = nullptr;

    // Determine which way, if any, we will shift this record
    //
    if (currRecord->prev != linkEnd) {
        prevRecord = &( (*profileArray_)[currRecord->prev] );
    }
    if (currRecord->next != linkEnd) {
        nextRecord = &( (*profileArray_)[currRecord->next] );
    }

    short  nextQuality = currValue; // for movement check
    short  prevQuality = currValue;

    if (nextRecord) {
        nextRecord->profile->getRelativeQuality (nextQuality);
    }
    if (prevRecord) {
        prevRecord->profile->getRelativeQuality (prevQuality);
    }

    if ( (currValue <= nextQuality) && (currValue >= prevQuality) ) {
        // No change in relative position, we're done.
        //
        return true;
    }
    // Remove this profile record from current position.
    //
    highestQualityIndex_  = linkEnd;
    lowestQualityIndex_   = linkEnd;
    
    if (highestQualityIndex_ == currIndex) {
        highestQualityIndex_ = currRecord->prev;
    }
    if (lowestQualityIndex_ == currIndex) {
        lowestQualityIndex_ = currRecord->next;
    }
    if (baseIndex_ == currIndex) {
        if (currRecord->prev != linkEnd) {
            baseIndex_ = currRecord->prev;
        }
        else {
            baseIndex_ = currRecord->next;
        }
    }

    if (prevRecord) {
        prevRecord->next = currRecord->next;
    }
    if (nextRecord) {
        nextRecord->prev = currRecord->prev;
    }

    long  nextIndex;
    long  prevIndex;
    bool  insertPointFound = false;

    if (currValue > nextQuality) {
        // Increase in quality, move higher
        //
        nextIndex = nextRecord->next;

        // If this is the highest quality profile, no need to iterate, we place
        // at top of list.
        //
        if (nextIndex == linkEnd) {
            // Let the code at the end of the loop handle this case
            //
            insertPointFound = true;
        }

        // iterate up the list until we find record of greater quality
        //
        long  insertIndex = nextIndex;

        while (!insertPointFound) {
            nextRecord = &( (*profileArray_)[nextIndex] );
            insertIndex = nextIndex;
            nextIndex = nextRecord->next;
            nextRecord->profile->getRelativeQuality (nextQuality);

            if (nextQuality >= currValue) {
                // Found our higher block
                insertPointFound = true;
            }
        }


        // If we are at the top of the list, update links and return.
        //
        if (nextIndex == linkEnd) {
            nextRecord->next = currIndex;
            currRecord->next = linkEnd;
            currRecord->prev = insertIndex;
            highestQualityIndex_ = currIndex;

            // See if base index update required
            //
            short currDistance = abs(currValue);
            if (baseIndex_ == insertIndex) {
                nextRecord->profile->getRelativeQuality (prevQuality);
                short baseDistance = abs(prevQuality);

                if (currDistance < baseDistance) {
                    baseIndex_ = currIndex;
                }
            }

            return true;
        }
        // Insert into current location
        //
        long prevIndex = nextRecord->prev;
        prevRecord = &( (*profileArray_)[prevIndex] );
        nextRecord->prev = currIndex;
        prevRecord->next = currIndex;
        currRecord->next = insertIndex;
        currRecord->prev = prevIndex;

        // See if we need to update the base index
        //
        short currDistance = abs(currValue);
        if (baseIndex_ == prevIndex) {
            prevRecord->profile->getRelativeQuality (prevQuality);
            short baseDistance = abs(prevQuality);

            if (currDistance < baseDistance) {
                baseIndex_ = currIndex;
            }
        }
        else if (baseIndex_ == insertIndex) {
            nextRecord->profile->getRelativeQuality (nextQuality);
            short baseDistance = abs(nextQuality);

            if (currDistance < baseDistance) {
                baseIndex_ = currIndex;
            }
        }
    }
    else {
        // Decrease in quality, move lower
        //
        prevIndex = prevRecord->prev;

        // If this is the lowest quality profile, no need to iterate, we place
        // at bottom of list.
        //
        if (prevIndex == linkEnd) {
            // Let the code at the end of the loop handle this case
            //
            insertPointFound = true;
        }

        // iterate down the list until we find record of lesser quality
        //
        long  insertIndex = prevIndex;

        while (!insertPointFound) {
            prevRecord = &( (*profileArray_)[prevIndex] );
            insertIndex = prevIndex;
            prevIndex = prevRecord->prev;
            prevRecord->profile->getRelativeQuality (prevQuality);

            if (prevQuality < currValue) {
                // Found lower quality profile
                insertPointFound = true;
            }
        }


        // If we are at the bottom of the list, update links and return.
        //
        if (prevIndex == linkEnd) {
            prevRecord->prev = currIndex;
            currRecord->prev = linkEnd;
            currRecord->next = insertIndex;
            lowestQualityIndex_ = currIndex;

            // See if base index update required
            //
            short currDistance = abs(currValue);
            if (baseIndex_ == insertIndex) {
                prevRecord->profile->getRelativeQuality (prevQuality);
                short baseDistance = abs(prevQuality);

                if (currDistance < baseDistance) {
                    baseIndex_ = currIndex;
                }
            }

            return true;
        }
        // Insert into current location
        //
        long nextIndex = prevRecord->next;
        nextRecord = &( (*profileArray_)[nextIndex] );
        prevRecord->next = currIndex;
        nextRecord->prev = currIndex;
        currRecord->prev = insertIndex;
        currRecord->next = nextIndex;

        // See if we need to update the base index
        //
        short currDistance = abs(currValue);
        if (baseIndex_ == nextIndex) {
            nextRecord->profile->getRelativeQuality (nextQuality);
            short baseDistance = abs(nextQuality);

            if (currDistance < baseDistance) {
                baseIndex_ = currIndex;
            }
        }
        else if (baseIndex_ == insertIndex) {
            prevRecord->profile->getRelativeQuality (prevQuality);
            short baseDistance = abs(prevQuality);

            if (currDistance < baseDistance) {
                baseIndex_ = currIndex;
            }
        }
    }


    return true;
}



