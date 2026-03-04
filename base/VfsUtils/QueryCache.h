/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <OptHash.h>
#include <ReadWriteSem.h>
#include <AlpineStackInterface.h>
#include <chrono>
#include <unordered_map>


class QueryCache
{
  public:

    struct t_CachedQuery {
        ulong                                       queryId;
        string                                      queryTerm;
        AlpineStackInterface::t_PeerResourcesIndex  results;
        AlpineStackInterface::t_QueryStatus         status;
        std::chrono::steady_clock::time_point       fetchedAt;
        bool                                        complete;
    };

    QueryCache ()  = default;
    ~QueryCache () = default;


    static void  initialize (ulong  ttlSeconds = 60);

    static bool  getOrCreateQuery (const string &   searchTerm,
                                   t_CachedQuery &  cached);

    static bool  refreshQuery (const string &  searchTerm);

    static bool  isCached (const string &  searchTerm);

    static void  evictExpired ();

    static bool  remove (const string &  searchTerm);

    static void  getAllTerms (t_StringList &  terms);


  private:

    static std::unordered_map<string, t_CachedQuery, OptHash<string>>  queryIndex_s;
    static ReadWriteSem                                                 dataLock_s;
    static ulong                                                        ttlSeconds_s;

    static bool  executeQuery (const string &   searchTerm,
                               t_CachedQuery &  entry);
};
