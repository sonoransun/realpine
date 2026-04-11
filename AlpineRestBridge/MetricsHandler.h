/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <HttpRequest.h>
#include <HttpResponse.h>
#include <HttpRouter.h>
#include <array>
#include <atomic>
#include <memory>
#include <mutex>
#include <unordered_map>


/// Pre-registered label combination IDs for lock-free labeled counters.
/// Add new entries before Count and update labelNames_s in the .cpp file.
enum class MetricLabel : size_t {
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
    static void incrementCounter(const string & name, long long delta = 1);

    static void setGauge(const string & name, double value);

    static void incrementLabeledCounter(const string & name, const string & labels, long long delta = 1);

    /// Lock-free labeled counter increment via pre-registered enum.
    static void incrementLabeledCounter(MetricLabel label, long long delta = 1);

    /// Record a value into a histogram with default latency buckets.
    static void recordHistogram(const string & name, double value);

    static string serialize();


  private:
    /// Atomic counters keyed by name — registered on first use.
    struct AtomicCounter
    {
        string name;
        std::atomic<long long> value{0};
    };

    /// Atomic gauge keyed by name — registered on first use.
    struct AtomicGauge
    {
        string name;
        std::atomic<double> value{0.0};
    };

    /// Atomic labeled counter keyed by composite key — registered on first use.
    struct AtomicLabeledCounter
    {
        string key;  // "name{labels}"
        std::atomic<long long> value{0};
    };

    /// Histogram with fixed bucket boundaries (thread-safe via atomics).
    struct AtomicHistogram
    {
        string name;
        std::array<double, 11> boundaries = {0.005, 0.01, 0.025, 0.05, 0.1, 0.25, 0.5, 1.0, 2.5, 5.0, 10.0};
        std::array<std::atomic<long long>, 12> bucketCounts{};  // 11 buckets + 1 for +Inf
        std::atomic<double> sum{0.0};
        std::atomic<long long> count{0};
    };

    /// Registry mutex — only held during first registration of a new metric name,
    /// never during increment or serialize.
    static std::mutex registryMutex_s;
    static std::unordered_map<string, std::unique_ptr<AtomicCounter>> counters_s;
    static std::unordered_map<string, std::unique_ptr<AtomicGauge>> gauges_s;
    static std::unordered_map<string, std::unique_ptr<AtomicLabeledCounter>> labeledCounters_s;
    static std::unordered_map<string, std::unique_ptr<AtomicHistogram>> histograms_s;

    /// Pre-registered flat array for enum-based labeled counters (fully lock-free).
    static std::array<std::atomic<long long>, METRIC_LABEL_COUNT> labelArray_s;
    static const std::array<string, METRIC_LABEL_COUNT> labelNames_s;

    static AtomicCounter * findOrCreateCounter(const string & name);
    static AtomicGauge * findOrCreateGauge(const string & name);
    static AtomicLabeledCounter * findOrCreateLabeled(const string & key);
    static AtomicHistogram * findOrCreateHistogram(const string & name);
};


class MetricsHandler
{
  public:
    static void registerRoutes(HttpRouter & router);


  private:
    static HttpResponse getMetrics(const HttpRequest & request, const std::unordered_map<string, string> & params);
};
