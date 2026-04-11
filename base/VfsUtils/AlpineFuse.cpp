/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AlpineFuse.h>
#include <FuseOps.h>
#include <Log.h>
#include <QueryCache.h>
#include <VfsNode.h>

#include <fuse.h>

#include <filesystem>


std::unique_ptr<VfsNode> AlpineFuse::root_s;
std::unique_ptr<AlpineFuse::FuseThread> AlpineFuse::fuseThread_s;
string AlpineFuse::mountPoint_s;
ulong AlpineFuse::cacheTtlSeconds_s = 60;
ulong AlpineFuse::feedbackThreshold_s = 5;
bool AlpineFuse::running_s = false;
ReadWriteSem AlpineFuse::dataLock_s;


// ---------------------------------------------------------------------------
// initialize
// ---------------------------------------------------------------------------

bool
AlpineFuse::initialize(const string & mountPoint, ulong cacheTtlSeconds, ulong feedbackThresholdVal)
{
    dataLock_s.acquireWrite();

    mountPoint_s = mountPoint;
    cacheTtlSeconds_s = cacheTtlSeconds;
    feedbackThreshold_s = feedbackThresholdVal;

    dataLock_s.releaseWrite();

    QueryCache::initialize(cacheTtlSeconds);
    buildRootTree();

    if (!prepareMountPoint()) {
        Log::Error("AlpineFuse: failed to prepare mount point: "s + mountPoint);
        return false;
    }

    Log::Info("AlpineFuse: initialized at "s + mountPoint);
    return true;
}


// ---------------------------------------------------------------------------
// run
// ---------------------------------------------------------------------------

bool
AlpineFuse::run()
{
    dataLock_s.acquireWrite();

    if (running_s) {
        dataLock_s.releaseWrite();
        Log::Error("AlpineFuse: already running"s);
        return false;
    }

    fuseThread_s = std::make_unique<FuseThread>();

    if (!fuseThread_s->create()) {
        dataLock_s.releaseWrite();
        Log::Error("AlpineFuse: failed to create FUSE thread"s);
        return false;
    }

    running_s = true;
    dataLock_s.releaseWrite();

    Log::Info("AlpineFuse: FUSE running on "s + mountPoint_s);
    return true;
}


// ---------------------------------------------------------------------------
// shutdown
// ---------------------------------------------------------------------------

bool
AlpineFuse::shutdown()
{
    dataLock_s.acquireWrite();

    if (!running_s) {
        dataLock_s.releaseWrite();
        return true;
    }

    running_s = false;
    dataLock_s.releaseWrite();

    if (fuseThread_s) {
        fuseThread_s->destroy();
        fuseThread_s.reset();
    }

    Log::Info("AlpineFuse: shutdown complete"s);
    return true;
}


// ---------------------------------------------------------------------------
// isRunning
// ---------------------------------------------------------------------------

bool
AlpineFuse::isRunning()
{
    dataLock_s.acquireRead();
    bool result = running_s;
    dataLock_s.releaseRead();
    return result;
}


// ---------------------------------------------------------------------------
// getMountPoint
// ---------------------------------------------------------------------------

const string &
AlpineFuse::getMountPoint()
{
    return mountPoint_s;
}


// ---------------------------------------------------------------------------
// rootNode
// ---------------------------------------------------------------------------

VfsNode *
AlpineFuse::rootNode()
{
    return root_s.get();
}


// ---------------------------------------------------------------------------
// feedbackThreshold
// ---------------------------------------------------------------------------

ulong
AlpineFuse::feedbackThreshold()
{
    dataLock_s.acquireRead();
    ulong result = feedbackThreshold_s;
    dataLock_s.releaseRead();
    return result;
}


// ---------------------------------------------------------------------------
// buildRootTree
// ---------------------------------------------------------------------------

void
AlpineFuse::buildRootTree()
{
    root_s = std::make_unique<VfsNode>(""s, VfsNode::t_NodeType::Directory);

    root_s->addChild("queries"s, VfsNode::t_NodeType::Directory);
    root_s->addChild("by-peer"s, VfsNode::t_NodeType::Directory);
    root_s->addChild("by-group"s, VfsNode::t_NodeType::Directory);
    root_s->addChild("by-quality"s, VfsNode::t_NodeType::Directory);
    root_s->addChild("recent"s, VfsNode::t_NodeType::Directory);
    root_s->addChild("popular"s, VfsNode::t_NodeType::Directory);

    // Pre-create quality tier subdirectories
    auto * qualityDir = root_s->findChild("by-quality"s);
    qualityDir->addChild("high"s, VfsNode::t_NodeType::Directory);
    qualityDir->addChild("medium"s, VfsNode::t_NodeType::Directory);
    qualityDir->addChild("low"s, VfsNode::t_NodeType::Directory);

    root_s->addChild(".stats"s, VfsNode::t_NodeType::File);

    Log::Debug("AlpineFuse: root tree built"s);
}


// ---------------------------------------------------------------------------
// prepareMountPoint
// ---------------------------------------------------------------------------

bool
AlpineFuse::prepareMountPoint()
{
    std::error_code ec;
    std::filesystem::create_directories(mountPoint_s, ec);

    if (ec) {
        Log::Error("AlpineFuse: create_directories failed: "s + ec.message());
        return false;
    }

    return true;
}


// ---------------------------------------------------------------------------
// FuseThread::threadMain
// ---------------------------------------------------------------------------

void
AlpineFuse::FuseThread::threadMain()
{
    Log::Info("AlpineFuse: FUSE event loop starting"s);

    auto mountStr = mountPoint_s;
    const char * argv[] = {"alpine_fuse", "-f", mountStr.c_str()};
    int argc = 3;

    struct fuse_args args = FUSE_ARGS_INIT(argc, const_cast<char **>(argv));

    auto & ops = FuseOps::operations();

    int ret = fuse_main(args.argc, args.argv, &ops, nullptr);

    if (ret != 0)
        Log::Error("AlpineFuse: fuse_main returned "s + std::to_string(ret));

    dataLock_s.acquireWrite();
    running_s = false;
    dataLock_s.releaseWrite();

    Log::Info("AlpineFuse: FUSE event loop exited"s);
}
