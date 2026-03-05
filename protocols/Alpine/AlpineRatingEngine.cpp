/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AlpineRatingEngine.h>
#include <AlpinePeerProfile.h>
#include <Configuration.h>
#include <ReadLock.h>
#include <WriteLock.h>
#include <Log.h>
#include <StringUtils.h>
#include <algorithm>
#include <cmath>
#include <sstream>
#include <iomanip>



std::unordered_map<ulong, AlpineRatingEngine::t_PeerRating>  AlpineRatingEngine::ratings_s;
ReadWriteSem                                                  AlpineRatingEngine::lock_s;



// ---------------------------------------------------------------------------
//  Helper — extract peer ID from profile
// ---------------------------------------------------------------------------

ulong
AlpineRatingEngine::getPeerId (AlpinePeerProfile * profile)
{
    ulong peerId = 0;
    if (profile)
        profile->getPeerId(peerId);
    return peerId;
}



// ---------------------------------------------------------------------------
//  Helper — apply time-based decay toward kDecayTarget (1% per hour)
// ---------------------------------------------------------------------------

double
AlpineRatingEngine::applyDecay (t_PeerRating & rating)
{
    auto now     = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration<double, std::ratio<3600>>(now - rating.lastUpdate);
    double hours = elapsed.count();

    if (hours > 0.0) {
        double decayFactor = std::pow(1.0 - kDecayRatePerHr, hours);
        rating.score = kDecayTarget + (rating.score - kDecayTarget) * decayFactor;
        rating.lastUpdate = now;
    }

    return rating.score;
}



// ---------------------------------------------------------------------------
//  Helper — EMA update: score = (1 - alpha) * score + alpha * (score + delta)
//           Simplified: score = score + alpha * delta
// ---------------------------------------------------------------------------

double
AlpineRatingEngine::applyDelta (t_PeerRating & rating, double delta, bool isSuccess)
{
    applyDecay(rating);

    rating.score = rating.score + kAlpha * delta;
    rating.score = clampScore(rating.score);

    if (isSuccess)
        ++rating.successCount;
    else
        ++rating.failureCount;

    rating.lastUpdate = std::chrono::steady_clock::now();

    return rating.score;
}



// ---------------------------------------------------------------------------
//  Helper — clamp score to [0.0, 1.0]
// ---------------------------------------------------------------------------

double
AlpineRatingEngine::clampScore (double score)
{
    return std::clamp(score, kMinScore, kMaxScore);
}



// ---------------------------------------------------------------------------
//  queryResponseEvent — +0.1 boost
// ---------------------------------------------------------------------------

bool
AlpineRatingEngine::queryResponseEvent (AlpinePeerProfile *  profile,
                                        short &              qualityDelta)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineRatingEngine::queryResponseEvent invoked.");
#endif

    if (!profile)
        return false;

    ulong peerId = getPeerId(profile);

    WriteLock lock(lock_s);
    auto & rating = ratings_s[peerId];
    applyDelta(rating, 0.1, true);

    qualityDelta = 1;
    return true;
}



// ---------------------------------------------------------------------------
//  badPacketEvent — -0.3 penalty
// ---------------------------------------------------------------------------

bool
AlpineRatingEngine::badPacketEvent (AlpinePeerProfile *  profile,
                                    short &              qualityDelta)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineRatingEngine::badPacketEvent invoked.");
#endif

    if (!profile)
        return false;

    ulong peerId = getPeerId(profile);

    WriteLock lock(lock_s);
    auto & rating = ratings_s[peerId];
    applyDelta(rating, -0.3, false);

    qualityDelta = -3;
    return true;
}



// ---------------------------------------------------------------------------
//  transferFailureEvent — -0.2 penalty
// ---------------------------------------------------------------------------

bool
AlpineRatingEngine::transferFailureEvent (AlpinePeerProfile *  profile,
                                          short &              qualityDelta)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineRatingEngine::transferFailureEvent invoked.");
#endif

    if (!profile)
        return false;

    ulong peerId = getPeerId(profile);

    WriteLock lock(lock_s);
    auto & rating = ratings_s[peerId];
    applyDelta(rating, -0.2, false);

    qualityDelta = -2;
    return true;
}



// ---------------------------------------------------------------------------
//  naResourceEvent — -0.05 minor penalty
// ---------------------------------------------------------------------------

