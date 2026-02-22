/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <HttpRequest.h>
#include <HttpResponse.h>
#include <HttpRouter.h>


class PeerHandler
{
  public:

    static void  registerRoutes (HttpRouter & router);


  private:

    static HttpResponse  getAllPeers (const HttpRequest & request,
                                     const std::unordered_map<string, string> & params);

    static HttpResponse  getPeer (const HttpRequest & request,
                                  const std::unordered_map<string, string> & params);

};
