/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <HttpRouter.h>
#include <OptHash.h>
#include <ReadWriteSem.h>
#include <SysThread.h>
#include <UdpConnection.h>
#include <array>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <unordered_set>


class ClusterCoordinator
{
  public:
    // --- Lifecycle ---

    static bool initialize(ushort restPort, ushort beaconPort);

    static void shutdown();

    static void registerRoutes(HttpRouter & router);


    // --- Cluster membership ---

    struct NodeInfo
    {
        string nodeId;
        string host;
        ushort restPort{0};
        uint activeQueries{0};
        uint connectionCount{0};
        double cpuLoadEstimate{0.0};
        string region;
        double rttMs{0.0};  // measured round-trip time in milliseconds
        vector<string> capabilities;
        std::chrono::steady_clock::time_point lastHeartbeat;
    };

    static vector<NodeInfo> getClusterNodes();

    static NodeInfo getLocalNodeInfo();


    // --- Query deduplication ---

    static string hashQuery(const string & queryString, const string & groupName, ulong autoHaltLimit);

    // Returns true if the query is already being processed by another node.
    // If so, ownerHost/ownerPort are set to the node handling it.
    static bool isDuplicateQuery(const string & queryHash, string & ownerHost, ushort & ownerPort);

    // Register a locally-started query for cluster-wide dedup.
    static void registerActiveQuery(const string & queryHash, ulong queryId);

    // Remove a finished/cancelled query from dedup tracking.
    static void unregisterActiveQuery(const string & queryHash);


    // --- Load-aware routing ---

    // Returns true if this node should redirect to a less-loaded node.
    // Sets redirectUrl to the target if so.
    static bool shouldRedirect(string & redirectUrl);


    // --- Cluster health ---

    /// Returns true if this node is in isolated/split-brain mode.
    static bool isIsolated();


    // --- Federated result aggregation ---

    // Fetch results from a remote node for a query hash.
    // Returns JSON string of aggregated results, or empty on failure.
    static string fetchRemoteResults(const string & host, ushort port, ulong queryId);


  private:
    // --- Heartbeat thread ---

    class HeartbeatThread : public SysThread
    {
      public:
        void threadMain() override;
        bool stop();

      private:
        std::mutex cvMutex_;
        std::condition_variable cv_;
    };

    // --- Beacon listener thread ---

    class ListenerThread : public SysThread
    {
      public:
        bool initialize(ushort beaconPort);
        void threadMain() override;

      private:
        UdpConnection udpSocket_;
        ushort beaconPort_{0};
    };


    // --- REST route handlers ---

    static HttpResponse handleClusterStatus(const HttpRequest & request,
                                            const std::unordered_map<string, string> & params);

    static HttpResponse handleClusterQuery(const HttpRequest & request,
                                           const std::unordered_map<string, string> & params);

    static HttpResponse handleClusterHeartbeat(const HttpRequest & request,
                                               const std::unordered_map<string, string> & params);

    static HttpResponse handleClusterResults(const HttpRequest & request,
                                             const std::unordered_map<string, string> & params);


    // --- Internal helpers ---

    static string generateNodeId();
    static void evictStaleNodes();
    static void broadcastHeartbeat();
    static void gossipActiveQueries();
    static double estimateCpuLoad();
    static double measureRtt(const string & host, ushort port);
    static void checkSplitBrain();
    static int computeAdaptiveTimeoutSec();


    // --- Node identity ---

    static string nodeId_s;
    static string localHost_s;
    static string localRegion_s;
    static ushort restPort_s;
    static ushort beaconPort_s;

    // --- Split-brain / isolation ---

    static std::atomic<bool> isolated_s;


    // --- Cluster membership (nodeId -> NodeInfo) ---

    static std::unordered_map<string, NodeInfo> nodes_s;
    static ReadWriteSem nodesMutex_s;


    // --- Query dedup tracking (queryHash -> {nodeId, queryId}) ---

    struct DedupEntry
    {
        string nodeId;
        ulong queryId{0};
        std::chrono::steady_clock::time_point registeredAt;
        uint64_t wallClockMs{0};  // wall-clock timestamp for last-writer-wins merge
    };

    static constexpr size_t DEDUP_SHARD_COUNT = 8;

    struct t_DedupShard
    {
        ReadWriteSem mutex;
        std::unordered_map<string, DedupEntry> entries;
    };

    static std::array<t_DedupShard, DEDUP_SHARD_COUNT> dedupShards_s;

    static size_t dedupShardIndex(const string & key);

    static constexpr int DEDUP_WINDOW_SEC = 30;


    // --- Heartbeat configuration ---

    static constexpr int HEARTBEAT_INTERVAL_SEC = 10;
    static constexpr int NODE_TIMEOUT_MIN_SEC = 35;
    static constexpr int NODE_TIMEOUT_MAX_SEC = 120;
    static constexpr double SPLIT_BRAIN_THRESHOLD = 0.5;  // >50% unreachable
    static constexpr int SPLIT_BRAIN_MULTIPLIER = 2;      // 2x timeout before isolated


    // --- Threads ---

    static std::unique_ptr<HeartbeatThread> heartbeatThread_s;
    static std::unique_ptr<ListenerThread> listenerThread_s;

    static std::once_flag initFlag_s;

    static std::atomic<bool> initialized_s;
};
