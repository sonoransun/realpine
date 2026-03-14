/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <HttpRequest.h>
#include <HttpResponse.h>
#include <HttpRouter.h>


class AdminHandler
{
  public:

    static void  registerRoutes (HttpRouter & router);


  private:

    static HttpResponse  banPeer (const HttpRequest & request,
                                  const std::unordered_map<string, string> & params);

    static HttpResponse  unbanPeer (const HttpRequest & request,
                                    const std::unordered_map<string, string> & params);

    static HttpResponse  getBannedPeers (const HttpRequest & request,
                                         const std::unordered_map<string, string> & params);

    static HttpResponse  reloadConfig (const HttpRequest & request,
                                       const std::unordered_map<string, string> & params);

    static HttpResponse  flushCache (const HttpRequest & request,
                                     const std::unordered_map<string, string> & params);

    static HttpResponse  getLogLevel (const HttpRequest & request,
                                      const std::unordered_map<string, string> & params);

    static HttpResponse  setLogLevel (const HttpRequest & request,
                                      const std::unordered_map<string, string> & params);

    static HttpResponse  rotateApiKey (const HttpRequest & request,
                                       const std::unordered_map<string, string> & params);

};
