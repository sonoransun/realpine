/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <ReadWriteSem.h>
#include <OptHash.h>
#include <HttpRouter.h>
#include <SysThread.h>
#include <UdpConnection.h>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <unordered_set>
#include <chrono>
#include <atomic>
#include <functional>


class ClusterCoordinator
{
  public:

    // --- Lifecycle ---

    static bool  initialize (ushort restPort, ushort beaconPort);

    static void  shutdown ();

    static void  registerRoutes (HttpRouter & router);


    // --- Cluster membership ---

    struct NodeInfo {
        string   nodeId;
        string   host;
        ushort   restPort{0};
        uint     activeQueries{0};
        uint     connectionCount{0};
        double   cpuLoadEstimate{0.0};
        vector<string>  capabilities;
        std::chrono::steady_clock::time_point  lastHeartbeat;
    };

    static vector<NodeInfo>  getClusterNodes ();

    static NodeInfo  getLocalNodeInfo ();


    // --- Query deduplication ---

    static string  hashQuery (const string & queryString,
                              const string & groupName,
                              ulong          autoHaltLimit);

    // Returns true if the query is already being processed by another node.
    // If so, ownerHost/ownerPort are set to the node handling it.
    static bool  isDuplicateQuery (const string & queryHash,
                                   string &       ownerHost,
                                   ushort &       ownerPort);

    // Register a locally-started query for cluster-wide dedup.
    static void  registerActiveQuery (const string & queryHash,
                                      ulong          queryId);

    // Remove a finished/cancelled query from dedup tracking.
    static void  unregisterActiveQuery (const string & queryHash);


    // --- Load-aware routing ---

    // Returns true if this node should redirect to a less-loaded node.
    // Sets redirectUrl to the target if so.
    static bool  shouldRedirect (string & redirectUrl);


    // --- Federated result aggregation ---

    // Fetch results from a remote node for a query hash.
    // Returns JSON string of aggregated results, or empty on failure.
    static string  fetchRemoteResults (const string & host,
                                       ushort         port,
                                       ulong          queryId);


  private:

    // --- Heartbeat thread ---

    class HeartbeatThread : public SysThread
    {
      public:
        void threadMain () override;
    };

    // --- Beacon listener thread ---

    class ListenerThread : public SysThread
    {
      public:
        bool initialize (ushort beaconPort);
        void threadMain () override;

      private:
        UdpConnection  udpSocket_;
        ushort         beaconPort_{0};
    };


    // --- REST route handlers ---

    static HttpResponse  handleClusterStatus (const HttpRequest & request,
                                              const std::unordered_map<string, string> & params);

    static HttpResponse  handleClusterQuery (const HttpRequest & request,
                                             const std::unordered_map<string, string> & params);

    static HttpResponse  handleClusterHeartbeat (const HttpRequest & request,
                                                 const std::unordered_map<string, string> & params);

    static HttpResponse  handleClusterResults (const HttpRequest & request,
                                               const std::unordered_map<string, string> & params);


    // --- Internal helpers ---

    static string  generateNodeId ();
    static void    evictStaleNodes ();
    static void    broadcastHeartbeat ();
    static void    gossipActiveQueries ();
    static double  estimateCpuLoad ();


    // --- Node identity ---

    static string   nodeId_s;
    static string   localHost_s;
    static ushort   restPort_s;
    static ushort   beaconPort_s;


    // --- Cluster membership (nodeId -> NodeInfo) ---

    static std::unordered_map<string, NodeInfo>  nodes_s;
    static ReadWriteSem                          nodesMutex_s;


    // --- Query dedup tracking (queryHash -> {nodeId, queryId}) ---

    struct DedupEntry {
        string   nodeId;
        ulong    queryId{0};
        std::chrono::steady_clock::time_point  registeredAt;
    };

    static std::unordered_map<string, DedupEntry>  dedupIndex_s;
    static ReadWriteSem                             dedupMutex_s;

    static constexpr int  DEDUP_WINDOW_SEC = 30;


    // --- Heartbeat configuration ---

    static constexpr int  HEARTBEAT_INTERVAL_SEC = 10;
    static constexpr int  NODE_TIMEOUT_SEC       = 35;
    static constexpr int  SLEEP_INCREMENT_MS     = 1000;


    // --- Threads ---

    static HeartbeatThread *  heartbeatThread_s;
    static ListenerThread *   listenerThread_s;

    static std::atomic<bool>  initialized_s;

};
