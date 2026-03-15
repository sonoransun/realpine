/// Copyright (C) 2026 sonoransun — see LICENCE.txt


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
        uint16_t                         negotiatedVersion{0};
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