bool
AlpineRatingEngine::naResourceEvent (AlpinePeerProfile *  profile,
                                     short &              qualityDelta)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineRatingEngine::naResourceEvent invoked.");
#endif

    if (!profile)
        return false;

    ulong peerId = getPeerId(profile);

    WriteLock lock(lock_s);
    auto & rating = ratings_s[peerId];
    applyDelta(rating, -0.05, false);

    qualityDelta = -1;
    return true;
}



// ---------------------------------------------------------------------------
//  deceptiveResourceEvent — -0.5 severe penalty
// ---------------------------------------------------------------------------

bool
AlpineRatingEngine::deceptiveResourceEvent (AlpinePeerProfile *  profile,
                                            short &              qualityDelta)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineRatingEngine::deceptiveResourceEvent invoked.");
#endif

    if (!profile)
        return false;

    ulong peerId = getPeerId(profile);

    WriteLock lock(lock_s);
    auto & rating = ratings_s[peerId];
    applyDelta(rating, -0.5, false);

    qualityDelta = -5;
    return true;
}



// ---------------------------------------------------------------------------
//  clientResourceEvaluation — variable delta based on user feedback
// ---------------------------------------------------------------------------

bool
AlpineRatingEngine::clientResourceEvaluation (AlpinePeerProfile *  profile,
                                              t_ResourceRating     rating,
                                              short &              qualityDelta)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineRatingEngine::clientResourceEvaluation invoked.");
#endif

    if (!profile)
        return false;

    double delta = 0.0;
    bool   isSuccess = true;

    switch (rating) {
        case t_ResourceRating::Low:
            delta     = -0.2;
            isSuccess = false;
            break;
        case t_ResourceRating::Average:
            delta     = 0.0;
            isSuccess = true;
            break;
        case t_ResourceRating::High:
            delta     = 0.15;
            isSuccess = true;
            break;
    }

    ulong peerId = getPeerId(profile);

    WriteLock lock(lock_s);
    auto & peerRating = ratings_s[peerId];
    applyDelta(peerRating, delta, isSuccess);

    qualityDelta = static_cast<short>(delta * 10.0);
    return true;
}



// ---------------------------------------------------------------------------
//  getScore — read-only score lookup for external consumers
// ---------------------------------------------------------------------------

double
AlpineRatingEngine::getScore (ulong peerId)
{
    ReadLock lock(lock_s);

    auto it = ratings_s.find(peerId);
    if (it == ratings_s.end())
        return kInitialScore;

    // Apply decay on read (const_cast-style via mutable copy not needed
    // since we hold only a read lock — return decayed estimate without writing)
    auto & rating = it->second;
    auto   now    = std::chrono::steady_clock::now();
    auto   elapsed = std::chrono::duration<double, std::ratio<3600>>(now - rating.lastUpdate);
    double hours  = elapsed.count();

    if (hours <= 0.0)
        return rating.score;

    double decayFactor = std::pow(1.0 - kDecayRatePerHr, hours);
    return clampScore(kDecayTarget + (rating.score - kDecayTarget) * decayFactor);
}



// ---------------------------------------------------------------------------
//  getTopScores — return top N peer scores sorted by interaction count
// ---------------------------------------------------------------------------

vector<AlpineRatingEngine::t_ScoreEntry>
AlpineRatingEngine::getTopScores (size_t maxEntries)
{
    ReadLock lock(lock_s);

    vector<t_ScoreEntry> entries;
    entries.reserve(ratings_s.size());

    for (const auto & [peerId, rating] : ratings_s) {
        ulong interactions = rating.successCount + rating.failureCount;
        if (interactions == 0)
            continue;

        t_ScoreEntry entry;
        entry.peerId       = peerId;
        entry.score        = rating.score;
        entry.interactions = interactions;
        entries.push_back(entry);
    }

    // Sort by interaction count descending, take top N
    std::partial_sort(entries.begin(),
                      entries.begin() + std::min(maxEntries, entries.size()),
                      entries.end(),
                      [](const t_ScoreEntry & a, const t_ScoreEntry & b) {
                          return a.interactions > b.interactions;
                      });

    if (entries.size() > maxEntries)
        entries.resize(maxEntries);

    return entries;
}



