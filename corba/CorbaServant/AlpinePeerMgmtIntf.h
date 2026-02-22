/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <AlpineCorbaClient.h>
#include <CorbaUtils.h>
#include <string>
#include <vector>


class AlpinePeerMgmtIntf
{
  public:


    // Public types
    //
    struct t_AlpinePeerInfo {
        ulong    id;
        string   alias;
        string   description;
        ulong    dateConnected;
        ulong    qualityPosition;
        ulong    relativeQualityRating;
        ulong    totalShared;
    };

    using t_AlpinePeerInfoList = vector<t_AlpinePeerInfo>;

    using t_PeerIdList = vector<ulong>;


    // Supported interface operations
    //
    static bool  getExtendedPeerList (t_PeerIdList & peerList);

    static bool  getPeerInformation (t_AlpinePeerInfo & peerInfo);

    static bool  updatePeerInformation (const t_AlpinePeerInfo & peerInfo);


  private:

};

