/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <PersistenceStore.h>
#include <Log.h>
#include <sqlite3.h>


sqlite3 *       PersistenceStore::db_s          = nullptr;
std::mutex      PersistenceStore::mutex_s;
bool            PersistenceStore::initialized_s = false;
string          PersistenceStore::dbPath_s;


static const char * SCHEMA_SQL = R"(
    CREATE TABLE IF NOT EXISTS schema_version (
        version  INTEGER PRIMARY KEY
    );

    CREATE TABLE IF NOT EXISTS peers (
        peer_id   TEXT PRIMARY KEY,
        address   TEXT NOT NULL,
        port      INTEGER NOT NULL,
        metadata  TEXT,
        updated   INTEGER DEFAULT (strftime('%s','now'))
    );

    CREATE TABLE IF NOT EXISTS groups (
        group_id    TEXT PRIMARY KEY,
        group_name  TEXT NOT NULL,
        metadata    TEXT,
        updated     INTEGER DEFAULT (strftime('%s','now'))
    );

    CREATE TABLE IF NOT EXISTS modules (
        module_id    TEXT PRIMARY KEY,
        module_name  TEXT NOT NULL,
        version      TEXT,
        updated      INTEGER DEFAULT (strftime('%s','now'))
    );

    CREATE TABLE IF NOT EXISTS query_results (
        query_string  TEXT NOT NULL,
        results_json  TEXT NOT NULL,
        timestamp     INTEGER NOT NULL
    );

    CREATE INDEX IF NOT EXISTS idx_query_results_query
        ON query_results(query_string);
)";



bool
PersistenceStore::initialize (const string & dbPath)
{
    std::lock_guard<std::mutex> lock(mutex_s);

    if (initialized_s) {
        Log::Error("PersistenceStore already initialized"s);
        return false;
    }

    int rc = sqlite3_open(dbPath.c_str(), &db_s);
    if (rc != SQLITE_OK) {
        Log::Error("PersistenceStore: failed to open database: "s + sqlite3_errmsg(db_s));
        sqlite3_close(db_s);
        db_s = nullptr;
        return false;
    }

    // Enable WAL mode for better concurrent read performance
    execute("PRAGMA journal_mode=WAL");
    execute("PRAGMA foreign_keys=ON");

    dbPath_s = dbPath;

    if (!createSchema()) {
        Log::Error("PersistenceStore: schema creation failed"s);
        sqlite3_close(db_s);
        db_s = nullptr;
        return false;
    }

    initialized_s = true;
    Log::Info("PersistenceStore initialized: "s + dbPath);
    return true;
}



void
PersistenceStore::shutdown ()
{
    std::lock_guard<std::mutex> lock(mutex_s);

    if (db_s) {
        sqlite3_close(db_s);
        db_s = nullptr;
    }
    initialized_s = false;
    Log::Info("PersistenceStore shut down"s);
}



bool
PersistenceStore::isInitialized ()
{
    return initialized_s;
}



bool
PersistenceStore::storePeer (const string & peerId,
                             const string & address,
                             ushort         port,
                             const string & metadata)
{
    std::lock_guard<std::mutex> lock(mutex_s);
    if (!db_s) return false;

    const char * sql = "INSERT OR REPLACE INTO peers (peer_id, address, port, metadata, updated) "
                       "VALUES (?, ?, ?, ?, strftime('%s','now'))";

    sqlite3_stmt * stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_s, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, peerId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, address.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, port);
    sqlite3_bind_text(stmt, 4, metadata.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}



bool
PersistenceStore::removePeer (const string & peerId)
{
    std::lock_guard<std::mutex> lock(mutex_s);
    if (!db_s) return false;

    const char * sql = "DELETE FROM peers WHERE peer_id = ?";
    sqlite3_stmt * stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_s, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, peerId.c_str(), -1, SQLITE_TRANSIENT);
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}



bool
PersistenceStore::loadPeers (vector<std::tuple<string, string, ushort, string>> & peers)
{
    std::lock_guard<std::mutex> lock(mutex_s);
    if (!db_s) return false;

    const char * sql = "SELECT peer_id, address, port, metadata FROM peers ORDER BY updated DESC";
    sqlite3_stmt * stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_s, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) return false;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        auto id   = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
        auto addr = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
        auto port = static_cast<ushort>(sqlite3_column_int(stmt, 2));
        auto meta = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3));
        peers.emplace_back(id ? id : "", addr ? addr : "", port, meta ? meta : "");
    }

    sqlite3_finalize(stmt);
    return true;
}



bool
PersistenceStore::storeGroup (const string & groupId,
                              const string & groupName,
                              const string & metadata)
{
    std::lock_guard<std::mutex> lock(mutex_s);
    if (!db_s) return false;

    const char * sql = "INSERT OR REPLACE INTO groups (group_id, group_name, metadata, updated) "
                       "VALUES (?, ?, ?, strftime('%s','now'))";

    sqlite3_stmt * stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_s, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, groupId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, groupName.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, metadata.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}



