/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <HttpRequest.h>
#include <HttpResponse.h>
#include <HttpRouter.h>


class ApiDocsHandler
{
  public:
    static void registerRoutes(HttpRouter & router);


  private:
    static HttpRouter * router_s;

    static HttpResponse getApiDocs(const HttpRequest & request, const std::unordered_map<string, string> & params);
};
