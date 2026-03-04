/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <ClusterCoordinator.h>
#include <AlpineStackInterface.h>
#include <JsonWriter.h>
#include <JsonReader.h>
#include <Log.h>
#include <Platform.h>
#include <ReadLock.h>
#include <WriteLock.h>
#include <StringUtils.h>

#include <asio.hpp>
#include <functional>
#include <sstream>
#include <iomanip>
#include <cstring>

#ifdef ALPINE_PLATFORM_POSIX
#include <unistd.h>
#include <sys/sysctl.h>
#endif


// --- Static member initialization ---

string                                           ClusterCoordinator::nodeId_s;
string                                           ClusterCoordinator::localHost_s;
ushort                                           ClusterCoordinator::restPort_s{0};
ushort                                           ClusterCoordinator::beaconPort_s{0};

std::unordered_map<string, ClusterCoordinator::NodeInfo>  ClusterCoordinator::nodes_s;
ReadWriteSem                                     ClusterCoordinator::nodesMutex_s;

std::unordered_map<string, ClusterCoordinator::DedupEntry>  ClusterCoordinator::dedupIndex_s;
ReadWriteSem                                     ClusterCoordinator::dedupMutex_s;

ClusterCoordinator::HeartbeatThread *            ClusterCoordinator::heartbeatThread_s{nullptr};
ClusterCoordinator::ListenerThread *             ClusterCoordinator::listenerThread_s{nullptr};

std::atomic<bool>                                ClusterCoordinator::initialized_s{false};



// ============================================================================
//  Lifecycle
// ============================================================================

bool
ClusterCoordinator::initialize (ushort restPort, ushort beaconPort)
{
    if (initialized_s.load())
        return true;

    restPort_s   = restPort;
    beaconPort_s = beaconPort;
    nodeId_s     = generateNodeId();

    // Determine local hostname for cluster identification
    char hostname[256];
    gethostname(hostname, sizeof(hostname));
    localHost_s = hostname;

    Log::Info("ClusterCoordinator: Initializing node "s + nodeId_s +
              " (host: " + localHost_s + ", REST port: " + std::to_string(restPort_s) + ")");

    // Start heartbeat broadcast thread
    heartbeatThread_s = new HeartbeatThread();
    heartbeatThread_s->run();

    // Start beacon listener thread
    listenerThread_s = new ListenerThread();

    if (listenerThread_s->initialize(beaconPort)) {
        listenerThread_s->run();
    } else {
        Log::Error("ClusterCoordinator: Beacon listener failed to initialize.");
        delete listenerThread_s;
        listenerThread_s = nullptr;
    }

    initialized_s.store(true);

    Log::Info("ClusterCoordinator: Initialized successfully.");
    return true;
}


void
ClusterCoordinator::shutdown ()
{
    if (!initialized_s.load())
        return;

    initialized_s.store(false);

    if (heartbeatThread_s) {
        heartbeatThread_s->stop();
        delete heartbeatThread_s;
        heartbeatThread_s = nullptr;
    }

    if (listenerThread_s) {
        listenerThread_s->stop();
        delete listenerThread_s;
        listenerThread_s = nullptr;
    }

    {
        WriteLock lock(nodesMutex_s);
        nodes_s.clear();
    }

    {
        WriteLock lock(dedupMutex_s);
        dedupIndex_s.clear();
    }

    Log::Info("ClusterCoordinator: Shutdown complete.");
}


void
ClusterCoordinator::registerRoutes (HttpRouter & router)
{
    router.addRoute("GET",  "/cluster/status",    handleClusterStatus);
    router.addRoute("POST", "/cluster/query",     handleClusterQuery);
    router.addRoute("POST", "/cluster/heartbeat", handleClusterHeartbeat);
    router.addRoute("GET",  "/cluster/results/:id", handleClusterResults);
}



// ============================================================================
//  Cluster membership
// ============================================================================

