/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AlpineFeedbackAggregator.h>
#include <AlpinePeerProfile.h>
#include <AlpineRatingEngine.h>
#include <EventBus.h>
#include <FsObserver.h>
#include <Log.h>
#include <ResourceStore.h>
#include <StringUtils.h>

#include <cstdlib>


AlpineFeedbackAggregator::AlpineFeedbackAggregator(ResourceStore & store)
    : store_(store)
{}


AlpineFeedbackAggregator::AlpineFeedbackAggregator(ResourceStore & store, const Config & cfg)
    : store_(store),
      cfg_(cfg)
{}


AlpineFeedbackAggregator::~AlpineFeedbackAggregator() = default;


AlpineFeedbackAggregator::Config
AlpineFeedbackAggregator::configFromEnvironment()
{
    Config cfg;

    if (const char * v = std::getenv("ALPINE_FEEDBACK_DWELL_MIN_MS"); v && *v) {
        cfg.dwellMinMs = static_cast<ulong>(std::strtoul(v, nullptr, 10));
    }
    if (const char * v = std::getenv("ALPINE_FEEDBACK_COMPLETION_HIGH"); v && *v) {
        cfg.completionHigh = std::strtod(v, nullptr);
    }
    if (const char * v = std::getenv("ALPINE_FEEDBACK_COMPLETION_LOW"); v && *v) {
        cfg.completionLow = std::strtod(v, nullptr);
    }
    if (const char * v = std::getenv("ALPINE_FEEDBACK_DEBOUNCE_SEC"); v && *v) {
        cfg.debounceSec = static_cast<ulong>(std::strtoul(v, nullptr, 10));
    }

    return cfg;
}


void
AlpineFeedbackAggregator::attach(FsObserver & observer)
{
    observer.setSink([this](const FsEvent & ev) { onEvent(ev); });
}


void
AlpineFeedbackAggregator::onEvent(const FsEvent & event)
{
    totalEvents_.fetch_add(1);

    SessionKey key{event.pid, event.fsPath};

    std::lock_guard<std::mutex> guard(mutex_);

    switch (event.op) {
    case FsEvent::Op::Open: {
        Session s;
        s.openedAt = event.ts;
        s.readCount = 0;
        s.bytesRead = 0;
        sessions_[key] = s;
        break;
    }
    case FsEvent::Op::Read: {
        auto it = sessions_.find(key);
        if (it != sessions_.end()) {
            it->second.readCount++;
            it->second.bytesRead += event.bytes;
        }
        break;
    }
    case FsEvent::Op::Close: {
        auto it = sessions_.find(key);
        Session s;
        if (it != sessions_.end()) {
            s = it->second;
            sessions_.erase(it);
        } else {
            // Close without prior Open (race or attached mid-flight).
            s.openedAt = event.ts;
            s.readCount = event.bytes > 0 ? 1 : 0;
            s.bytesRead = event.bytes;
        }
        finalizeSession(event.pid, event.fsPath, s);
        break;
    }
    }
}


void
AlpineFeedbackAggregator::finalizeSession(int /*pid*/, const string & fsPath, const Session & session)
{
    ulong resourceId = 0;
    ulong peerId = 0;
    if (!store_.resolvePath(fsPath, resourceId, peerId)) {
        // File is not a registered resource — ignore.
        return;
    }

    auto now = std::chrono::steady_clock::now();
    auto dwell = std::chrono::duration_cast<std::chrono::milliseconds>(now - session.openedAt).count();

    // Classification.  Without a reliable file size we approximate completion
    // from read-event count: 0 reads + short dwell = abandoned (Low).
    AlpineRatingEngine::t_ResourceRating rating;
    if (session.readCount == 0 || static_cast<ulong>(dwell) < cfg_.dwellMinMs) {
        rating = AlpineRatingEngine::t_ResourceRating::Low;
    } else if (session.bytesRead > 0 && session.readCount >= 4) {
        // Backends that supply byte counts let us favor High when the read
        // pattern looks like a full consumption.
        rating = AlpineRatingEngine::t_ResourceRating::High;
    } else {
        rating = AlpineRatingEngine::t_ResourceRating::Average;
    }

    if (shouldDebounce(/*uid placeholder*/ 0, resourceId))
        return;

    // The rating engine identifies the peer purely by peerId — it reads the
    // value out of the supplied profile via AlpinePeerProfile::getPeerId and
    // updates ratings_s[peerId].  We construct the profile directly from
    // peerId (matching the pattern in test/unit/test_rating_engine.cpp) so
    // the aggregator is independent of AlpinePeerMgr's initialization order
    // and works in unit tests where PeerMgr is not bootstrapped.
    AlpinePeerProfile profile(peerId);
    short delta = 0;
    if (AlpineRatingEngine::clientResourceEvaluation(&profile, rating, delta)) {
        totalRatings_.fetch_add(1);

        string ratingName = (rating == AlpineRatingEngine::t_ResourceRating::High)  ? "High"s
                            : (rating == AlpineRatingEngine::t_ResourceRating::Low) ? "Low"s
                                                                                    : "Average"s;
        EventBus::publish(t_Event::ResourceAccessed,
                          "peer="s + std::to_string(peerId) + " resource="s + std::to_string(resourceId) + " rating="s +
                              ratingName + " delta="s + std::to_string(delta));
    }
}


bool
AlpineFeedbackAggregator::shouldDebounce(int uid, ulong resourceId)
{
    auto key = (static_cast<uint64_t>(static_cast<uint32_t>(uid)) << 32) | resourceId;
    auto now = std::chrono::steady_clock::now();

    auto it = lastEmit_.find(key);
    if (it != lastEmit_.end()) {
        auto since = std::chrono::duration_cast<std::chrono::seconds>(now - it->second).count();
        if (static_cast<ulong>(since) < cfg_.debounceSec)
            return true;
    }
    lastEmit_[key] = now;
    return false;
}
