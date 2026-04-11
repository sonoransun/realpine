/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AccessTracker.h>
#include <AlpineFuse.h>
#include <AlpinePeerProfileIndex.h>
#include <AlpineRatingEngine.h>
#include <AlpineStackInterface.h>
#include <DtcpStackInterface.h>
#include <FuseOps.h>
#include <Log.h>
#include <QueryCache.h>

#include <fuse.h>

#include <algorithm>
#include <cerrno>
#include <chrono>
#include <cstring>
#include <sstream>


// ---------------------------------------------------------------------------
// operations()
// ---------------------------------------------------------------------------

const fuse_operations &
FuseOps::operations()
{
    static fuse_operations ops{};
    ops.getattr = fuseGetattr;
    ops.readdir = fuseReaddir;
    ops.open = fuseOpen;
    ops.read = fuseRead;
    ops.release = fuseRelease;
    return ops;
}


// ---------------------------------------------------------------------------
// Path resolution
// ---------------------------------------------------------------------------

VfsNode *
FuseOps::resolvePath(const char * path)
{
    auto * node = AlpineFuse::rootNode();
    if (!node)
        return nullptr;

    if (path[0] == '/' && path[1] == '\0')
        return node;

    string segment;

    for (const char * p = path + 1;; ++p) {
        if (*p == '/' || *p == '\0') {
            if (!segment.empty()) {
                auto * child = node->findChild(segment);
                if (!child)
                    return nullptr;
                node = child;
                segment.clear();
            }

            if (*p == '\0')
                break;
        } else {
            segment += *p;
        }
    }

    return node;
}


// ---------------------------------------------------------------------------
// Sanitize filename
// ---------------------------------------------------------------------------

string
FuseOps::sanitizeFilename(const string & name)
{
    if (name.empty())
        return "unnamed"s;

    string result;
    result.reserve(name.size());

    for (auto ch : name) {
        if (ch == '/')
            result += '_';
        else
            result += ch;
    }

    if (result.size() > 255)
        result.resize(255);

    return result;
}


// ---------------------------------------------------------------------------
// Populate: /queries/{term}/
// ---------------------------------------------------------------------------

void
FuseOps::populateQueries(VfsNode * queriesDir, const string & searchTerm)
{
    QueryCache::t_CachedQuery cached{};

    if (!QueryCache::getOrCreateQuery(searchTerm, cached))
        return;

    queriesDir->queryId = cached.queryId;

    // Add .query_status virtual file
    auto * statusFile = queriesDir->addChild(".query_status"s, VfsNode::t_NodeType::File);
    statusFile->queryId = cached.queryId;

    for (const auto & [peerId, peerRes] : cached.results) {
        auto peerDirName = "peer_"s + std::to_string(peerId);
        auto * peerDir = queriesDir->addChild(peerDirName, VfsNode::t_NodeType::Directory);
        peerDir->peerId = peerId;

        for (const auto & res : peerRes.resourceDescList) {
            auto fileName = sanitizeFilename(res.description.empty() ? "resource_"s + std::to_string(res.resourceId)
                                                                     : res.description);

            auto * fileNode = peerDir->addChild(fileName, VfsNode::t_NodeType::File);
            fileNode->resourceId = res.resourceId;
            fileNode->peerId = peerId;
            fileNode->size = res.size;
            fileNode->queryTerm = searchTerm;
            fileNode->queryId = cached.queryId;

            if (!res.locators.empty())
                fileNode->locatorUrl = res.locators.front();
        }
    }

    AccessTracker::recordQueryAccess(searchTerm);
}


// ---------------------------------------------------------------------------
// Populate: /by-peer/
// ---------------------------------------------------------------------------

void
FuseOps::populatePeers(VfsNode * peerDir)
{
    DtcpStackInterface::t_DtcpPeerIdList peerIds;

    if (!DtcpStackInterface::getAllDtcpPeerIds(peerIds))
        return;

    for (auto peerId : peerIds) {
        auto dirName = "peer_"s + std::to_string(peerId);
        auto * subDir = peerDir->addChild(dirName, VfsNode::t_NodeType::Directory);
        subDir->peerId = peerId;

        auto * infoFile = subDir->addChild(".peer_info"s, VfsNode::t_NodeType::File);
        infoFile->peerId = peerId;
    }
}


