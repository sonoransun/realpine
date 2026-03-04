/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <MetricsHandler.h>
#include <sstream>


std::mutex                                  MetricsRegistry::mutex_s;
std::unordered_map<string, long long>       MetricsRegistry::counters_s;
std::unordered_map<string, double>          MetricsRegistry::gauges_s;
std::unordered_map<string, long long>       MetricsRegistry::labeledCounters_s;



void
MetricsRegistry::incrementCounter (const string & name,
                                   long long      delta)
{
    std::lock_guard<std::mutex> lock(mutex_s);
    counters_s[name] += delta;
}



void
MetricsRegistry::setGauge (const string &  name,
                           double          value)
{
    std::lock_guard<std::mutex> lock(mutex_s);
    gauges_s[name] = value;
}



void
MetricsRegistry::incrementLabeledCounter (const string & name,
                                          const string & labels,
                                          long long      delta)
{
    std::lock_guard<std::mutex> lock(mutex_s);
    string key = name + "{" + labels + "}";
    labeledCounters_s[key] += delta;
}



string
MetricsRegistry::serialize ()
{
    std::lock_guard<std::mutex> lock(mutex_s);
    std::ostringstream oss;

    for (const auto & [name, value] : counters_s) {
        oss << "# TYPE " << name << " counter\n";
        oss << name << " " << value << "\n";
    }

    for (const auto & [name, value] : gauges_s) {
        oss << "# TYPE " << name << " gauge\n";
        oss << name << " " << value << "\n";
    }

    for (const auto & [key, value] : labeledCounters_s) {
        oss << key << " " << value << "\n";
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
