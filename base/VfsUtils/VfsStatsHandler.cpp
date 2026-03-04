/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <VfsStatsHandler.h>
#include <AccessTracker.h>
#include <AlpineFuse.h>
#include <nlohmann/json.hpp>


void
VfsStatsHandler::registerRoutes (HttpRouter & router)
{
    router.addRoute("GET", "/vfs/stats",          getStats);
    router.addRoute("GET", "/vfs/stats/popular",  getPopular);
    router.addRoute("GET", "/vfs/stats/recent",   getRecent);
    router.addRoute("GET", "/vfs/stats/peer/{id}", getPeerStats);
    router.addRoute("GET", "/vfs/status",         getVfsStatus);
}


HttpResponse
VfsStatsHandler::getStats (const HttpRequest & request,
                           const std::unordered_map<string, string> & params)
{
    return HttpResponse::ok(AccessTracker::serializeJson());
}


HttpResponse
VfsStatsHandler::getPopular (const HttpRequest & request,
                             const std::unordered_map<string, string> & params)
{
    vector<AccessTracker::t_ResourceStats> resources;
    AccessTracker::getMostAccessedResources(resources, 50);

    nlohmann::json arr = nlohmann::json::array();

    for (const auto & r : resources) {
        nlohmann::json obj;
        obj["resourceId"]   = r.resourceId;
        obj["peerId"]       = r.peerId;
        obj["description"]  = r.description;
        obj["accessCount"]  = r.accessCount;
        obj["totalBytesRead"] = r.totalBytesRead;
        arr.push_back(obj);
    }

    nlohmann::json result;
    result["popular"] = arr;

    return HttpResponse::ok(result.dump());
}


HttpResponse
VfsStatsHandler::getRecent (const HttpRequest & request,
                            const std::unordered_map<string, string> & params)
{
    vector<AccessTracker::t_ResourceStats> resources;
    AccessTracker::getMostRecentResources(resources, 50);

    nlohmann::json arr = nlohmann::json::array();

    for (const auto & r : resources) {
        nlohmann::json obj;
        obj["resourceId"]   = r.resourceId;
        obj["peerId"]       = r.peerId;
        obj["description"]  = r.description;
        obj["accessCount"]  = r.accessCount;
        obj["totalBytesRead"] = r.totalBytesRead;
        arr.push_back(obj);
    }

    nlohmann::json result;
    result["recent"] = arr;

    return HttpResponse::ok(result.dump());
}


HttpResponse
VfsStatsHandler::getPeerStats (const HttpRequest & request,
                               const std::unordered_map<string, string> & params)
{
    auto it = params.find("id");

    if (it == params.end())
        return HttpResponse::badRequest("Missing peer ID");

    ulong peerId = std::stoul(it->second);

    AccessTracker::t_PeerStats stats{};

    if (!AccessTracker::getPeerStats(peerId, stats))
        return HttpResponse::notFound();

    nlohmann::json obj;
    obj["peerId"]                   = stats.peerId;
    obj["aggregateAccessCount"]     = stats.aggregateAccessCount;
    obj["aggregateBytesRead"]       = stats.aggregateBytesRead;
    obj["distinctResourcesAccessed"] = stats.distinctResourcesAccessed;

    return HttpResponse::ok(obj.dump());
}


HttpResponse
VfsStatsHandler::getVfsStatus (const HttpRequest & request,
                               const std::unordered_map<string, string> & params)
{
    nlohmann::json obj;
    obj["mounted"]    = AlpineFuse::isRunning();
    obj["mountPoint"] = AlpineFuse::getMountPoint();

    return HttpResponse::ok(obj.dump());
}