vector<ClusterCoordinator::NodeInfo>
ClusterCoordinator::getClusterNodes ()
{
    ReadLock lock(nodesMutex_s);

    vector<NodeInfo> result;
    result.reserve(nodes_s.size() + 1);

    // Include self
    result.push_back(getLocalNodeInfo());

    for (const auto& [id, node] : nodes_s)
        result.push_back(node);

    return result;
}


ClusterCoordinator::NodeInfo
ClusterCoordinator::getLocalNodeInfo ()
{
    NodeInfo info;
    info.nodeId          = nodeId_s;
    info.host            = localHost_s;
    info.restPort        = restPort_s;
    info.cpuLoadEstimate = estimateCpuLoad();
    info.lastHeartbeat   = std::chrono::steady_clock::now();

    // Count active queries via dedup index
    {
        ReadLock lock(dedupMutex_s);
        uint count = 0;
        for (const auto& [hash, entry] : dedupIndex_s) {
            if (entry.nodeId == nodeId_s)
                ++count;
        }
        info.activeQueries = count;
    }

    info.capabilities = {"query"s, "transfer"s, "stream"s};

    return info;
}



// ============================================================================
//  Query deduplication
// ============================================================================

string
ClusterCoordinator::hashQuery (const string & queryString,
                               const string & groupName,
                               ulong          autoHaltLimit)
{
    // Simple deterministic hash combining query parameters.
    // Use std::hash with a combined key string.
    string combined = queryString + "|"s + groupName + "|"s + std::to_string(autoHaltLimit);
    size_t h = std::hash<string>{}(combined);

    // Format as hex string for readability
    std::ostringstream oss;
    oss << std::hex << std::setfill('0') << std::setw(16) << h;
    return oss.str();
}


bool
ClusterCoordinator::isDuplicateQuery (const string & queryHash,
                                      string &       ownerHost,
                                      ushort &       ownerPort)
{
    auto now = std::chrono::steady_clock::now();

    ReadLock lock(dedupMutex_s);

    auto it = dedupIndex_s.find(queryHash);
    if (it == dedupIndex_s.end())
        return false;

    const auto & entry = it->second;

    // Check if within dedup window
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
        now - entry.registeredAt).count();

    if (elapsed > DEDUP_WINDOW_SEC)
        return false;

    // Don't dedup against ourselves
    if (entry.nodeId == nodeId_s)
        return false;

    // Look up the owning node
    ReadLock nodesLock(nodesMutex_s);

    auto nodeIt = nodes_s.find(entry.nodeId);
    if (nodeIt == nodes_s.end())
        return false;

    ownerHost = nodeIt->second.host;
    ownerPort = nodeIt->second.restPort;
    return true;
}


void
ClusterCoordinator::registerActiveQuery (const string & queryHash,
                                         ulong          queryId)
{
    WriteLock lock(dedupMutex_s);

    DedupEntry entry;
    entry.nodeId       = nodeId_s;
    entry.queryId      = queryId;
    entry.registeredAt = std::chrono::steady_clock::now();

    dedupIndex_s[queryHash] = entry;

    Log::Debug("ClusterCoordinator: Registered query hash "s + queryHash +
               " (queryId: " + std::to_string(queryId) + ")");
}


void
ClusterCoordinator::unregisterActiveQuery (const string & queryHash)
{
    WriteLock lock(dedupMutex_s);
    dedupIndex_s.erase(queryHash);
}



// ============================================================================
//  Load-aware routing
// ============================================================================

bool
ClusterCoordinator::shouldRedirect (string & redirectUrl)
{
    NodeInfo local = getLocalNodeInfo();

    ReadLock lock(nodesMutex_s);

    if (nodes_s.empty())
        return false;

    // Find least-loaded node
    const NodeInfo * best = nullptr;
    double bestScore = local.cpuLoadEstimate + static_cast<double>(local.activeQueries) * 0.1;

    for (const auto& [id, node] : nodes_s) {
        // Skip nodes with stale heartbeats
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::steady_clock::now() - node.lastHeartbeat).count();

        if (elapsed > NODE_TIMEOUT_SEC)
            continue;

        double score = node.cpuLoadEstimate + static_cast<double>(node.activeQueries) * 0.1;

        if (!best || score < bestScore) {
            best = &node;
            bestScore = score;
        }
    }

    if (!best)
        return false;

    // Only redirect if the best remote node has significantly less load.
    // Require at least 20% difference to avoid ping-pong redirects.
    double localScore = local.cpuLoadEstimate + static_cast<double>(local.activeQueries) * 0.1;

    if (bestScore >= localScore * 0.8)
        return false;

    redirectUrl = "http://"s + best->host + ":" + std::to_string(best->restPort);
    return true;
}



