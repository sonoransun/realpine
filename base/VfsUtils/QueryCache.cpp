/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <QueryCache.h>


std::unordered_map<string, QueryCache::t_CachedQuery, OptHash<string>> QueryCache::queryIndex_s;

ReadWriteSem QueryCache::dataLock_s;
ulong QueryCache::ttlSeconds_s = 60;


void
QueryCache::initialize(ulong ttlSeconds)
{
    dataLock_s.acquireWrite();
    ttlSeconds_s = ttlSeconds;
    queryIndex_s.clear();
    dataLock_s.releaseWrite();
}


bool
QueryCache::getOrCreateQuery(const string & searchTerm, t_CachedQuery & cached)
{
    evictExpired();

    dataLock_s.acquireRead();

    if (queryIndex_s.contains(searchTerm)) {
        auto & entry = queryIndex_s[searchTerm];

        // Refresh status if query was still in progress
        if (!entry.complete) {
            dataLock_s.releaseRead();
            dataLock_s.acquireWrite();

            entry.complete = !AlpineStackInterface::queryInProgress(entry.queryId);

            auto statusResult = AlpineStackInterface::getQueryStatus2(entry.queryId);
            if (statusResult)
                entry.status = *statusResult;

            auto resultsResult = AlpineStackInterface::getQueryResults2(entry.queryId);
            if (resultsResult)
                entry.results = *resultsResult;

            cached = entry;
            dataLock_s.releaseWrite();
            return true;
        }

        cached = entry;
        dataLock_s.releaseRead();
        return true;
    }

    dataLock_s.releaseRead();

    // Not cached — start a new query
    t_CachedQuery entry{};

    if (!executeQuery(searchTerm, entry))
        return false;

    dataLock_s.acquireWrite();
    queryIndex_s[searchTerm] = entry;
    cached = entry;
    dataLock_s.releaseWrite();

    return true;
}


bool
QueryCache::refreshQuery(const string & searchTerm)
{
    // Remove old entry and re-query
    dataLock_s.acquireWrite();
    queryIndex_s.erase(searchTerm);
    dataLock_s.releaseWrite();

    t_CachedQuery entry{};

    if (!executeQuery(searchTerm, entry))
        return false;

    dataLock_s.acquireWrite();
    queryIndex_s[searchTerm] = entry;
    dataLock_s.releaseWrite();

    return true;
}


bool
QueryCache::isCached(const string & searchTerm)
{
    dataLock_s.acquireRead();

    if (!queryIndex_s.contains(searchTerm)) {
        dataLock_s.releaseRead();
        return false;
    }

    auto elapsed = std::chrono::steady_clock::now() - queryIndex_s[searchTerm].fetchedAt;

    bool valid = elapsed < std::chrono::seconds(ttlSeconds_s);
    dataLock_s.releaseRead();

    return valid;
}


void
QueryCache::evictExpired()
{
    dataLock_s.acquireWrite();

    auto now = std::chrono::steady_clock::now();
    auto ttl = std::chrono::seconds(ttlSeconds_s);

    for (auto it = queryIndex_s.begin(); it != queryIndex_s.end();) {
        if (now - it->second.fetchedAt > ttl)
            it = queryIndex_s.erase(it);
        else
            ++it;
    }

    dataLock_s.releaseWrite();
}


bool
QueryCache::remove(const string & searchTerm)
{
    dataLock_s.acquireWrite();
    bool erased = queryIndex_s.erase(searchTerm) > 0;
    dataLock_s.releaseWrite();

    return erased;
}


void
QueryCache::getAllTerms(t_StringList & terms)
{
    terms.clear();

    dataLock_s.acquireRead();

    for (const auto & [key, _] : queryIndex_s)
        terms.push_back(key);

    dataLock_s.releaseRead();
}


bool
QueryCache::executeQuery(const string & searchTerm, t_CachedQuery & entry)
{
    AlpineStackInterface::t_QueryOptions options{};
    auto queryResult = AlpineStackInterface::startQuery2(options, searchTerm);

    if (!queryResult)
        return false;

    entry.queryId = *queryResult;
    entry.queryTerm = searchTerm;
    entry.fetchedAt = std::chrono::steady_clock::now();
    entry.complete = !AlpineStackInterface::queryInProgress(entry.queryId);

    auto statusResult = AlpineStackInterface::getQueryStatus2(entry.queryId);
    if (statusResult)
        entry.status = *statusResult;

    auto resultsResult = AlpineStackInterface::getQueryResults2(entry.queryId);
    if (resultsResult)
        entry.results = *resultsResult;

    return true;
}
