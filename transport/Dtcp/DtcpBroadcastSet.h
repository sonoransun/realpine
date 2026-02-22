/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <list>
#include <vector>
#include <OptHash.h>


class DtcpBaseConnTransport;


class DtcpBroadcastSet
{
  public:


    // Public types
    // 
    using t_TransportList = list<DtcpBaseConnTransport *>;

    using t_TransportIdList = vector<ulong>;



    // Constructors
    //
    DtcpBroadcastSet ();

    DtcpBroadcastSet (t_TransportList &  transportList);

    DtcpBroadcastSet (t_TransportIdList &  transportIdList);

    ~DtcpBroadcastSet ();



    // Public operations
    //
    ulong  size ();

    // Insert always places new transport(s) at end of set.
    //
    bool  insert (DtcpBaseConnTransport *  transport);

    bool  insert (ulong  transportId);

    bool  exists (DtcpBaseConnTransport *  transport);

    bool  exists (ulong  transportId);

    bool  getTransportList (t_TransportList &  transportList);

    bool  getTransportIdList (t_TransportIdList &  transportIdList);

    // Remove is NOT EFFICIENT.  Use with care
    //
    bool  remove (DtcpBaseConnTransport *  transport);

    bool  remove (ulong  transportId);

    bool  clear ();



    // Internal types
    //
    using t_TransportIndex = std::unordered_map<ulong, // transport ID
                      ulong, // transport index
                      OptHash<ulong>,
                      equal_to<ulong> >;

    using t_TransportIndexPair = std::pair<ulong, ulong>;

    using t_TransportArray = vector<DtcpBaseConnTransport *>;
                 

  private:

    t_TransportArray *  transportArray_;
    t_TransportIndex *  transportIndex_;
    uint                reserve_;


    bool  extendTransportArray (uint  extent);

};

