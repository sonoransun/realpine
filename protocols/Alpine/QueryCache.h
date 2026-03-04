/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <ReadWriteSem.h>
#include <chrono>
#include <list>
#include <optional>
#include <unordered_map>


class QueryCache
{
  public:

    struct t_CacheEntry {
        string                                          queryKey;
        string                                          results;
        std::chrono::steady_clock::time_point           insertTime;
    };

    using t_EntryList     = std::list<t_CacheEntry>;
    using t_EntryIterator = t_EntryList::iterator;


    static void  configure (ulong maxEntries,
                            ulong ttlSeconds);

    static std::optional<string>  lookup (const string & queryKey);

    static void  store (const string & queryKey,
                        const string & results);

    static void  invalidate (const string & queryKey);

    static void  clear ();

    static ulong  size ();


  private:

    static void  evictExpired ();

    static ReadWriteSem                                          lock_s;
    static t_EntryList                                           entries_s;
    static std::unordered_map<string, t_EntryIterator>           index_s;
    static ulong                                                 maxEntries_s;
    static ulong                                                 ttlSeconds_s;

};