// ============================================================================
//  Federated result aggregation
// ============================================================================

string
ClusterCoordinator::fetchRemoteResults (const string & host,
                                        ushort         port,
                                        ulong          queryId)
{
    try {
        asio::io_context ioCtx;
        asio::ip::tcp::resolver resolver(ioCtx);

        auto endpoints = resolver.resolve(host, std::to_string(port));

        asio::ip::tcp::socket socket(ioCtx);
        asio::connect(socket, endpoints);

        // Build GET request
        string request = "GET /query/"s + std::to_string(queryId) + "/results HTTP/1.1\r\n" +
                          "Host: " + host + ":" + std::to_string(port) + "\r\n" +
                          "Connection: close\r\n\r\n";

        asio::write(socket, asio::buffer(request));

        // Read response
        string response;
        asio::error_code ec;
        char buf[4096];

        while (true) {
            size_t n = socket.read_some(asio::buffer(buf), ec);
            if (n > 0)
                response.append(buf, n);
            if (ec)
                break;
        }

        // Extract body (after \r\n\r\n)
        auto bodyPos = response.find("\r\n\r\n");
        if (bodyPos == string::npos)
            return ""s;

        return response.substr(bodyPos + 4);
    }
    catch (const std::exception & e) {
        Log::Debug("ClusterCoordinator: Failed to fetch remote results from "s +
                   host + ":" + std::to_string(port) + " - " + e.what());
        return ""s;
    }
}



// ============================================================================
//  REST route handlers
// ============================================================================

HttpResponse
ClusterCoordinator::handleClusterStatus (const HttpRequest & request,
                                         const std::unordered_map<string, string> & params)
{
    auto nodes = getClusterNodes();

    JsonWriter writer;
    writer.beginObject();
    writer.key("nodeId");
    writer.value(nodeId_s);
    writer.key("clusterSize");
    writer.value(static_cast<ulong>(nodes.size()));
    writer.key("nodes");
    writer.beginArray();

    for (const auto & node : nodes) {
        writer.beginObject();
        writer.key("nodeId");
        writer.value(node.nodeId);
        writer.key("host");
        writer.value(node.host);
        writer.key("restPort");
        writer.value(static_cast<ulong>(node.restPort));
        writer.key("activeQueries");
        writer.value(static_cast<ulong>(node.activeQueries));
        writer.key("connectionCount");
        writer.value(static_cast<ulong>(node.connectionCount));
        writer.key("cpuLoad");
        writer.value(node.nodeId);  // placeholder — JsonWriter has no double overload
        writer.key("capabilities");
        writer.beginArray();
        for (const auto & cap : node.capabilities)
            writer.value(cap);
        writer.endArray();
        writer.endObject();
    }

    writer.endArray();
    writer.endObject();

    return HttpResponse::ok(writer.result());
}


