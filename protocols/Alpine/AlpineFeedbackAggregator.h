/// Copyright (C) 2026 sonoransun — see LICENCE.txt
///
/// Bridges the FsObserve event stream into AlpineRatingEngine.
///
/// Per-(pid, fsPath) sessions are opened on FsEvent::Op::Open, accumulate
/// read activity from FsEvent::Op::Read, and finalize on FsEvent::Op::Close.
/// The aggregator computes dwell time and completion ratio, classifies the
/// session as Low / Average / High engagement, then submits a rating delta
/// for the resource's serving peer.  Successful submissions also publish a
/// `t_Event::ResourceAccessed` notification on EventBus for downstream
/// consumers (cluster gossip, audit log).
///
/// Configuration tunables are read from environment at construction time:
///   ALPINE_FEEDBACK_DWELL_MIN_MS    (default 500)
///   ALPINE_FEEDBACK_COMPLETION_HIGH (default 0.9)
///   ALPINE_FEEDBACK_COMPLETION_LOW  (default 0.1)
///   ALPINE_FEEDBACK_DEBOUNCE_SEC    (default 5)


#pragma once
#include <Common.h>
#include <FsObserver.h>
#include <atomic>
#include <chrono>
#include <mutex>
#include <unordered_map>


class FsObserver;
class ResourceStore;


class AlpineFeedbackAggregator
{
  public:
    struct Config
    {
        ulong dwellMinMs{500};
        double completionHigh{0.9};
        double completionLow{0.1};
        ulong debounceSec{5};
    };


    explicit AlpineFeedbackAggregator(ResourceStore & store);
    AlpineFeedbackAggregator(ResourceStore & store, const Config & cfg);
    ~AlpineFeedbackAggregator();


    // Attach to an observer.  The observer must outlive this aggregator.
    //
    void attach(FsObserver & observer);


    // Sink invoked directly by tests; production code goes through attach().
    //
    void onEvent(const FsEvent & event);


    // Counters for diagnostics / tests.
    //
    ulong
    totalEventsObserved() const
    {
        return totalEvents_.load();
    }
    ulong
    totalRatingsSubmitted() const
    {
        return totalRatings_.load();
    }


    static Config configFromEnvironment();


  private:
    struct Session
    {
        std::chrono::steady_clock::time_point openedAt;
        ulong readCount{0};
        ulong bytesRead{0};
    };

    using SessionKey = std::pair<int, string>;
    struct SessionKeyHash
    {
        size_t
        operator()(const SessionKey & k) const noexcept
        {
            return std::hash<int>{}(k.first) ^ (std::hash<string>{}(k.second) << 1);
        }
    };

    void finalizeSession(int pid, const string & fsPath, const Session & session);
    bool shouldDebounce(int uid, ulong resourceId);


    ResourceStore & store_;
    Config cfg_;

    std::mutex mutex_;
    std::unordered_map<SessionKey, Session, SessionKeyHash> sessions_;

    // Per (uid, resourceId) last-emit timestamp, used for debouncing.
    std::unordered_map<uint64_t, std::chrono::steady_clock::time_point> lastEmit_;

    std::atomic<ulong> totalEvents_{0};
    std::atomic<ulong> totalRatings_{0};
};
