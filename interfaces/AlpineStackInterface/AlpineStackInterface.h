/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <ConfigData.h>
#include <Error.h>
#include <Mutex.h>
#include <OptHash.h>
#include <functional>
#include <string>
#include <vector>


class AlpineStackInterface
{
  public:
    // Public types
    //
    struct t_QueryOptions
    {
        string groupName;
        ulong autoHaltLimit;
        bool autoDownload;
        ulong peerDescMax;
        ulong optionId;
        string optionData;
        uint8_t priority{128};
    };


    struct t_QueryStatus
    {
        ulong totalPeers;
        ulong peersQueried;
        ulong numPeerResponses;
        ulong totalHits;
    };

    using t_LocatorList = vector<string>;

    struct t_ResourceDesc
    {
        ulong resourceId;
        ulong size;
        t_LocatorList locators;
        string description;
        ulong optionId;
        string optionData;
    };

    using t_ResourceDescList = vector<t_ResourceDesc>;


    struct t_PeerResources
    {
        unsigned long peerId;
        t_ResourceDescList resourceDescList;
    };

    using t_PeerResourcesIndex = std::unordered_map<ulong,  // Peer ID
                                                    t_PeerResources,
                                                    OptHash<ulong>,
                                                    equal_to<ulong>>;

    using t_PeerResourcesIndexPair = std::pair<ulong, t_PeerResources>;


    struct t_GroupInfo
    {
        ulong groupId;
        string groupName;
        string description;
        ulong numPeers;
        ulong totalQueries;
        ulong totalResponses;
    };

    struct t_PeerProfile
    {
        ulong peerId;
        short relativeQuality;
        ulong totalQueries;
        ulong totalResponses;
    };


    struct t_ModuleInfo
    {
        ulong moduleId;
        string moduleName;
        string description;
        string version;
        string libraryPath;
        string bootstrapSymbol;
        ulong activeTime;
    };

    using t_IdList = vector<ulong>;


    // -----------------------------------------------------------------------
    // Legacy interface (bool return)
    // -----------------------------------------------------------------------

    // Query operations
    //
    [[deprecated("use startQuery2")]] [[nodiscard]] static bool
    startQuery(const t_QueryOptions & options, const string & queryString, ulong & queryId);

    [[nodiscard]] static bool queryInProgress(ulong queryId);

    [[deprecated("use getQueryStatus2")]] [[nodiscard]] static bool getQueryStatus(ulong queryId,
                                                                                   t_QueryStatus & queryStatus);

    [[nodiscard]] static bool pauseQuery(ulong queryId);

    [[nodiscard]] static bool resumeQuery(ulong queryId);

    [[nodiscard]] static bool cancelQuery(ulong queryId);

    [[deprecated("use getQueryResults2")]] [[nodiscard]] static bool getQueryResults(ulong queryId,
                                                                                     t_PeerResourcesIndex & results);


    // Group operations
    //
    [[deprecated("use createGroup2")]] [[nodiscard]] static bool
    createGroup(const string & name, const string & description, ulong & groupId);

    [[nodiscard]] static bool
    copyGroup(ulong copyGroupId, const string & name, const string & description, ulong newGroupId);

    [[nodiscard]] static bool deleteGroup(ulong groupId);

    [[nodiscard]] static bool groupExists(const string & groupName);

    [[nodiscard]] static bool groupExists(ulong groupId);

    [[deprecated("use listGroups2")]] [[nodiscard]] static bool listGroups(t_IdList & groupIdList);

    [[deprecated("use getGroupInfo2")]] [[nodiscard]] static bool getGroupInfo(ulong groupId, t_GroupInfo & groupInfo);

    [[deprecated("use getDefaultGroupInfo2")]] [[nodiscard]] static bool getDefaultGroupInfo(t_GroupInfo & groupInfo);

    [[deprecated("use getGroupPeerList2")]] [[nodiscard]] static bool getGroupPeerList(ulong groupId,
                                                                                       t_IdList & peerIdList);

    [[deprecated("use getGroupPeerProfile2")]] [[nodiscard]] static bool
    getGroupPeerProfile(ulong groupId, ulong peerId, t_PeerProfile & peerProfile);

    [[deprecated("use getDefaultPeerProfile2")]] [[nodiscard]] static bool
    getDefaultPeerProfile(ulong peerId, t_PeerProfile & peerProfile);

    [[nodiscard]] static bool addPeerToGroup(ulong groupId, ulong peerId);

    [[nodiscard]] static bool removePeerFromGroup(ulong groupId, ulong peerId);


