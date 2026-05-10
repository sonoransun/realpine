/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <Log.h>
#include <ReadLock.h>
#include <ResourceStore.h>
#include <WriteLock.h>

#include <filesystem>
#include <system_error>


bool
ResourceStore::initialize(const string & rootDirectory)
{
    if (rootDirectory.empty()) {
        Log::Error("ResourceStore: empty root directory"s);
        return false;
    }

    std::error_code ec;
    std::filesystem::create_directories(rootDirectory, ec);

    if (ec) {
        Log::Error("ResourceStore: create_directories failed for "s + rootDirectory + ": "s + ec.message());
        return false;
    }

    auto canonical = std::filesystem::weakly_canonical(rootDirectory, ec);
    if (ec) {
        Log::Error("ResourceStore: weakly_canonical failed for "s + rootDirectory + ": "s + ec.message());
        return false;
    }

    {
        WriteLock lock(lock_);
        root_ = canonical.string();
        byPath_.clear();
        idToPath_.clear();
    }

    Log::Info("ResourceStore: initialized at "s + root_);
    return true;
}


void
ResourceStore::shutdown()
{
    WriteLock lock(lock_);
    byPath_.clear();
    idToPath_.clear();
}


bool
ResourceStore::registerDownload(ulong resourceId, ulong peerId, const string & fsPath)
{
    if (resourceId == 0 || peerId == 0 || fsPath.empty())
        return false;

    auto canonical = canonicalize(fsPath);

    WriteLock lock(lock_);

    if (root_.empty() || canonical.rfind(root_, 0) != 0) {
        Log::Error("ResourceStore: registerDownload path outside root: "s + canonical);
        return false;
    }

    auto existing = idToPath_.find(resourceId);
    if (existing != idToPath_.end() && existing->second != canonical) {
        byPath_.erase(existing->second);
        existing->second = canonical;
    } else if (existing == idToPath_.end()) {
        idToPath_[resourceId] = canonical;
    }

    byPath_[canonical] = {resourceId, peerId, canonical};
    return true;
}


bool
ResourceStore::resolvePath(const string & fsPath, ulong & resourceId, ulong & peerId)
{
    auto canonical = canonicalize(fsPath);

    ReadLock lock(lock_);

    auto it = byPath_.find(canonical);
    if (it == byPath_.end())
        return false;

    resourceId = it->second.resourceId;
    peerId = it->second.peerId;
    return true;
}


bool
ResourceStore::unregister(ulong resourceId)
{
    WriteLock lock(lock_);

    auto it = idToPath_.find(resourceId);
    if (it == idToPath_.end())
        return false;

    byPath_.erase(it->second);
    idToPath_.erase(it);
    return true;
}


ulong
ResourceStore::size()
{
    ReadLock lock(lock_);
    return static_cast<ulong>(idToPath_.size());
}


string
ResourceStore::canonicalize(const string & fsPath)
{
    std::error_code ec;
    auto canonical = std::filesystem::weakly_canonical(fsPath, ec);
    if (ec)
        return fsPath;
    return canonical.string();
}
