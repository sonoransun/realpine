/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <HttpRequest.h>
#include <HttpResponse.h>
#include <HttpRouter.h>
#include <Mutex.h>
#include <unordered_map>
#include <OptHash.h>


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

    static HttpResponse  streamQueryResults (const HttpRequest & request,
                                              const std::unordered_map<string, string> & params);

    // Webhook callback URL tracking per query
    static void  registerWebhookCallback (ulong queryId, const string & callbackUrl);
    static void  onQueryCompleted (ulong queryId, ulong peerId);

    static std::unordered_map<ulong, string, OptHash<ulong>>  callbackUrls_s;
    static Mutex                                                callbackUrlsMutex_s;

};