// ---------------------------------------------------------------------------
// Populate: /by-group/
// ---------------------------------------------------------------------------

void
FuseOps::populateGroups(VfsNode * groupDir)
{
    auto groupsResult = AlpineStackInterface::listGroups2();
    if (!groupsResult)
        return;

    for (auto groupId : *groupsResult) {
        auto infoResult = AlpineStackInterface::getGroupInfo2(groupId);
        if (!infoResult)
            continue;

        auto dirName = sanitizeFilename(infoResult->groupName);
        auto * subDir = groupDir->addChild(dirName, VfsNode::t_NodeType::Directory);

        // Store groupId in queryId field for generateGroupInfo
        auto * infoFile = subDir->addChild(".group_info"s, VfsNode::t_NodeType::File);
        infoFile->queryId = groupId;

        // Peers sub-directory
        auto * peersSubDir = subDir->addChild("peers"s, VfsNode::t_NodeType::Directory);

        auto peerListResult = AlpineStackInterface::getGroupPeerList2(groupId);
        if (peerListResult) {
            for (auto peerId : *peerListResult) {
                auto peerName = "peer_"s + std::to_string(peerId);
                auto * peerFile = peersSubDir->addChild(peerName, VfsNode::t_NodeType::File);
                peerFile->peerId = peerId;
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Populate: /by-quality/
// ---------------------------------------------------------------------------

void
FuseOps::populateQualityTiers(VfsNode * qualityDir)
{
    auto * highDir = qualityDir->findChild("high"s);
    auto * mediumDir = qualityDir->findChild("medium"s);
    auto * lowDir = qualityDir->findChild("low"s);

    if (!highDir || !mediumDir || !lowDir)
        return;

    DtcpStackInterface::t_DtcpPeerIdList peerIds;

    if (!DtcpStackInterface::getAllDtcpPeerIds(peerIds))
        return;

    for (auto peerId : peerIds) {
        auto profileResult = AlpineStackInterface::getDefaultPeerProfile2(peerId);
        if (!profileResult)
            continue;

        auto peerName = "peer_"s + std::to_string(peerId);
        VfsNode * targetDir;

        if (profileResult->relativeQuality > 50)
            targetDir = highDir;
        else if (profileResult->relativeQuality > -50)
            targetDir = mediumDir;
        else
            targetDir = lowDir;

        auto * peerFile = targetDir->addChild(peerName, VfsNode::t_NodeType::File);
        peerFile->peerId = peerId;
    }
}


// ---------------------------------------------------------------------------
// Populate: /recent/
// ---------------------------------------------------------------------------

void
FuseOps::populateRecent(VfsNode * recentDir)
{
    vector<AccessTracker::t_ResourceStats> recentList;
    AccessTracker::getMostRecentResources(recentList, 50);

    for (const auto & rs : recentList) {
        auto fileName =
            sanitizeFilename(rs.description.empty() ? "resource_"s + std::to_string(rs.resourceId) : rs.description);

        auto * fileNode = recentDir->addChild(fileName, VfsNode::t_NodeType::File);
        fileNode->resourceId = rs.resourceId;
        fileNode->peerId = rs.peerId;
    }
}


// ---------------------------------------------------------------------------
// Populate: /popular/
// ---------------------------------------------------------------------------

void
FuseOps::populatePopular(VfsNode * popularDir)
{
    vector<AccessTracker::t_ResourceStats> popularList;
    AccessTracker::getMostAccessedResources(popularList, 50);

    for (const auto & rs : popularList) {
        auto fileName =
            sanitizeFilename(rs.description.empty() ? "resource_"s + std::to_string(rs.resourceId) : rs.description);

        auto * fileNode = popularDir->addChild(fileName, VfsNode::t_NodeType::File);
        fileNode->resourceId = rs.resourceId;
        fileNode->peerId = rs.peerId;
    }
}


// ---------------------------------------------------------------------------
// Content generators
// ---------------------------------------------------------------------------

string
FuseOps::generateStatsContent()
{
    return AccessTracker::serializeText();
}


string
FuseOps::generatePeerInfo(ulong peerId)
{
    std::ostringstream out;

    out << "=== Peer " << peerId << " ===\n";

    DtcpStackInterface::t_DtcpPeerStatus dtcpStatus{};
    if (DtcpStackInterface::getDtcpPeerStatus(peerId, dtcpStatus)) {
        out << "IP: " << dtcpStatus.ipAddress << ":" << dtcpStatus.port << "\n"
            << "Avg bandwidth: " << dtcpStatus.avgBandwidth << "\n"
            << "Peak bandwidth: " << dtcpStatus.peakBandwidth << "\n";
    }

    auto profileResult = AlpineStackInterface::getDefaultPeerProfile2(peerId);
    if (profileResult) {
        out << "Quality: " << profileResult->relativeQuality << "\n"
            << "Total queries: " << profileResult->totalQueries << "\n"
            << "Total responses: " << profileResult->totalResponses << "\n";
    }

    AccessTracker::t_PeerStats peerStats{};
    if (AccessTracker::getPeerStats(peerId, peerStats)) {
        out << "VFS accesses: " << peerStats.aggregateAccessCount << "\n"
            << "VFS bytes read: " << peerStats.aggregateBytesRead << "\n"
            << "Distinct resources: " << peerStats.distinctResourcesAccessed << "\n";
    }

    return out.str();
}


string
FuseOps::generateQueryStatus(ulong queryId)
{
    std::ostringstream out;

    out << "=== Query " << queryId << " ===\n";

    auto statusResult = AlpineStackInterface::getQueryStatus2(queryId);
    if (statusResult) {
        out << "Total peers: " << statusResult->totalPeers << "\n"
            << "Peers queried: " << statusResult->peersQueried << "\n"
            << "Responses: " << statusResult->numPeerResponses << "\n"
            << "Total hits: " << statusResult->totalHits << "\n";
    }

    bool inProgress = AlpineStackInterface::queryInProgress(queryId);
    out << "In progress: " << (inProgress ? "yes" : "no") << "\n";

    return out.str();
}


string
FuseOps::generateGroupInfo(ulong groupId)
{
    std::ostringstream out;

    auto infoResult = AlpineStackInterface::getGroupInfo2(groupId);
    if (infoResult) {
        out << "=== Group: " << infoResult->groupName << " ===\n"
            << "ID: " << infoResult->groupId << "\n"
            << "Description: " << infoResult->description << "\n"
            << "Peers: " << infoResult->numPeers << "\n"
            << "Total queries: " << infoResult->totalQueries << "\n"
            << "Total responses: " << infoResult->totalResponses << "\n";
    }

    return out.str();
}


// ---------------------------------------------------------------------------
// fuseGetattr
// ---------------------------------------------------------------------------

int
FuseOps::fuseGetattr(const char * path,
                     struct stat * stbuf
#if FUSE_USE_VERSION >= 30
                     ,
                     struct fuse_file_info * /* fi */
#endif
)
{
    std::memset(stbuf, 0, sizeof(struct stat));

    auto * node = resolvePath(path);

    if (!node)
        return -ENOENT;

    auto ctime = std::chrono::system_clock::to_time_t(node->createdAt);
    auto atime = std::chrono::system_clock::to_time_t(node->accessedAt);

    if (node->isDirectory()) {
        stbuf->st_mode = S_IFDIR | 0555;
        stbuf->st_nlink = 2;
    } else {
        stbuf->st_mode = S_IFREG | 0444;
        stbuf->st_nlink = 1;
        stbuf->st_size = static_cast<off_t>(node->size);
    }

    stbuf->st_atime = atime;
    stbuf->st_mtime = ctime;
    stbuf->st_ctime = ctime;

    return 0;
}


// ---------------------------------------------------------------------------
// fuseReaddir
// ---------------------------------------------------------------------------

int
FuseOps::fuseReaddir(const char * path,
                     void * buf,
                     fuse_fill_dir_t filler,
                     off_t /* offset */,
                     struct fuse_file_info * /* fi */
#if FUSE_USE_VERSION >= 30
                     ,
                     enum fuse_readdir_flags /* flags */
#endif
)
{
    auto * node = resolvePath(path);

    if (!node || !node->isDirectory())
        return -ENOENT;

    // Dynamic population based on directory role
    auto nodePath = string(path);

    if (nodePath == "/by-peer"s)
        populatePeers(node);
    else if (nodePath == "/by-group"s)
        populateGroups(node);
    else if (nodePath == "/by-quality"s)
        populateQualityTiers(node);
    else if (nodePath == "/recent"s)
        populateRecent(node);
    else if (nodePath == "/popular"s)
        populatePopular(node);
    else if (nodePath.starts_with("/queries/"s) && !node->queryTerm.empty())
        populateQueries(node, node->queryTerm);

    filler(buf,
           ".",
           nullptr,
           0
#if FUSE_USE_VERSION >= 30
           ,
           static_cast<fuse_fill_dir_flags>(0)
#endif
    );
    filler(buf,
           "..",
           nullptr,
           0
#if FUSE_USE_VERSION >= 30
           ,
           static_cast<fuse_fill_dir_flags>(0)
#endif
    );

    for (const auto & [childName, _] : node->children) {
        filler(buf,
               childName.c_str(),
               nullptr,
               0
#if FUSE_USE_VERSION >= 30
               ,
               static_cast<fuse_fill_dir_flags>(0)
#endif
        );
    }

    return 0;
}


// ---------------------------------------------------------------------------
// fuseOpen
// ---------------------------------------------------------------------------

int
FuseOps::fuseOpen(const char * path, struct fuse_file_info * fi)
{
    auto * node = resolvePath(path);

    if (!node)
        return -ENOENT;

    if (node->isDirectory())
        return -EISDIR;

    // Read-only filesystem
    if ((fi->flags & O_ACCMODE) != O_RDONLY)
        return -EACCES;

    node->accessedAt = std::chrono::system_clock::now();

    // Record access and check feedback threshold
    if (node->resourceId != 0) {
        AccessTracker::recordResourceAccess(node->resourceId, node->peerId, node->name);

        AccessTracker::t_ResourceStats stats{};
        if (AccessTracker::getResourceStats(node->resourceId, stats)) {
            if (stats.accessCount >= AlpineFuse::feedbackThreshold()) {
                AlpinePeerProfile * profile = nullptr;
                short qualityDelta = 0;
                AlpineRatingEngine::clientResourceEvaluation(
                    profile, AlpineRatingEngine::t_ResourceRating::Average, qualityDelta);
            }
        }
    }

    return 0;
}


// ---------------------------------------------------------------------------
// fuseRead
// ---------------------------------------------------------------------------

int
FuseOps::fuseRead(const char * path, char * buf, size_t size, off_t offset, struct fuse_file_info * /* fi */)
{
    auto * node = resolvePath(path);

    if (!node)
        return -ENOENT;

    if (node->isDirectory())
        return -EISDIR;

    // Generate content based on virtual file type
    string content;

    if (node->name == ".stats"s)
        content = generateStatsContent();
    else if (node->name == ".peer_info"s)
        content = generatePeerInfo(node->peerId);
    else if (node->name == ".query_status"s)
        content = generateQueryStatus(node->queryId);
    else if (node->name == ".group_info"s)
        content = generateGroupInfo(node->queryId);
    else {
        // Resource file stub with metadata
        std::ostringstream out;
        out << "resource_id: " << node->resourceId << "\n"
            << "peer_id: " << node->peerId << "\n"
            << "size: " << node->size << "\n"
            << "query: " << node->queryTerm << "\n"
            << "locator: " << node->locatorUrl << "\n";

        if (!node->locatorUrl.empty())
            out << "\nContent available via locator URL above.\n";

        content = out.str();
    }

    if (static_cast<size_t>(offset) >= content.size())
        return 0;

    auto bytesAvailable = content.size() - static_cast<size_t>(offset);
    auto bytesToCopy = std::min(size, bytesAvailable);

    std::memcpy(buf, content.data() + offset, bytesToCopy);

    if (node->resourceId != 0) {
        AccessTracker::recordResourceAccess(
            node->resourceId, node->peerId, node->name, static_cast<ulong>(bytesToCopy));
    }

    return static_cast<int>(bytesToCopy);
}


// ---------------------------------------------------------------------------
// fuseRelease
// ---------------------------------------------------------------------------

int
FuseOps::fuseRelease(const char * /* path */, struct fuse_file_info * /* fi */)
{
    return 0;
}