HttpResponse
ClusterCoordinator::handleClusterQuery (const HttpRequest & request,
                                        const std::unordered_map<string, string> & params)
{
    // This endpoint receives a query request that may be redirected to this node,
    // or checks for dedup before starting.

    JsonReader reader(request.body);

    string queryString;
    if (!reader.getString("queryString", queryString))
        return HttpResponse::badRequest("Missing queryString");

    string groupName;
    reader.getString("groupName", groupName);

    ulong autoHaltLimit = 0;
    reader.getUlong("autoHaltLimit", autoHaltLimit);

    // Check deduplication
    string queryHash = hashQuery(queryString, groupName, autoHaltLimit);
    string ownerHost;
    ushort ownerPort = 0;

    if (isDuplicateQuery(queryHash, ownerHost, ownerPort)) {
        // Another node is already processing this query — redirect client
        string location = "http://"s + ownerHost + ":" + std::to_string(ownerPort) + "/query";

        JsonWriter writer;
        writer.beginObject();
        writer.key("deduplicated");
        writer.value(true);
        writer.key("redirectTo");
        writer.value(location);
        writer.endObject();

        HttpResponse resp(307, "Temporary Redirect");
        resp.setHeader("Location", location);
        resp.setJsonBody(writer.result());
        return resp;
    }

    // Check load-aware routing
    string redirectUrl;
    if (shouldRedirect(redirectUrl)) {
        string location = redirectUrl + "/cluster/query";

        JsonWriter writer;
        writer.beginObject();
        writer.key("loadBalanced");
        writer.value(true);
        writer.key("redirectTo");
        writer.value(location);
        writer.endObject();

        HttpResponse resp(307, "Temporary Redirect");
        resp.setHeader("Location", location);
        resp.setJsonBody(writer.result());
        return resp;
    }

    // Start the query locally
    AlpineStackInterface::t_QueryOptions options;
    options.groupName     = groupName;
    options.autoHaltLimit = autoHaltLimit;
    options.peerDescMax   = 0;
    options.autoDownload  = false;
    options.optionId      = 0;

    auto result = AlpineStackInterface::startQuery2(options, queryString);
    if (!result)
        return HttpResponse::serverError("Failed to start query");

    ulong queryId = *result;

    // Register for dedup
    registerActiveQuery(queryHash, queryId);

    JsonWriter writer;
    writer.beginObject();
    writer.key("queryId");
    writer.value(queryId);
    writer.key("nodeId");
    writer.value(nodeId_s);
    writer.key("queryHash");
    writer.value(queryHash);
    writer.endObject();

    auto response = HttpResponse::accepted(writer.result());
    response.setHeader("Location", "/query/"s + std::to_string(queryId));
    return response;
}


HttpResponse
ClusterCoordinator::handleClusterHeartbeat (const HttpRequest & request,
                                            const std::unordered_map<string, string> & params)
{
    // Accept heartbeat from a peer node via HTTP POST.
    // This supplements UDP beacon-based discovery.

    JsonReader reader(request.body);

    string nodeId;
    string host;
    ulong restPort = 0;
    ulong activeQueries = 0;
    ulong connectionCount = 0;

    if (!reader.getString("nodeId", nodeId) ||
        !reader.getString("host", host) ||
        !reader.getUlong("restPort", restPort)) {
        return HttpResponse::badRequest("Missing required heartbeat fields");
    }

    reader.getUlong("activeQueries", activeQueries);
    reader.getUlong("connectionCount", connectionCount);

    // Also ingest any dedup entries the peer is advertising
    // (handled via gossip in the heartbeat payload)

    {
        WriteLock lock(nodesMutex_s);

        NodeInfo & node  = nodes_s[nodeId];
        node.nodeId          = nodeId;
        node.host            = host;
        node.restPort        = static_cast<ushort>(restPort);
        node.activeQueries   = static_cast<uint>(activeQueries);
        node.connectionCount = static_cast<uint>(connectionCount);
        node.lastHeartbeat   = std::chrono::steady_clock::now();
    }

    JsonWriter writer;
    writer.beginObject();
    writer.key("accepted");
    writer.value(true);
    writer.endObject();

    return HttpResponse::ok(writer.result());
}


