/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <VfsNode.h>

// Forward-declare to avoid pulling FUSE headers into the header
struct fuse_operations;


class FuseOps
{
  public:
    FuseOps() = default;
    ~FuseOps() = default;

    static const fuse_operations & operations();


  private:
    // FUSE operation callbacks
    //
    static int fuseGetattr(const char * path,
                           struct stat * stbuf
#if FUSE_USE_VERSION >= 30
                           ,
                           struct fuse_file_info * fi
#endif
    );

    static int fuseReaddir(const char * path,
                           void * buf,
                           fuse_fill_dir_t filler,
                           off_t offset,
                           struct fuse_file_info * fi
#if FUSE_USE_VERSION >= 30
                           ,
                           enum fuse_readdir_flags flags
#endif
    );

    static int fuseOpen(const char * path, struct fuse_file_info * fi);

    static int fuseRead(const char * path, char * buf, size_t size, off_t offset, struct fuse_file_info * fi);

    static int fuseRelease(const char * path, struct fuse_file_info * fi);


    // Path resolution
    //
    static VfsNode * resolvePath(const char * path);

    // Populate dynamic directories
    //
    static void populateQueries(VfsNode * queriesDir, const string & searchTerm);

    static void populatePeers(VfsNode * peerDir);

    static void populateGroups(VfsNode * groupDir);

    static void populateQualityTiers(VfsNode * qualityDir);

    static void populateRecent(VfsNode * recentDir);

    static void populatePopular(VfsNode * popularDir);

    // Content generators for virtual files
    //
    static string generateStatsContent();

    static string generatePeerInfo(ulong peerId);

    static string generateQueryStatus(ulong queryId);

    static string generateGroupInfo(ulong groupId);

    // Sanitize filenames
    //
    static string sanitizeFilename(const string & name);
};
