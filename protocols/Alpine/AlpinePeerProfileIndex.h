/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <ReadWriteSem.h>
#include <OptHash.h>
#include <list>
#include <vector>


class AlpinePeerProfile;


class AlpinePeerProfileIndex
{
  public:

    AlpinePeerProfileIndex (long  reserve = 1024);

    ~AlpinePeerProfileIndex ();


    using t_PeerIdList = vector<ulong>;


    bool  create (ulong  peerId);

    bool  erase (ulong  peerId);

    long  size ();

    bool  exists (ulong  peerId);

    bool  locate (ulong                 peerId,
                  AlpinePeerProfile *&  profile);

    bool  adjustQuality (ulong  peerId,
                         short  delta); 

    bool  querySent (ulong  peerId);

    bool  responseReceived (ulong  peerId);

    bool  badPacketReceived (ulong  peerId);

    bool  reliableTransferFailed (ulong  peerId);
 
    bool  highestQuality (ulong &  peerId);

    bool  lowestQuality (ulong &  peerId);

    bool  getAllPeers (t_PeerIdList &  peerIdList); // ordered according to quality, high to low

    bool  getWeightedPeerList (t_PeerIdList &  peerIdList, // higher quality to lower, however, semi-random
                               ulong           limit = 0);



    // Internal types used for indexing and ordering profiles
    //
    // These containers map the relationship between an oft modified quality value
    // relative to the other members of this index, as well as a hash to the actual record.
    // (hence the convoluted data structure to make ordering and searching fast)
    //
    struct t_ProfileRecord {
        AlpinePeerProfile *  profile;
        long                 next;
        long                 prev;
    };

    using t_ProfileRecordArray = vector<t_ProfileRecord>;

    using t_FreeIndexList = list<long>;

    using t_ProfileIndex = std::unordered_map< ulong, // peer ID
                      long,  // profile array index
                      OptHash<ulong>,
                      equal_to<ulong> >;

    using t_ProfileIndexPair = std::pair<ulong, long>;



  private:

    long                    size_;
    long                    numRecords_;
    t_ProfileRecordArray *  profileArray_;
    t_FreeIndexList *       freeIndexList_;
    t_ProfileIndex *        profileIndex_;
    long                    highestQualityIndex_;
    long                    lowestQualityIndex_;
    long                    baseIndex_; // the base index should always point to the record
                                        // closest to the base quality value of 0.
    ReadWriteSem            dataLock_;


    bool  resize (long size);

    bool  relocate (long               currIndex,
                    t_ProfileRecord *  currRecord,
                    short              currValue);


    // Copy constructor and assignement operator not implemented
    //
    AlpinePeerProfileIndex (const AlpinePeerProfileIndex & copy);
    AlpinePeerProfileIndex & operator = (const AlpinePeerProfileIndex & copy);
};


