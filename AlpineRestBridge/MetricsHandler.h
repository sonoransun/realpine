/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <HttpRequest.h>
#include <HttpResponse.h>
#include <HttpRouter.h>
#include <atomic>
#include <mutex>
#include <unordered_map>


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

    static string  serialize ();


  private:

    static std::mutex                                  mutex_s;
    static std::unordered_map<string, long long>       counters_s;
    static std::unordered_map<string, double>          gauges_s;
    static std::unordered_map<string, long long>       labeledCounters_s;

};



class MetricsHandler
{
  public:

    static void  registerRoutes (HttpRouter & router);


  private:

    static HttpResponse  getMetrics (const HttpRequest & request,
                                     const std::unordered_map<string, string> & params);

};
