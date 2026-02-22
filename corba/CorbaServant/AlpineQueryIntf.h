/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <AlpineCorbaClient.h>
#include <CorbaUtils.h>
#include <string>
#include <vector>
#include <OptHash.h>


class AlpineQueryIntf
{
  public:


    // Public types
    //
    struct t_QueryOptions {
        string    groupName;
        ulong     autoHaltLimit;
        bool      autoDownload;
        ulong     peerDescMax;
        ulong     optionId;
        string    optionData;
    };


    struct t_QueryStatus {
        ulong   totalPeers;
        ulong   peersQueried;
        ulong   numPeerResponses;
        ulong   totalHits;
    };


    struct t_ResourceDesc {
        ulong    resourceId;
        ulong    size;
        string   locator;
        string   description;
        ulong    optionId;
        string   optionData;
    };

    using t_ResourceDescList = vector<t_ResourceDesc>;


    struct t_PeerResources {
        unsigned long       peerId;
        t_ResourceDescList  resourceDescList;
    };

    using t_PeerResourcesIndex = std::unordered_map < ulong, // Peer ID
                       t_PeerResources,
                       OptHash<ulong>,
                       equal_to<ulong> >;

    using t_PeerResourcesIndexPair = std::pair <ulong, t_PeerResources>;

    using t_QueryIdList = vector<ulong>;





    // Supported interface operations
    //
    static bool  getDefaultOptions (t_QueryOptions &  options);

    static bool  setDefaultOptions (const t_QueryOptions &  options);

    static bool  startQuery (const t_QueryOptions &  options,
                             const string &          queryString,
                             ulong &                 queryId);

    static bool  inProgress (ulong  queryId);

    static bool  getQueryStatus (ulong            queryId,
                                 t_QueryStatus &  status);

    static bool  pauseQuery (ulong  queryId);

    static bool  resumeQuery (ulong  queryId);

    static bool  cancelQuery (ulong  queryId);

    static bool  getQueryResults (ulong                   queryId,
                                  t_PeerResourcesIndex &  results);

    static bool  getActiveQueryIdList (t_QueryIdList &  queryIdList);

    static bool  getPastQueryIdList (t_QueryIdList &  queryIdList);



  private:

};

