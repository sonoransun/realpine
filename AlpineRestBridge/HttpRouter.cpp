/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <HttpRouter.h>
#include <string_view>

using namespace std::string_view_literals;


HttpRouter::HttpMethod
HttpRouter::parseMethod (std::string_view method)
{
    if (method == "GET"sv)     return HttpMethod::GET;
    if (method == "POST"sv)    return HttpMethod::POST;
    if (method == "PUT"sv)     return HttpMethod::PUT;
    if (method == "DELETE"sv)  return HttpMethod::DELETE_;
    if (method == "PATCH"sv)   return HttpMethod::PATCH;
    if (method == "HEAD"sv)    return HttpMethod::HEAD;
    if (method == "OPTIONS"sv) return HttpMethod::OPTIONS;
    return HttpMethod::UNKNOWN;
}


void
HttpRouter::addRoute (const string &  method,
                      const string &  pattern,
                      RouteHandler    handler,
                      const string &  description)
{
    routes_.push_back({method, pattern, description});
    addRoute(method, pattern, handler);
}


const vector<RouteInfo> &
HttpRouter::getRoutes () const
{
    return routes_;
}


void
HttpRouter::addRoute (const string &  method,
                      const string &  pattern,
                      RouteHandler    handler)
{
    auto httpMethod = parseMethod(method);

    TrieNode * node = &root_;

    // Walk pattern segments, building trie nodes as needed
    std::string_view sv(pattern);

    // Skip leading slash
    if (!sv.empty() && sv.front() == '/')
        sv.remove_prefix(1);

    while (!sv.empty())
    {
        auto slash = sv.find('/');
        auto segment = sv.substr(0, slash);

        if (!segment.empty() && segment.front() == ':')
        {
            // Parameterized segment — use wildcard child
            if (!node->paramChild)
            {
                node->paramChild = std::make_unique<TrieNode>();
                node->paramChild->paramName = string(segment.substr(1));
            }
            node = node->paramChild.get();
        }
        else
        {
            // Literal segment
            auto key = string(segment);
            auto it = node->children.find(key);
            if (it == node->children.end())
            {
                auto [inserted, _] = node->children.emplace(std::move(key),
                                                             std::make_unique<TrieNode>());
                node = inserted->second.get();
            }
            else
            {
                node = it->second.get();
            }
        }

        if (slash == std::string_view::npos)
            break;

        sv.remove_prefix(slash + 1);
    }

    node->handlers[httpMethod] = handler;
}


void
HttpRouter::setAuthMiddleware (AuthMiddleware middleware)
{
    authMiddleware_ = std::move(middleware);
}


HttpResponse
HttpRouter::dispatch (const HttpRequest & request)
{
    // Handle OPTIONS preflight for CORS
    if (request.method == "OPTIONS"s)
        return HttpResponse(200, "OK"s);

    auto httpMethod = parseMethod(request.method);

    // Walk the trie using string_view segments — zero allocation
    std::string_view path(request.path);

    // Skip leading slash
    if (!path.empty() && path.front() == '/')
        path.remove_prefix(1);

    // Remove trailing slash
    if (!path.empty() && path.back() == '/')
        path.remove_suffix(1);

    const TrieNode * node = &root_;
    std::unordered_map<string, string> params;
    bool pathMatched = false;

    while (true)
    {
        if (path.empty())
        {
            // Reached the end of the path — check for handlers
            if (!node->handlers.empty())
                pathMatched = true;
            break;
        }

        auto slash = path.find('/');
        auto segment = path.substr(0, slash);

        // Try literal child first
        const TrieNode * next = nullptr;
        auto it = node->children.find(string(segment));
        if (it != node->children.end())
        {
            next = it->second.get();
        }
        else if (node->paramChild)
        {
            // Try parameterized child
            params[node->paramChild->paramName] = string(segment);
            next = node->paramChild.get();
        }

        if (!next)
            break;

        node = next;

        if (slash == std::string_view::npos)
        {
            // Last segment — check for handlers
            if (!node->handlers.empty())
                pathMatched = true;
            break;
        }

        path.remove_prefix(slash + 1);
    }

    if (!pathMatched)
        return HttpResponse::notFound();

    auto handlerIt = node->handlers.find(httpMethod);
    if (handlerIt == node->handlers.end())
        return HttpResponse::methodNotAllowed();

    // Run auth middleware only if set
    if (authMiddleware_)
    {
        HttpResponse authResp(401, "Unauthorized"s);
        if (!authMiddleware_(request, authResp))
            return authResp;
    }

    return handlerIt->second(request, params);
}
