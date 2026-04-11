/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <HttpRequest.h>
#include <HttpResponse.h>
#include <HttpRouter.h>


class VfsStatsHandler
{
  public:
    static void registerRoutes(HttpRouter & router);


  private:
    static HttpResponse getStats(const HttpRequest & request, const std::unordered_map<string, string> & params);

    static HttpResponse getPopular(const HttpRequest & request, const std::unordered_map<string, string> & params);

    static HttpResponse getRecent(const HttpRequest & request, const std::unordered_map<string, string> & params);

    static HttpResponse getPeerStats(const HttpRequest & request, const std::unordered_map<string, string> & params);

    static HttpResponse getVfsStatus(const HttpRequest & request, const std::unordered_map<string, string> & params);
};
