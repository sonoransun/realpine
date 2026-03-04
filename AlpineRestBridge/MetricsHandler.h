/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <HttpRequest.h>
#include <HttpResponse.h>
#include <HttpRouter.h>
#include <atomic>
#include <array>
#include <memory>
#include <mutex>
#include <unordered_map>


/// Pre-registered label combination IDs for lock-free labeled counters.
/// Add new entries before Count and update labelNames_s in the .cpp file.
enum class MetricLabel : size_t
{
    HttpRequestGet,
    HttpRequestPost,
    HttpRequestPut,
    HttpRequestDelete,
    HttpRequestOther,
    QueryStarted,
    QueryCompleted,
    QueryFailed,
    PeerConnected,
    PeerDisconnected,
    RateLimited,
    WebSocketOpened,
    WebSocketClosed,
    Count
};

static constexpr size_t METRIC_LABEL_COUNT = static_cast<size_t>(MetricLabel::Count);


class MetricsRegistry
{
  public:

    static void  incrementCounter (const string & name,
                                   long long      delta = 1);

    static void  setGauge (const string &  name,
                           double          value);

    static void  incrementLabeledCounter (const string & name,
                                          const string & labels,
                                          long long      delta = 1);

    /// Lock-free labeled counter increment via pre-registered enum.
    static void  incrementLabeledCounter (MetricLabel  label,
                                          long long    delta = 1);

    static string  serialize ();


  private:

    /// Atomic counters keyed by name — registered on first use.
    struct AtomicCounter {
        string                     name;
        std::atomic<long long>     value{0};
    };

    /// Atomic gauge keyed by name — registered on first use.
    struct AtomicGauge {
        string                     name;
        std::atomic<double>        value{0.0};
    };

    /// Atomic labeled counter keyed by composite key — registered on first use.
    struct AtomicLabeledCounter {
        string                     key;        // "name{labels}"
        std::atomic<long long>     value{0};
    };

    /// Registry mutex — only held during first registration of a new metric name,
    /// never during increment or serialize.
    static std::mutex                                               registryMutex_s;
    static std::unordered_map<string, std::unique_ptr<AtomicCounter>>       counters_s;
    static std::unordered_map<string, std::unique_ptr<AtomicGauge>>         gauges_s;
    static std::unordered_map<string, std::unique_ptr<AtomicLabeledCounter>> labeledCounters_s;

    /// Pre-registered flat array for enum-based labeled counters (fully lock-free).
    static std::array<std::atomic<long long>, METRIC_LABEL_COUNT>   labelArray_s;
    static const std::array<string, METRIC_LABEL_COUNT>             labelNames_s;

    static AtomicCounter *        findOrCreateCounter (const string & name);
    static AtomicGauge *          findOrCreateGauge   (const string & name);
    static AtomicLabeledCounter * findOrCreateLabeled (const string & key);

};



class MetricsHandler
{
  public:

    static void  registerRoutes (HttpRouter & router);


  private:

    static HttpResponse  getMetrics (const HttpRequest & request,
                                     const std::unordered_map<string, string> & params);

};