bool
PersistenceStore::removeGroup (const string & groupId)
{
    std::lock_guard<std::mutex> lock(mutex_s);
    if (!db_s) return false;

    const char * sql = "DELETE FROM groups WHERE group_id = ?";
    sqlite3_stmt * stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_s, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, groupId.c_str(), -1, SQLITE_TRANSIENT);
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}



bool
PersistenceStore::loadGroups (vector<std::tuple<string, string, string>> & groups)
{
    std::lock_guard<std::mutex> lock(mutex_s);
    if (!db_s) return false;

    const char * sql = "SELECT group_id, group_name, metadata FROM groups ORDER BY updated DESC";
    sqlite3_stmt * stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_s, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) return false;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        auto id   = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
        auto name = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
        auto meta = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
        groups.emplace_back(id ? id : "", name ? name : "", meta ? meta : "");
    }

    sqlite3_finalize(stmt);
    return true;
}



bool
PersistenceStore::storeModule (const string & moduleId,
                               const string & moduleName,
                               const string & version)
{
    std::lock_guard<std::mutex> lock(mutex_s);
    if (!db_s) return false;

    const char * sql = "INSERT OR REPLACE INTO modules (module_id, module_name, version, updated) "
                       "VALUES (?, ?, ?, strftime('%s','now'))";

    sqlite3_stmt * stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_s, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, moduleId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, moduleName.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, version.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}



bool
PersistenceStore::removeModule (const string & moduleId)
{
    std::lock_guard<std::mutex> lock(mutex_s);
    if (!db_s) return false;

    const char * sql = "DELETE FROM modules WHERE module_id = ?";
    sqlite3_stmt * stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_s, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, moduleId.c_str(), -1, SQLITE_TRANSIENT);
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}



bool
PersistenceStore::loadModules (vector<std::tuple<string, string, string>> & modules)
{
    std::lock_guard<std::mutex> lock(mutex_s);
    if (!db_s) return false;

    const char * sql = "SELECT module_id, module_name, version FROM modules ORDER BY updated DESC";
    sqlite3_stmt * stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_s, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) return false;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        auto id   = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
        auto name = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
        auto ver  = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
        modules.emplace_back(id ? id : "", name ? name : "", ver ? ver : "");
    }

    sqlite3_finalize(stmt);
    return true;
}



bool
PersistenceStore::storeQueryResult (const string & queryString,
                                    const string & resultsJson,
                                    ulong          timestamp)
{
    std::lock_guard<std::mutex> lock(mutex_s);
    if (!db_s) return false;

    const char * sql = "INSERT INTO query_results (query_string, results_json, timestamp) "
                       "VALUES (?, ?, ?)";

    sqlite3_stmt * stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_s, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, queryString.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, resultsJson.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 3, static_cast<sqlite3_int64>(timestamp));

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return rc == SQLITE_DONE;
}



bool
PersistenceStore::loadQueryResults (const string & queryString,
                                    string &       resultsJson)
{
    std::lock_guard<std::mutex> lock(mutex_s);
    if (!db_s) return false;

    const char * sql = "SELECT results_json FROM query_results "
                       "WHERE query_string = ? ORDER BY timestamp DESC LIMIT 1";

    sqlite3_stmt * stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_s, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, queryString.c_str(), -1, SQLITE_TRANSIENT);

    bool found = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        auto text = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
        if (text) {
            resultsJson = text;
            found = true;
        }
    }

    sqlite3_finalize(stmt);
    return found;
}



bool
PersistenceStore::clearQueryResults ()
{
    std::lock_guard<std::mutex> lock(mutex_s);
    if (!db_s) return false;

    return sqlite3_exec(db_s, "DELETE FROM query_results", nullptr, nullptr, nullptr) == SQLITE_OK;
}



bool
PersistenceStore::execute (const string & sql)
{
    if (!db_s) return false;

    char * errMsg = nullptr;
    int rc = sqlite3_exec(db_s, sql.c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        if (errMsg) {
            Log::Error("PersistenceStore SQL error: "s + errMsg);
            sqlite3_free(errMsg);
        }
        return false;
    }
    return true;
}



bool
PersistenceStore::executeWithCallback (const string & sql,
                                       std::function<void(int, char**, char**)> callback)
{
    if (!db_s) return false;

    auto wrapper = [](void * ctx, int cols, char ** values, char ** names) -> int {
        auto & cb = *static_cast<std::function<void(int, char**, char**)> *>(ctx);
        cb(cols, values, names);
        return 0;
    };

    char * errMsg = nullptr;
    int rc = sqlite3_exec(db_s, sql.c_str(), wrapper, &callback, &errMsg);
    if (rc != SQLITE_OK) {
        if (errMsg) {
            Log::Error("PersistenceStore SQL error: "s + errMsg);
            sqlite3_free(errMsg);
        }
        return false;
    }
    return true;
}



bool
PersistenceStore::createSchema ()
{
    return execute(SCHEMA_SQL);
}



bool
PersistenceStore::migrateSchema ()
{
    // Future schema migrations go here
    return true;
}
