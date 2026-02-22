/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <HttpRouter.h>


class JsonRpcHandler
{
  public:

    static void  registerRoutes (HttpRouter & router);


  private:

    static HttpResponse  handleRpc (const HttpRequest & request,
                                    const std::unordered_map<string, string> & params);

};
