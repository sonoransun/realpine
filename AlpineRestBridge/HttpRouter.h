/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <HttpRequest.h>
#include <HttpResponse.h>
#include <unordered_map>
#include <functional>
#include <memory>
#include <string_view>

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

    enum class HttpMethod : uint8_t {
        GET,
        POST,
        PUT,
        DELETE_,
        PATCH,
        HEAD,
        OPTIONS,
        UNKNOWN
    };

    struct TrieNode {
        std::unordered_map<string, std::unique_ptr<TrieNode>>  children;
        std::unique_ptr<TrieNode>                              paramChild;
        string                                                 paramName;
        std::unordered_map<HttpMethod, RouteHandler>           handlers;
        bool                                                   requiresAuth = true;
    };

    static HttpMethod  parseMethod (std::string_view method);

    TrieNode root_;
    AuthMiddleware authMiddleware_;

};
