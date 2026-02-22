/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <OptHash.h>
#include <AgedQueue.h>
#include <sys/time.h>
#include <string>
#include <list>


class DtcpMemTest
{
  public:

    DtcpMemTest ();
    ~DtcpMemTest ();


    bool  allocate (ulong  numPeers);

    bool  cleanUp ();


    // Emulated DTCP object memory use
    //
    struct t_ConnTransportData {
        void *                  parentTransport_;
        void *                  mux_;
        ulong                   transportId_;
        ulong                   peerId_;
        ulong                   myId_;
        ulong                   peerIpAddress_;
        ushort                  peerPort_;
#ifndef _LEAN_N_MEAN
        struct timeval          lastRecv_;
        struct timeval          lastSend_;
        bool                    pendingAck_;
        ulong                   requestId_;
        ulong                   sendSequenceNum_;
        ulong                   recvSequenceNum_;
#endif
    };

#ifndef _LEAN_N_MEAN
    struct t_IpPortPair {
        ulong           ipAddress;
        int             port;
        struct timeval  timestamp;
    };
#endif



    // Indexes
    //
    using t_TransportList = list<t_ConnTransportData *>;

#ifndef _LEAN_N_MEAN
    using t_LocationList = list<t_IpPortPair *>;
#endif

    using t_PeerIdIndex = std::unordered_map< ulong,
                       t_ConnTransportData *,
                       OptHash<ulong>,
                       equal_to<ulong> >;

    using t_PeerIdIndexPair = std::pair <ulong, t_ConnTransportData *>;

#ifndef _LEAN_N_MEAN
    using t_LocationIndex = std::unordered_map< ulong,
                       t_IpPortPair *,
                       OptHash<ulong>,
                       equal_to<ulong> >;

    using t_LocationIndexPair = std::pair <ulong, t_IpPortPair *>;
#else
    using t_LocationIndex = std::unordered_map< ulong, // peer ID
                       ulong, // IP
                       OptHash<ulong>,
                       equal_to<ulong> >;

    using t_LocationIndexPair = std::pair <ulong, ulong>;
#endif

    using t_TransportIdIndex = std::unordered_map< ulong,
                       t_ConnTransportData *,
                       OptHash<ulong>,
                       equal_to<ulong> >;

    using t_TransportIdIndexPair = std::pair <ulong, t_ConnTransportData *>;



  private:

    ulong                  currPeerId_;
    ulong                  currTransportId_;

    t_TransportList *      transportList_;
#ifndef _LEAN_N_MEAN
    t_LocationList *       locationList_;   
#endif
    t_PeerIdIndex *        peerIdIndex_;
    t_LocationIndex *      locationIndex_;
    t_TransportIdIndex *   transportIndex_;

};