    // Module operations
    //
    [[deprecated("use registerModule2")]] [[nodiscard]] static bool
    registerModule(const string & libraryPath, const string & boostrapSymbol, ulong & moduleId);

    [[nodiscard]] static bool moduleExists(ulong moduleId);

    [[nodiscard]] static bool moduleExists(const string & libraryPath);

    [[nodiscard]] static bool unregisterModule(ulong moduleId);

    [[nodiscard]] static bool setModuleConfiguration(ulong moduleId, ConfigData & configData);

    [[nodiscard]] static bool getModuleConfiguration(ulong moduleId, ConfigData & configData);

    [[deprecated("use getModuleInfo2")]] [[nodiscard]] static bool getModuleInfo(ulong moduleId,
                                                                                 t_ModuleInfo & moduleInfo);

    [[nodiscard]] static bool loadModule(ulong moduleId);

    [[nodiscard]] static bool moduleIsActive(ulong moduleId);

    [[nodiscard]] static bool unloadModule(ulong moduleId);

    [[deprecated("use listActiveModules2")]] [[nodiscard]] static bool listActiveModules(t_IdList & idList);

    [[deprecated("use listAllModules2")]] [[nodiscard]] static bool listAllModules(t_IdList & idList);


    // -----------------------------------------------------------------------
    // Modern interface (std::expected return)
    // -----------------------------------------------------------------------

    // Query operations
    //
    [[nodiscard]] static Result<ulong> startQuery2(const t_QueryOptions & options, const string & queryString);

    [[nodiscard]] static Result<t_QueryStatus> getQueryStatus2(ulong queryId);

    [[nodiscard]] static Status pauseQuery2(ulong queryId);

    [[nodiscard]] static Status resumeQuery2(ulong queryId);

    [[nodiscard]] static Status cancelQuery2(ulong queryId);

    [[nodiscard]] static Result<t_PeerResourcesIndex> getQueryResults2(ulong queryId);


    // Group operations
    //
    [[nodiscard]] static Result<ulong> createGroup2(const string & name, const string & description);

    [[nodiscard]] static Status deleteGroup2(ulong groupId);

    [[nodiscard]] static Result<t_IdList> listGroups2();

    [[nodiscard]] static Result<t_GroupInfo> getGroupInfo2(ulong groupId);

    [[nodiscard]] static Result<t_GroupInfo> getDefaultGroupInfo2();

    [[nodiscard]] static Result<t_IdList> getGroupPeerList2(ulong groupId);

    [[nodiscard]] static Result<t_PeerProfile> getGroupPeerProfile2(ulong groupId, ulong peerId);

    [[nodiscard]] static Result<t_PeerProfile> getDefaultPeerProfile2(ulong peerId);

    [[nodiscard]] static Status addPeerToGroup2(ulong groupId, ulong peerId);

    [[nodiscard]] static Status removePeerFromGroup2(ulong groupId, ulong peerId);


    // Module operations
    //
    [[nodiscard]] static Result<ulong> registerModule2(const string & libraryPath, const string & bootstrapSymbol);

    [[nodiscard]] static Status unregisterModule2(ulong moduleId);

    [[nodiscard]] static Result<t_ModuleInfo> getModuleInfo2(ulong moduleId);

    [[nodiscard]] static Status loadModule2(ulong moduleId);

    [[nodiscard]] static Status unloadModule2(ulong moduleId);

    [[nodiscard]] static Result<t_IdList> listActiveModules2();

    [[nodiscard]] static Result<t_IdList> listAllModules2();


    // -----------------------------------------------------------------------
    // Async query interface
    // -----------------------------------------------------------------------

    using QueryCallback = std::function<void(Result<t_PeerResourcesIndex>)>;
    using StatusCallback = std::function<void(Result<t_QueryStatus>)>;

    // Callback invoked incrementally as each peer replies.
    // Parameters: queryId, peerId that just replied.
    using QueryResultCallback = std::function<void(ulong queryId, ulong peerId)>;

    [[nodiscard]] static Result<ulong>
    startQueryAsync(const t_QueryOptions & options, const string & queryString, QueryCallback callback);

    [[nodiscard]] static bool registerQueryResultCallback(ulong queryId, QueryResultCallback callback);


  private:
    static void notifyAsyncCallbacks(const std::vector<ulong> & completedIds);

    static std::unordered_map<ulong, QueryCallback, OptHash<ulong>> asyncCallbacks_s;
    static Mutex callbackLock_s;

    friend class AlpineQueryMgr;
};
