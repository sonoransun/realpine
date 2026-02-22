/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <HttpRequest.h>
#include <HttpResponse.h>
#include <HttpRouter.h>


class QueryHandler
{
  public:

    static void  registerRoutes (HttpRouter & router);


  private:

    static HttpResponse  startQuery (const HttpRequest & request,
                                     const std::unordered_map<string, string> & params);

    static HttpResponse  getQuery (const HttpRequest & request,
                                   const std::unordered_map<string, string> & params);

    static HttpResponse  getQueryResults (const HttpRequest & request,
                                          const std::unordered_map<string, string> & params);

    static HttpResponse  cancelQuery (const HttpRequest & request,
                                      const std::unordered_map<string, string> & params);

};