// ---------------------------------------------------------------------------
//  mergeRemoteScores — weighted average merge from gossip
// ---------------------------------------------------------------------------

void
AlpineRatingEngine::mergeRemoteScores (const vector<t_ScoreEntry> & remoteScores,
                                       ulong minInteractions)
{
    WriteLock lock(lock_s);

    for (const auto & remote : remoteScores) {
        if (remote.interactions < minInteractions)
            continue;

        auto it = ratings_s.find(remote.peerId);
        if (it == ratings_s.end()) {
            // No local data — adopt remote score directly
            t_PeerRating rating;
            rating.score        = clampScore(remote.score);
            rating.successCount = 0;
            rating.failureCount = 0;
            rating.lastUpdate   = std::chrono::steady_clock::now();
            ratings_s[remote.peerId] = rating;
        } else {
            // Weighted average: local weight = local interactions,
            // remote weight = remote interactions
            auto & local = it->second;
            ulong localInteractions = local.successCount + local.failureCount;

            if (localInteractions < minInteractions) {
                // Not enough local data — adopt remote with 50% weight
                local.score = clampScore(
                    local.score * 0.5 + remote.score * 0.5);
            } else {
                // Weighted average based on interaction counts
                double totalWeight = static_cast<double>(localInteractions + remote.interactions);
                double localWeight = static_cast<double>(localInteractions) / totalWeight;
                double remoteWeight = static_cast<double>(remote.interactions) / totalWeight;

                local.score = clampScore(
                    local.score * localWeight + remote.score * remoteWeight);
            }
            local.lastUpdate = std::chrono::steady_clock::now();
        }
    }
}



// ---------------------------------------------------------------------------
//  persist — save ratings to Configuration
// ---------------------------------------------------------------------------

bool
AlpineRatingEngine::persist ()
{
#ifdef _VERBOSE
    Log::Debug ("AlpineRatingEngine::persist invoked.");
#endif

    ReadLock lock(lock_s);

    string serialized;

    for (const auto & [peerId, rating] : ratings_s) {
        if (!serialized.empty())
            serialized += ";";

        std::ostringstream oss;
        oss << peerId << ":"
            << std::fixed << std::setprecision(6) << rating.score << ":"
            << rating.successCount << ":"
            << rating.failureCount;
        serialized += oss.str();
    }

    return Configuration::setValue("rating.peerScores"s, serialized);
}



// ---------------------------------------------------------------------------
//  load — restore ratings from Configuration
// ---------------------------------------------------------------------------

bool
AlpineRatingEngine::load ()
{
#ifdef _VERBOSE
    Log::Debug ("AlpineRatingEngine::load invoked.");
#endif

    string serialized;

    if (!Configuration::getValue("rating.peerScores"s, serialized))
        return false;

    if (serialized.empty())
        return true;

    WriteLock lock(lock_s);
    ratings_s.clear();

    // Format: peerId:score:successCount:failureCount;...
    size_t pos = 0;
    while (pos < serialized.size()) {
        size_t end = serialized.find(';', pos);
        if (end == string::npos)
            end = serialized.size();

        auto entry = serialized.substr(pos, end - pos);
        pos = end + 1;

        // Parse peerId:score:successCount:failureCount
        size_t c1 = entry.find(':');
        if (c1 == string::npos) continue;
        size_t c2 = entry.find(':', c1 + 1);
        if (c2 == string::npos) continue;
        size_t c3 = entry.find(':', c2 + 1);
        if (c3 == string::npos) continue;

        try {
            ulong  peerId       = std::stoul(entry.substr(0, c1));
            double score        = std::stod(entry.substr(c1 + 1, c2 - c1 - 1));
            ulong  successCount = std::stoul(entry.substr(c2 + 1, c3 - c2 - 1));
            ulong  failureCount = std::stoul(entry.substr(c3 + 1));

            t_PeerRating rating;
            rating.score        = clampScore(score);
            rating.successCount = successCount;
            rating.failureCount = failureCount;
            rating.lastUpdate   = std::chrono::steady_clock::now();

            ratings_s[peerId] = rating;
        }
        catch (...) {
            Log::Error("AlpineRatingEngine::load — failed to parse entry: "s + entry);
        }
    }

    Log::Info("AlpineRatingEngine::load — restored "s + std::to_string(ratings_s.size()) + " peer ratings."s);
    return true;
}


