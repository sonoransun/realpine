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
#include <AlpinePeerProfile.h>
#include <ReadWriteSem.h>
#include <OptHash.h>
#include <vector>
#include <memory>
#include <optional>


class AlpineDtcpConnTransport;
class AlpinePeerProfileIndex;


class AlpinePeerMgr
{
  public:

    AlpinePeerMgr () = default;
    ~AlpinePeerMgr () = default;


  
    static bool  getAlias (ulong     peerId,
                           string &  alias);

    static bool  setAlias (ulong           peerId,
                           const string &  alias);

    static bool  getProfile (ulong                peerId,
                             AlpinePeerProfile &  profile);  // global peer profile

    static bool  getGroupProfile (ulong                peerId,
                                  ulong                groupId,
                                  AlpinePeerProfile &  profile);  // group specific peer profile

    static bool  getTransport (ulong                       peerId,
                               AlpineDtcpConnTransport *&  transport);

    static bool  badPacketReceived (ulong  peerId);

    static bool  reliableTransferFailed (ulong  peerId);



    // Internal types
    //
    using t_GroupIdList = vector<ulong>;

    struct t_PeerInfo {
        ulong                            peerId;
        AlpineDtcpConnTransport *        transport;
        std::optional<string>            alias;
        std::unique_ptr<t_GroupIdList>   groupList;
    };

    using t_PeerInfoIndex = std::unordered_map< ulong,
                      std::unique_ptr<t_PeerInfo>,
                      OptHash<ulong>,
                      equal_to<ulong> >;

    using t_PeerInfoIndexPair = std::pair<ulong, std::unique_ptr<t_PeerInfo>>;


  private:

    static bool                      initialized_s;
    static AlpinePeerProfileIndex *  baseProfileIndex_s;
    static ReadWriteSem              dataLock_s;
    static t_PeerInfoIndex *         peerInfoIndex_s;
    static ReadWriteSem              peerInfoLock_s;


    // Initialization performed by AlpineStack
    //
    static bool  initialize ();

  
    static bool  querySent (ulong  peerId);

    static bool  responseReceived (ulong  peerId);

    static bool  registerTransport (ulong                      peerId,
                                    AlpineDtcpConnTransport *  transport);

    static bool  deactivatePeer (ulong  peerId);

    static bool  deletePeer (ulong  peerId);

    static bool  banPeer (ulong  peerId);


    // Timer and event management entry point
    // Used solely by the AlpineStack
    //
    static void  processTimedEvents ();

                                 

    friend class AlpineStack;
    friend class AlpineQueryResults;
    friend class AlpineDtcpConnAcceptor;
    friend class AlpineDtcpConnConnector;
};