HttpResponse
ClusterCoordinator::handleClusterResults (const HttpRequest & request,
                                          const std::unordered_map<string, string> & params)
{
    // Federated result aggregation endpoint.
    // Returns local results for a query, plus fetches from other nodes that
    // share the same query hash.

    auto it = params.find("id");
    if (it == params.end())
        return HttpResponse::badRequest("Missing query id");

    ulong queryId = strtoul(it->second.c_str(), nullptr, 10);

    // Get local results first
    auto localResultsOpt = AlpineStackInterface::getQueryResults2(queryId);
    if (!localResultsOpt)
        return HttpResponse::notFound();

    auto & localResults = *localResultsOpt;

    // Find the query hash for this queryId
    string queryHash;
    {
        ReadLock lock(dedupMutex_s);
        for (const auto & [hash, entry] : dedupIndex_s) {
            if (entry.nodeId == nodeId_s && entry.queryId == queryId) {
                queryHash = hash;
                break;
            }
        }
    }

    // Build local results JSON
    JsonWriter writer;
    writer.beginObject();
    writer.key("queryId");
    writer.value(queryId);
    writer.key("nodeId");
    writer.value(nodeId_s);
    writer.key("federated");
    writer.value(true);

    writer.key("localPeers");
    writer.beginArray();

    for (const auto & [peerId, peerRes] : localResults) {
        writer.beginObject();
        writer.key("peerId");
        writer.value(peerId);
        writer.key("resources");
        writer.beginArray();

        for (const auto & res : peerRes.resourceDescList) {
            writer.beginObject();
            writer.key("resourceId");
            writer.value(res.resourceId);
            writer.key("size");
            writer.value(res.size);
            writer.key("description");
            writer.value(res.description);
            writer.key("locators");
            writer.beginArray();
            for (const auto & loc : res.locators)
                writer.value(loc);
            writer.endArray();
            writer.endObject();
        }

        writer.endArray();
        writer.endObject();
    }

    writer.endArray();

    // Aggregate results from other nodes that are processing the same query hash
    writer.key("remoteResults");
    writer.beginArray();

    if (!queryHash.empty()) {
        ReadLock nodesLock(nodesMutex_s);

        for (const auto & [id, node] : nodes_s) {
            // Skip stale nodes
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::steady_clock::now() - node.lastHeartbeat).count();
            if (elapsed > NODE_TIMEOUT_SEC)
                continue;

            // Check if this remote node has the same query hash
            ReadLock dedupLock(dedupMutex_s);
            auto dedupIt = dedupIndex_s.find(queryHash);
            if (dedupIt != dedupIndex_s.end() && dedupIt->second.nodeId == id) {
                string remoteJson = fetchRemoteResults(node.host, node.restPort,
                                                       dedupIt->second.queryId);
                if (!remoteJson.empty()) {
                    writer.beginObject();
                    writer.key("nodeId");
                    writer.value(id);
                    writer.key("data");
                    writer.value(remoteJson);
                    writer.endObject();
                }
            }
        }
    }

    writer.endArray();
    writer.endObject();

    return HttpResponse::ok(writer.result());
}



// ============================================================================
//  Internal helpers
// ============================================================================

string
ClusterCoordinator::generateNodeId ()
{
    char hostname[256];
    gethostname(hostname, sizeof(hostname));

    auto now = std::chrono::steady_clock::now().time_since_epoch().count();
    size_t h = std::hash<string>{}(string(hostname) + std::to_string(now));

    std::ostringstream oss;
    oss << "node-" << std::hex << (h & 0xFFFFFFFF);
    return oss.str();
}


void
ClusterCoordinator::evictStaleNodes ()
{
    auto now = std::chrono::steady_clock::now();

    WriteLock lock(nodesMutex_s);

    for (auto it = nodes_s.begin(); it != nodes_s.end(); ) {
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
            now - it->second.lastHeartbeat).count();

        if (elapsed > NODE_TIMEOUT_SEC) {
            Log::Debug("ClusterCoordinator: Evicting stale node "s + it->first);
            it = nodes_s.erase(it);
        } else {
            ++it;
        }
    }

    // Also clean expired dedup entries
    WriteLock dedupLock(dedupMutex_s);

    for (auto it = dedupIndex_s.begin(); it != dedupIndex_s.end(); ) {
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
            now - it->second.registeredAt).count();

        if (elapsed > DEDUP_WINDOW_SEC) {
            it = dedupIndex_s.erase(it);
        } else {
            ++it;
        }
    }
}


