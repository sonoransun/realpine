/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <MetricsHandler.h>
#include <sstream>
#include <iomanip>


std::mutex                                               MetricsRegistry::registryMutex_s;
std::unordered_map<string, std::unique_ptr<MetricsRegistry::AtomicCounter>>       MetricsRegistry::counters_s;
std::unordered_map<string, std::unique_ptr<MetricsRegistry::AtomicGauge>>         MetricsRegistry::gauges_s;
std::unordered_map<string, std::unique_ptr<MetricsRegistry::AtomicLabeledCounter>> MetricsRegistry::labeledCounters_s;
std::unordered_map<string, std::unique_ptr<MetricsRegistry::AtomicHistogram>>     MetricsRegistry::histograms_s;

std::array<std::atomic<long long>, METRIC_LABEL_COUNT>  MetricsRegistry::labelArray_s{};

const std::array<string, METRIC_LABEL_COUNT>            MetricsRegistry::labelNames_s = {{
    "http_requests_total{method=\"GET\"}"s,
    "http_requests_total{method=\"POST\"}"s,
    "http_requests_total{method=\"PUT\"}"s,
    "http_requests_total{method=\"DELETE\"}"s,
    "http_requests_total{method=\"OTHER\"}"s,
    "queries_total{status=\"started\"}"s,
    "queries_total{status=\"completed\"}"s,
    "queries_total{status=\"failed\"}"s,
    "peers_total{event=\"connected\"}"s,
    "peers_total{event=\"disconnected\"}"s,
    "rate_limited_total{}"s,
    "websocket_sessions_total{event=\"opened\"}"s,
    "websocket_sessions_total{event=\"closed\"}"s,
}};



MetricsRegistry::AtomicCounter *
MetricsRegistry::findOrCreateCounter (const string & name)
{
    // Fast path: check without lock (pointer read is safe once published).
    {
        std::lock_guard lock(registryMutex_s);
        auto it = counters_s.find(name);
        if (it != counters_s.end())
            return it->second.get();

        auto ptr = std::make_unique<AtomicCounter>();
        ptr->name = name;
        auto * raw = ptr.get();
        counters_s[name] = std::move(ptr);
        return raw;
    }
}


MetricsRegistry::AtomicGauge *
MetricsRegistry::findOrCreateGauge (const string & name)
{
    std::lock_guard lock(registryMutex_s);
    auto it = gauges_s.find(name);
    if (it != gauges_s.end())
        return it->second.get();

    auto ptr = std::make_unique<AtomicGauge>();
    ptr->name = name;
    auto * raw = ptr.get();
    gauges_s[name] = std::move(ptr);
    return raw;
}


MetricsRegistry::AtomicLabeledCounter *
MetricsRegistry::findOrCreateLabeled (const string & key)
{
    std::lock_guard lock(registryMutex_s);
    auto it = labeledCounters_s.find(key);
    if (it != labeledCounters_s.end())
        return it->second.get();

    auto ptr = std::make_unique<AtomicLabeledCounter>();
    ptr->key = key;
    auto * raw = ptr.get();
    labeledCounters_s[key] = std::move(ptr);
    return raw;
}



MetricsRegistry::AtomicHistogram *
MetricsRegistry::findOrCreateHistogram (const string & name)
{
    std::lock_guard lock(registryMutex_s);
    auto it = histograms_s.find(name);
    if (it != histograms_s.end())
        return it->second.get();

    auto ptr = std::make_unique<AtomicHistogram>();
    ptr->name = name;
    auto * raw = ptr.get();
    histograms_s[name] = std::move(ptr);
    return raw;
}



