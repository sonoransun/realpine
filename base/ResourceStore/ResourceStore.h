/// Copyright (C) 2026 sonoransun — see LICENCE.txt
///
/// On-disk store of downloaded resources plus a path -> {resourceId, peerId}
/// registry.  Acts as the watched directory for the FsObserve subsystem so
/// that implicit-feedback signals flow into AlpineRatingEngine without FUSE.


#pragma once
#include <Common.h>
#include <ReadWriteSem.h>
#include <unordered_map>


class ResourceStore
{
  public:
    ResourceStore() = default;
    ~ResourceStore() = default;


    // Initialize with a root directory; created if missing.
    //
    bool initialize(const string & rootDirectory);


    // Cleanly close any persistent backing storage.
    //
    void shutdown();


    // Record that a query reply for resourceId from peerId was materialized
    // to fsPath.  fsPath must live underneath root().
    //
    bool registerDownload(ulong resourceId, ulong peerId, const string & fsPath);


    // Reverse lookup used by FsObserve event sinks.  Returns false when the
    // path does not refer to a registered resource.
    //
    bool resolvePath(const string & fsPath, ulong & resourceId, ulong & peerId);


    // Forget a resource (e.g. after eviction or removal from disk).
    //
    bool unregister(ulong resourceId);


    // Watched root.
    //
    const string &
    root() const
    {
        return root_;
    }


    // Number of registered resources (debug/test).
    //
    ulong size();


  private:
    struct t_Entry
    {
        ulong resourceId;
        ulong peerId;
        string fsPath;
    };


    string root_;
    std::unordered_map<string, t_Entry> byPath_;  // canonical path -> entry
    std::unordered_map<ulong, string> idToPath_;  // resourceId -> canonical path
    ReadWriteSem lock_;


    static string canonicalize(const string & fsPath);


    ResourceStore(const ResourceStore &) = delete;
    ResourceStore & operator=(const ResourceStore &) = delete;
};
