/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>

struct sqlite3;
struct sqlite3_stmt;


class PersistenceStore
{
  public:
    static bool initialize(const string & dbPath);

    static void shutdown();

    static bool isInitialized();


    // Peer profiles
    static bool storePeer(const string & peerId, const string & address, ushort port, const string & metadata);

    static bool removePeer(const string & peerId);

    static bool loadPeers(vector<std::tuple<string, string, ushort, string>> & peers);


    // Group membership
    static bool storeGroup(const string & groupId, const string & groupName, const string & metadata);

    static bool removeGroup(const string & groupId);

    static bool loadGroups(vector<std::tuple<string, string, string>> & groups);


    // Module registrations
    static bool storeModule(const string & moduleId, const string & moduleName, const string & version);

    static bool removeModule(const string & moduleId);

    static bool loadModules(vector<std::tuple<string, string, string>> & modules);


    // Query results history
    static bool storeQueryResult(const string & queryString, const string & resultsJson, ulong timestamp);

    static bool loadQueryResults(const string & queryString, string & resultsJson);

    static bool clearQueryResults();


    // Generic execute
    static bool execute(const string & sql);

    static bool executeWithCallback(const string & sql, std::function<void(int, char **, char **)> callback);


  private:
    static bool createSchema();

    static bool migrateSchema();

    static sqlite3 * db_s;
    static std::mutex mutex_s;
    static bool initialized_s;
    static string dbPath_s;
};
