/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <HttpRequest.h>
#include <HttpResponse.h>
#include <unordered_map>
#include <functional>

using AuthMiddleware = std::function<bool(const HttpRequest&, HttpResponse&)>;


using RouteHandler = HttpResponse (*)(const HttpRequest & request,
                                      const std::unordered_map<string, string> & params);


class HttpRouter
{
  public:

    HttpRouter () = default;
    ~HttpRouter () = default;

    void  addRoute (const string &  method,
                    const string &  pattern,
                    RouteHandler    handler);

    HttpResponse  dispatch (const HttpRequest & request);

    void  setAuthMiddleware (AuthMiddleware middleware);


  private:

    struct Route {
        string        method;
        string        pattern;
        RouteHandler  handler;
    };

    static void  splitPath (const string &     path,
                            vector<string> &   segments);

    static bool  matchRoute (const vector<string> &                  patternParts,
                             const vector<string> &                  pathParts,
                             std::unordered_map<string, string> &    params);

    vector<Route>  routes_;
    AuthMiddleware authMiddleware_;

};