void
MetricsRegistry::recordHistogram (const string & name,
                                  double         value)
{
    auto * hist = findOrCreateHistogram(name);

    // Increment all buckets whose boundary >= value (cumulative).
    for (size_t i = 0; i < hist->boundaries.size(); ++i) {
        if (value <= hist->boundaries[i])
            hist->bucketCounts[i].fetch_add(1, std::memory_order_relaxed);
    }
    // +Inf bucket always incremented.
    hist->bucketCounts[hist->boundaries.size()].fetch_add(1, std::memory_order_relaxed);

    // Accumulate sum via CAS loop.
    double oldSum = hist->sum.load(std::memory_order_relaxed);
    while (!hist->sum.compare_exchange_weak(oldSum, oldSum + value,
                                            std::memory_order_relaxed,
                                            std::memory_order_relaxed))
        ;

    hist->count.fetch_add(1, std::memory_order_relaxed);
}



void
MetricsRegistry::incrementCounter (const string & name,
                                   long long      delta)
{
    auto * counter = findOrCreateCounter(name);
    counter->value.fetch_add(delta, std::memory_order_relaxed);
}



void
MetricsRegistry::setGauge (const string &  name,
                           double          value)
{
    auto * gauge = findOrCreateGauge(name);
    gauge->value.store(value, std::memory_order_relaxed);
}



void
MetricsRegistry::incrementLabeledCounter (const string & name,
                                          const string & labels,
                                          long long      delta)
{
    string key = name + "{" + labels + "}";
    auto * counter = findOrCreateLabeled(key);
    counter->value.fetch_add(delta, std::memory_order_relaxed);
}



void
MetricsRegistry::incrementLabeledCounter (MetricLabel  label,
                                          long long    delta)
{
    auto idx = static_cast<size_t>(label);
    labelArray_s[idx].fetch_add(delta, std::memory_order_relaxed);
}



string
MetricsRegistry::serialize ()
{
    std::ostringstream oss;

    // Snapshot counters — lock only to iterate the registry, not the values.
    {
        std::lock_guard lock(registryMutex_s);

        for (const auto & [name, ptr] : counters_s) {
            oss << "# TYPE " << name << " counter\n";
            oss << name << " " << ptr->value.load(std::memory_order_relaxed) << "\n";
        }

        for (const auto & [name, ptr] : gauges_s) {
            oss << "# TYPE " << name << " gauge\n";
            oss << name << " " << ptr->value.load(std::memory_order_relaxed) << "\n";
        }

        for (const auto & [key, ptr] : labeledCounters_s) {
            oss << key << " " << ptr->value.load(std::memory_order_relaxed) << "\n";
        }

        for (const auto & [name, ptr] : histograms_s) {
            oss << "# TYPE " << name << " histogram\n";

            long long cumulative = 0;
            for (size_t i = 0; i < ptr->boundaries.size(); ++i) {
                cumulative += ptr->bucketCounts[i].load(std::memory_order_relaxed);
                oss << name << "_bucket{le=\""
                    << std::fixed << std::setprecision(3) << ptr->boundaries[i]
                    << "\"} " << cumulative << "\n";
            }
            // +Inf bucket
            long long total = ptr->bucketCounts[ptr->boundaries.size()].load(std::memory_order_relaxed);
            oss << name << "_bucket{le=\"+Inf\"} " << total << "\n";

            oss << name << "_sum " << std::fixed << std::setprecision(1)
                << ptr->sum.load(std::memory_order_relaxed) << "\n";
            oss << name << "_count " << ptr->count.load(std::memory_order_relaxed) << "\n";
        }
    }

    // Pre-registered label array — no lock needed.
    for (size_t i = 0; i < METRIC_LABEL_COUNT; ++i) {
        auto val = labelArray_s[i].load(std::memory_order_relaxed);
        if (val != 0)
            oss << labelNames_s[i] << " " << val << "\n";
    }

    return oss.str();
}



void
MetricsHandler::registerRoutes (HttpRouter & router)
{
    router.addRoute("GET", "/metrics", getMetrics);
}



HttpResponse
MetricsHandler::getMetrics (const HttpRequest & request,
                            const std::unordered_map<string, string> & params)
{
    string body = MetricsRegistry::serialize();

    HttpResponse response(200, "OK");
    response.setHeader("Content-Type", "text/plain; version=0.0.4; charset=utf-8");
    response.setBody(body);
    return response;
}
