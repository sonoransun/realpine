/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <OptHash.h>
#include <ReadWriteSem.h>
#include <chrono>
#include <unordered_map>


class AccessTracker
{
  public:

    struct t_ResourceStats {
        ulong   resourceId;
        ulong   peerId;
        string  description;
        ulong   accessCount;
        ulong   totalBytesRead;
        std::chrono::system_clock::time_point  lastAccessTime;
    };

    struct t_PeerStats {
        ulong  peerId;
        ulong  aggregateAccessCount;
        ulong  aggregateBytesRead;
        ulong  distinctResourcesAccessed;
        std::chrono::system_clock::time_point  lastAccessTime;
    };

    struct t_QueryTermStats {
        string  queryTerm;
        ulong   accessCount;
        std::chrono::system_clock::time_point  lastAccessTime;
    };


    AccessTracker ()  = default;
    ~AccessTracker () = default;


    static void  recordResourceAccess (ulong           resourceId,
                                       ulong           peerId,
                                       const string &  description,
                                       ulong           bytesRead = 0);

    static void  recordQueryAccess (const string &  queryTerm);

    static bool  getResourceStats (ulong             resourceId,
                                   t_ResourceStats & stats);

    static bool  getPeerStats (ulong          peerId,
                               t_PeerStats &  stats);

    static void  getMostAccessedResources (vector<t_ResourceStats> &  list,
                                           ulong                      limit = 50);

    static void  getMostRecentResources (vector<t_ResourceStats> &  list,
                                         ulong                      limit = 50);

    static string  serializeJson ();

    static string  serializeText ();


  private:

    static std::unordered_map<ulong, t_ResourceStats, OptHash<ulong>>    resourceStats_s;
    static std::unordered_map<ulong, t_PeerStats, OptHash<ulong>>        peerStats_s;
    static std::unordered_map<string, t_QueryTermStats, OptHash<string>> queryTermStats_s;
    static ReadWriteSem                                                   dataLock_s;
};
