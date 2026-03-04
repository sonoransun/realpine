/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include "QueryCache.h"
#include <WriteLock.h>
#include <ReadLock.h>
#include <Log.h>


ReadWriteSem                                          QueryCache::lock_s;
QueryCache::t_EntryList                               QueryCache::entries_s;
std::unordered_map<string, QueryCache::t_EntryIterator>  QueryCache::index_s;
ulong                                                 QueryCache::maxEntries_s  = 1000;
ulong                                                 QueryCache::ttlSeconds_s  = 300;



void
QueryCache::configure (ulong maxEntries,
                       ulong ttlSeconds)
{
    WriteLock lock(lock_s);
    maxEntries_s = maxEntries;
    ttlSeconds_s = ttlSeconds;
}



std::optional<string>
QueryCache::lookup (const string & queryKey)
{
    WriteLock lock(lock_s);

    auto it = index_s.find(queryKey);
    if (it == index_s.end())
        return std::nullopt;

    auto & entry = *(it->second);
    auto elapsed = std::chrono::steady_clock::now() - entry.insertTime;
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(elapsed).count();

    if (static_cast<ulong>(seconds) >= ttlSeconds_s) {
        entries_s.erase(it->second);
        index_s.erase(it);
        return std::nullopt;
    }

    // Move to front (most recently used)
    entries_s.splice(entries_s.begin(), entries_s, it->second);

    return entry.results;
}



void
QueryCache::store (const string & queryKey,
                   const string & results)
{
    WriteLock lock(lock_s);

    // Remove existing entry if present
    auto it = index_s.find(queryKey);
    if (it != index_s.end()) {
        entries_s.erase(it->second);
        index_s.erase(it);
    }

    // Insert at front
    entries_s.push_front({queryKey, results, std::chrono::steady_clock::now()});
    index_s[queryKey] = entries_s.begin();

    // Evict LRU entries if over capacity
    while (entries_s.size() > maxEntries_s) {
        auto & back = entries_s.back();
        index_s.erase(back.queryKey);
        entries_s.pop_back();
    }
}



void
QueryCache::invalidate (const string & queryKey)
{
    WriteLock lock(lock_s);

    auto it = index_s.find(queryKey);
    if (it != index_s.end()) {
        entries_s.erase(it->second);
        index_s.erase(it);
    }
}



void
QueryCache::clear ()
{
    WriteLock lock(lock_s);
    entries_s.clear();
    index_s.clear();
}



ulong
QueryCache::size ()
{
    ReadLock lock(lock_s);
    return entries_s.size();
}



void
QueryCache::evictExpired ()
{
    auto now = std::chrono::steady_clock::now();

    auto it = entries_s.begin();
    while (it != entries_s.end()) {
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - it->insertTime).count();
        if (static_cast<ulong>(elapsed) >= ttlSeconds_s) {
            index_s.erase(it->queryKey);
            it = entries_s.erase(it);
        } else {
            ++it;
        }
    }
}