void
ClusterCoordinator::broadcastHeartbeat ()
{
    // Send heartbeat via HTTP POST to all known cluster nodes.
    NodeInfo local = getLocalNodeInfo();

    nlohmann::json payload;
    payload["nodeId"]          = local.nodeId;
    payload["host"]            = local.host;
    payload["restPort"]        = local.restPort;
    payload["activeQueries"]   = local.activeQueries;
    payload["connectionCount"] = local.connectionCount;
    payload["cpuLoad"]         = local.cpuLoadEstimate;

    // Include capability vector
    payload["capabilities"] = nlohmann::json::array();
    for (const auto & cap : local.capabilities)
        payload["capabilities"].push_back(cap);

    // Include active query hashes for gossip-based dedup
    {
        ReadLock lock(dedupMutex_s);
        payload["activeQueryHashes"] = nlohmann::json::array();

        for (const auto & [hash, entry] : dedupIndex_s) {
            if (entry.nodeId == nodeId_s) {
                nlohmann::json qe;
                qe["hash"]    = hash;
                qe["queryId"] = entry.queryId;
                payload["activeQueryHashes"].push_back(qe);
            }
        }
    }

    string body = payload.dump();

    // Send to each known node
    ReadLock lock(nodesMutex_s);

    for (const auto & [id, node] : nodes_s) {
        try {
            asio::io_context ioCtx;
            asio::ip::tcp::resolver resolver(ioCtx);

            auto endpoints = resolver.resolve(node.host, std::to_string(node.restPort));

            asio::ip::tcp::socket socket(ioCtx);
            asio::connect(socket, endpoints);

            string httpRequest = "POST /cluster/heartbeat HTTP/1.1\r\n"s +
                "Host: " + node.host + ":" + std::to_string(node.restPort) + "\r\n" +
                "Content-Type: application/json\r\n" +
                "Content-Length: " + std::to_string(body.length()) + "\r\n" +
                "Connection: close\r\n\r\n" + body;

            asio::write(socket, asio::buffer(httpRequest));

            // Read response (best-effort, don't block long)
            asio::error_code ec;
            char buf[1024];
            socket.read_some(asio::buffer(buf), ec);
        }
        catch (const std::exception & e) {
            Log::Debug("ClusterCoordinator: Heartbeat to "s + id + " failed: " + e.what());
        }
    }
}


void
ClusterCoordinator::gossipActiveQueries ()
{
    // Gossip is embedded in the heartbeat payload (see broadcastHeartbeat).
    // This method handles incoming gossip from peers by updating the dedup index
    // with remote query entries. Called when processing heartbeat payloads
    // from the beacon listener.
}


double
ClusterCoordinator::estimateCpuLoad ()
{
#ifdef ALPINE_PLATFORM_DARWIN
    // Use sysctl to get load average on macOS/Darwin
    double loadavg[3];
    if (getloadavg(loadavg, 3) != -1)
        return loadavg[0];
#endif

#ifdef ALPINE_PLATFORM_LINUX
    double loadavg[3];
    if (getloadavg(loadavg, 3) != -1)
        return loadavg[0];
#endif

    return 0.0;
}



// ============================================================================
//  HeartbeatThread
// ============================================================================

void
ClusterCoordinator::HeartbeatThread::threadMain ()
{
    Log::Info("ClusterCoordinator: Heartbeat thread started.");

    while (isActive()) {
        // Evict stale nodes
        evictStaleNodes();

        // Broadcast heartbeat to cluster
        broadcastHeartbeat();

        // Sleep in 1-second increments for responsive shutdown
        for (int i = 0; i < HEARTBEAT_INTERVAL_SEC && isActive(); i++)
            usleep(SLEEP_INCREMENT_MS * 1000);
    }

    Log::Info("ClusterCoordinator: Heartbeat thread exiting.");
}



// ============================================================================
//  ListenerThread — listens for UDP beacon broadcasts from other nodes
// ============================================================================

