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
#include <OptHash.h>
#include <AgedQueue.h>
#include <Platform.h>
#include <string>


class DtcpBaseConnTransport;


class DtcpConnectionMap
{
  public:

    DtcpConnectionMap ();
    ~DtcpConnectionMap ();


    bool exists (ulong ipAddress,
                 int   port);

    bool exists (ulong peerId);

    bool createConnection (ulong    destIpAddress,
                           int      destPort,
                           ulong &  peerId);

    bool pendingAccept (ulong peerId);

    bool removePendingConnection (ulong peerId);

    bool removeConnection (DtcpBaseConnTransport * connection);

    bool indexConnection (ulong                    peerId,
                          DtcpBaseConnTransport *  connection);

    bool locateConnection (ulong                     peerId,
                           DtcpBaseConnTransport *&  connection);

    bool locateConnection (ulong                     ipAddress,
                           int                       port,
                           DtcpBaseConnTransport *&  connection);

    int  numConnections (ulong ipAddress);



    ////
    //
    // Indexing Data Structures
    //

    struct t_IpPortPair {
        ulong           ipAddress;
        int             port;
        struct timeval  timestamp;
    };

    using t_PeerIdIndex = std::unordered_map< ulong,
                       DtcpBaseConnTransport *,
                       OptHash<ulong>,
                       equal_to<ulong> >;

    using t_PeerIdIndexPair = std::pair <ulong, DtcpBaseConnTransport *>;

    using t_PortIndex = std::unordered_map< int,
                       DtcpBaseConnTransport *,
                       OptHash<int>,
                       equal_to<int> >;

    using t_PortIndexPair = std::pair <int, DtcpBaseConnTransport *>;

    using t_IpAddrPortIndex = std::unordered_map< ulong,
                       t_PortIndex *,
                       OptHash<ulong>,
                       equal_to<ulong> >;

    using t_IpAddrPortIndexPair = std::pair <ulong, t_PortIndex *>;

    using t_PendingAcceptIndex = std::unordered_map< ulong,
                       t_IpPortPair *,
                       OptHash<ulong>,
                       equal_to<ulong> >;

    using t_PendingAcceptIndexPair = std::pair <ulong, t_IpPortPair *>;



  private:

    ulong                 numConnections_;
    ulong                 currConnectionId_;  // to generate IDs
    t_PeerIdIndex         peerIndex_;
    t_IpAddrPortIndex     addressIndex_;
    t_PendingAcceptIndex  pendingIndex_;

};


