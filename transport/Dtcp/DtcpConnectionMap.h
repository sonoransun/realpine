/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <AgedQueue.h>
#include <Common.h>
#include <OptHash.h>
#include <Platform.h>
#include <string>


class DtcpBaseConnTransport;


class DtcpConnectionMap
{
  public:
    DtcpConnectionMap();
    ~DtcpConnectionMap();


    bool exists(ulong ipAddress, int port);

    bool exists(ulong peerId);

    bool createConnection(ulong destIpAddress, int destPort, ulong & peerId);

    bool pendingAccept(ulong peerId);

    bool removePendingConnection(ulong peerId);

    bool removeConnection(DtcpBaseConnTransport * connection);

    bool indexConnection(ulong peerId, DtcpBaseConnTransport * connection);

    bool locateConnection(ulong peerId, DtcpBaseConnTransport *& connection);

    bool locateConnection(ulong ipAddress, int port, DtcpBaseConnTransport *& connection);

    int numConnections(ulong ipAddress);


    ////
    //
    // Indexing Data Structures
    //

    struct t_IpPortPair
    {
        ulong ipAddress;
        int port;
        struct timeval timestamp;
    };

    using t_PeerIdIndex = std::unordered_map<ulong, DtcpBaseConnTransport *, OptHash<ulong>, equal_to<ulong>>;

    using t_PeerIdIndexPair = std::pair<ulong, DtcpBaseConnTransport *>;

    using t_PortIndex = std::unordered_map<int, DtcpBaseConnTransport *, OptHash<int>, equal_to<int>>;

    using t_PortIndexPair = std::pair<int, DtcpBaseConnTransport *>;

    using t_IpAddrPortIndex = std::unordered_map<ulong, t_PortIndex *, OptHash<ulong>, equal_to<ulong>>;

    using t_IpAddrPortIndexPair = std::pair<ulong, t_PortIndex *>;

    using t_PendingAcceptIndex = std::unordered_map<ulong, t_IpPortPair *, OptHash<ulong>, equal_to<ulong>>;

    using t_PendingAcceptIndexPair = std::pair<ulong, t_IpPortPair *>;


  private:
    ulong numConnections_;
    ulong currConnectionId_;  // to generate IDs
    t_PeerIdIndex peerIndex_;
    t_IpAddrPortIndex addressIndex_;
    t_PendingAcceptIndex pendingIndex_;
};