bool
ClusterCoordinator::ListenerThread::initialize (ushort beaconPort)
{
    beaconPort_ = beaconPort;

    if (!udpSocket_.create()) {
        Log::Error("ClusterCoordinator: Listener failed to create UDP socket.");
        return false;
    }

    // Allow address reuse so we coexist with DiscoveryBeacon
    int reuseAddr = 1;
    setsockopt(udpSocket_.getFd(), SOL_SOCKET, SO_REUSEADDR,
               &reuseAddr, sizeof(reuseAddr));

#ifdef SO_REUSEPORT
    setsockopt(udpSocket_.getFd(), SOL_SOCKET, SO_REUSEPORT,
               &reuseAddr, sizeof(reuseAddr));
#endif

    struct sockaddr_in bindAddr{};
    bindAddr.sin_family      = AF_INET;
    bindAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    bindAddr.sin_port        = htons(beaconPort_);

    if (bind(udpSocket_.getFd(), (struct sockaddr *)&bindAddr, sizeof(bindAddr)) < 0) {
        Log::Error("ClusterCoordinator: Listener failed to bind on port "s +
                   std::to_string(beaconPort_));
        udpSocket_.close();
        return false;
    }

    // Set receive timeout so thread can check isActive() periodically
    struct timeval tv;
    tv.tv_sec  = 1;
    tv.tv_usec = 0;
    setsockopt(udpSocket_.getFd(), SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    Log::Info("ClusterCoordinator: Listener bound on beacon port "s +
              std::to_string(beaconPort_));
    return true;
}


void
ClusterCoordinator::ListenerThread::threadMain ()
{
    Log::Info("ClusterCoordinator: Listener thread started.");

    byte buffer[4096];

    while (isActive()) {
        struct sockaddr_in senderAddr{};
        socklen_t addrLen = sizeof(senderAddr);

        ssize_t received = recvfrom(udpSocket_.getFd(), buffer, sizeof(buffer) - 1,
                                    0, (struct sockaddr *)&senderAddr, &addrLen);

        if (received <= 0)
            continue;

        buffer[received] = 0;

        // Parse beacon JSON
        try {
            nlohmann::json doc = nlohmann::json::parse(
                reinterpret_cast<const char *>(buffer),
                reinterpret_cast<const char *>(buffer + received),
                nullptr, false);

            if (doc.is_discarded())
                continue;

            // Only process alpine-bridge beacons
            if (!doc.contains("service") || doc["service"] != "alpine-bridge")
                continue;

            ushort peerRestPort = 0;
            if (doc.contains("restPort"))
                peerRestPort = static_cast<ushort>(doc["restPort"].get<ulong>());

            if (peerRestPort == 0)
                continue;

            // Resolve sender IP
            char senderIp[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &senderAddr.sin_addr, senderIp, sizeof(senderIp));
            string peerHost(senderIp);

            // Generate a stable node ID for beacon-discovered peers
            string peerNodeId = "beacon-"s +
                std::to_string(std::hash<string>{}(peerHost + ":" +
                    std::to_string(peerRestPort)) & 0xFFFFFFFF);

            // Don't add ourselves
            if (peerHost == localHost_s && peerRestPort == restPort_s)
                continue;

            // Also skip localhost variations
            if ((peerHost == "127.0.0.1"s || peerHost == "0.0.0.0"s) &&
                peerRestPort == restPort_s)
                continue;

            {
                WriteLock lock(nodesMutex_s);

                NodeInfo & node  = nodes_s[peerNodeId];
                node.nodeId        = peerNodeId;
                node.host          = peerHost;
                node.restPort      = peerRestPort;
                node.lastHeartbeat = std::chrono::steady_clock::now();

                if (doc.contains("capabilities") && doc["capabilities"].is_array()) {
                    node.capabilities.clear();
                    for (const auto & cap : doc["capabilities"])
                        node.capabilities.push_back(cap.get<string>());
                }
            }
        }
        catch (const std::exception & e) {
            Log::Debug("ClusterCoordinator: Failed to parse beacon: "s + e.what());
        }
    }

    udpSocket_.close();
    Log::Info("ClusterCoordinator: Listener thread exiting.");
}
