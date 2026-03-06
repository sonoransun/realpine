/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <StatusHandler.h>
#include <JsonWriter.h>
#include <AlpineStackInterface.h>
#include <DtcpStackInterface.h>
#include <ClusterCoordinator.h>

#ifdef ALPINE_FUSE_ENABLED
#include <AlpineFuse.h>
#endif

#ifdef ALPINE_TRACING_ENABLED
#include <Tracing.h>
#endif


std::chrono::steady_clock::time_point  StatusHandler::startTime_s;


void
StatusHandler::recordStartTime ()
{
    startTime_s = std::chrono::steady_clock::now();
}


void
StatusHandler::registerRoutes (HttpRouter & router)
{
    router.addRoute("GET", "/status", getStatus);
    router.addRoute("GET", "/status/health", getHealth);
}


HttpResponse
StatusHandler::getStatus (const HttpRequest & request,
                          const std::unordered_map<string, string> & params)
{
#ifdef ALPINE_TRACING_ENABLED
    ScopedSpan span("status.get"s);
#endif

    auto uptimeSeconds = static_cast<ulong>(
        std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::steady_clock::now() - startTime_s).count());

    // Peer count
    DtcpStackInterface::t_DtcpPeerIdList peerIds;
    DtcpStackInterface::getAllDtcpPeerIds(peerIds);
    ulong activePeerCount = peerIds.size();

    // Group info for query counts
    auto groupInfoResult = AlpineStackInterface::getDefaultGroupInfo2();
    AlpineStackInterface::t_GroupInfo groupInfo{};
    if (groupInfoResult)
        groupInfo = *groupInfoResult;

    // Cluster info
    auto clusterNodes = ClusterCoordinator::getClusterNodes();
    auto localNode    = ClusterCoordinator::getLocalNodeInfo();
    bool isolated     = ClusterCoordinator::isIsolated();

    JsonWriter writer;
    writer.beginObject();

    writer.key("status");
    writer.value("running"s);

    writer.key("version");
    writer.value("devel-00019"s);

    writer.key("apiVersion");
    writer.value("1.0"s);

    writer.key("uptimeSeconds");
    writer.value(uptimeSeconds);

    // Peers
    writer.key("activePeerCount");
    writer.value(activePeerCount);

    // Queries
    writer.key("totalQueries");
    writer.value(groupInfo.totalQueries);

    writer.key("totalResponses");
    writer.value(groupInfo.totalResponses);

    // Enabled services
    writer.key("services");
    writer.beginObject();

#ifdef ALPINE_FUSE_ENABLED
    writer.key("fuse");
    writer.value(AlpineFuse::isRunning());
#else
    writer.key("fuse");
    writer.value(false);
#endif

    writer.endObject();

    // Cluster
    writer.key("cluster");
    writer.beginObject();

    writer.key("nodeId");
    writer.value(localNode.nodeId);

    writer.key("nodeCount");
    writer.value(static_cast<ulong>(clusterNodes.size()));

    writer.key("region");
    writer.value(localNode.region);

    writer.key("isolated");
    writer.value(isolated);

    writer.endObject();

    writer.endObject();

    return HttpResponse::ok(writer.result());
}


HttpResponse
StatusHandler::getHealth (const HttpRequest & request,
                          const std::unordered_map<string, string> & params)
{
    bool healthy = true;

    // Check cluster isolation
    if (ClusterCoordinator::isIsolated())
        healthy = false;

    JsonWriter writer;
    writer.beginObject();
    writer.key("healthy");
    writer.value(healthy);
    writer.endObject();

    if (healthy)
        return HttpResponse::ok(writer.result());

    HttpResponse resp(503, "Service Unavailable");
    resp.setJsonBody(writer.result());
    return resp;
}
