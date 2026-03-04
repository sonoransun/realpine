/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AccessTracker.h>
#include <nlohmann/json.hpp>
#include <algorithm>
#include <iomanip>
#include <sstream>


std::unordered_map<ulong, AccessTracker::t_ResourceStats, OptHash<ulong>>
    AccessTracker::resourceStats_s;

std::unordered_map<ulong, AccessTracker::t_PeerStats, OptHash<ulong>>
    AccessTracker::peerStats_s;

std::unordered_map<string, AccessTracker::t_QueryTermStats, OptHash<string>>
    AccessTracker::queryTermStats_s;

ReadWriteSem  AccessTracker::dataLock_s;


void
AccessTracker::recordResourceAccess (ulong           resourceId,
                                     ulong           peerId,
                                     const string &  description,
                                     ulong           bytesRead)
{
    auto now = std::chrono::system_clock::now();

    dataLock_s.acquireWrite();

    // Update resource stats
    auto & rs          = resourceStats_s[resourceId];
    rs.resourceId      = resourceId;
    rs.peerId          = peerId;
    rs.description     = description;
    rs.accessCount++;
    rs.totalBytesRead += bytesRead;
    rs.lastAccessTime  = now;

    // Update peer stats
    auto & ps = peerStats_s[peerId];
    ps.peerId = peerId;
    ps.aggregateAccessCount++;
    ps.aggregateBytesRead += bytesRead;
    ps.lastAccessTime      = now;

    // Track distinct resources per peer
    if (rs.accessCount == 1)
        ps.distinctResourcesAccessed++;

    dataLock_s.releaseWrite();
}


void
AccessTracker::recordQueryAccess (const string &  queryTerm)
{
    auto now = std::chrono::system_clock::now();

    dataLock_s.acquireWrite();

    auto & qs        = queryTermStats_s[queryTerm];
    qs.queryTerm     = queryTerm;
    qs.accessCount++;
    qs.lastAccessTime = now;

    dataLock_s.releaseWrite();
}


bool
AccessTracker::getResourceStats (ulong             resourceId,
                                 t_ResourceStats & stats)
{
    dataLock_s.acquireRead();

    if (!resourceStats_s.contains(resourceId))
    {
        dataLock_s.releaseRead();
        return false;
    }

    stats = resourceStats_s[resourceId];
    dataLock_s.releaseRead();
    return true;
}


bool
AccessTracker::getPeerStats (ulong          peerId,
                             t_PeerStats &  stats)
{
    dataLock_s.acquireRead();

    if (!peerStats_s.contains(peerId))
    {
        dataLock_s.releaseRead();
        return false;
    }

    stats = peerStats_s[peerId];
    dataLock_s.releaseRead();
    return true;
}


void
AccessTracker::getMostAccessedResources (vector<t_ResourceStats> &  list,
                                         ulong                      limit)
{
    list.clear();

    dataLock_s.acquireRead();

    for (const auto & [_, stats] : resourceStats_s)
        list.push_back(stats);

    dataLock_s.releaseRead();

    std::sort(list.begin(), list.end(),
              [](const t_ResourceStats & a, const t_ResourceStats & b)
              { return a.accessCount > b.accessCount; });

    if (list.size() > limit)
        list.resize(limit);
}


void
AccessTracker::getMostRecentResources (vector<t_ResourceStats> &  list,
                                       ulong                      limit)
{
    list.clear();

    dataLock_s.acquireRead();

    for (const auto & [_, stats] : resourceStats_s)
        list.push_back(stats);

    dataLock_s.releaseRead();

    std::sort(list.begin(), list.end(),
              [](const t_ResourceStats & a, const t_ResourceStats & b)
              { return a.lastAccessTime > b.lastAccessTime; });

    if (list.size() > limit)
        list.resize(limit);
}


string
AccessTracker::serializeJson ()
{
    nlohmann::json root;

    dataLock_s.acquireRead();

    // Resources
    auto & resources = root["resources"];
    resources = nlohmann::json::array();

    for (const auto & [_, rs] : resourceStats_s)
    {
        resources.push_back({
            {"resourceId",    rs.resourceId},
            {"peerId",        rs.peerId},
            {"description",   rs.description},
            {"accessCount",   rs.accessCount},
            {"totalBytesRead", rs.totalBytesRead},
            {"lastAccessTime", std::chrono::system_clock::to_time_t(rs.lastAccessTime)}
        });
    }

    // Peers
    auto & peers = root["peers"];
    peers = nlohmann::json::array();

    for (const auto & [_, ps] : peerStats_s)
    {
        peers.push_back({
            {"peerId",                    ps.peerId},
            {"aggregateAccessCount",      ps.aggregateAccessCount},
            {"aggregateBytesRead",        ps.aggregateBytesRead},
            {"distinctResourcesAccessed", ps.distinctResourcesAccessed},
            {"lastAccessTime",            std::chrono::system_clock::to_time_t(ps.lastAccessTime)}
        });
    }

    // Query terms
    auto & queryTerms = root["queryTerms"];
    queryTerms = nlohmann::json::array();

    for (const auto & [_, qs] : queryTermStats_s)
    {
        queryTerms.push_back({
            {"queryTerm",      qs.queryTerm},
            {"accessCount",    qs.accessCount},
            {"lastAccessTime", std::chrono::system_clock::to_time_t(qs.lastAccessTime)}
        });
    }

    dataLock_s.releaseRead();

    return root.dump(2);
}


string
AccessTracker::serializeText ()
{
    std::ostringstream out;

    dataLock_s.acquireRead();

    out << "=== Resource Access Statistics ===\n";

    for (const auto & [_, rs] : resourceStats_s)
    {
        double mb = static_cast<double>(rs.totalBytesRead) / (1024.0 * 1024.0);
        out << "Resource " << rs.resourceId
            << " (peerId: " << rs.peerId << "): \""
            << rs.description << "\" — "
            << rs.accessCount << " accesses, "
            << std::fixed << std::setprecision(1) << mb << " MB read\n";
    }

    out << "\n=== Peer Access Statistics ===\n";

    for (const auto & [_, ps] : peerStats_s)
    {
        double mb = static_cast<double>(ps.aggregateBytesRead) / (1024.0 * 1024.0);
        out << "Peer " << ps.peerId << ": "
            << ps.aggregateAccessCount << " accesses, "
            << ps.distinctResourcesAccessed << " resources, "
            << std::fixed << std::setprecision(1) << mb << " MB read\n";
    }

    out << "\n=== Query Term Statistics ===\n";

    for (const auto & [_, qs] : queryTermStats_s)
    {
        out << "\"" << qs.queryTerm << "\": "
            << qs.accessCount << " accesses\n";
    }

    dataLock_s.releaseRead();

    return out.str();
}
