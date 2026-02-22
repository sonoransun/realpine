/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <HttpRouter.h>
#include <Log.h>


// Ctor defaulted in header

// Dtor defaulted in header


void
HttpRouter::addRoute (const string &  method,
                      const string &  pattern,
                      RouteHandler    handler)
{
    Route route;
    route.method  = method;
    route.pattern = pattern;
    route.handler = handler;

    routes_.push_back(route);
}


void
HttpRouter::setAuthMiddleware (AuthMiddleware middleware)
{
    authMiddleware_ = std::move(middleware);
}


void
HttpRouter::splitPath (const string &     path,
                       vector<string> &   segments)
{
    segments.clear();

    ulong start = 0;

    // Skip leading slash
    if (!path.empty() && path[0] == '/')
        start = 1;

    while (start < path.length())
    {
        ulong end = path.find('/', start);
        if (end == string::npos)
            end = path.length();

        if (end > start)
            segments.push_back(path.substr(start, end - start));

        start = end + 1;
    }
}


bool
HttpRouter::matchRoute (const vector<string> &                  patternParts,
                        const vector<string> &                  pathParts,
                        std::unordered_map<string, string> &    params)
{
    if (patternParts.size() != pathParts.size())
        return false;

    params.clear();

    for (ulong i = 0; i < patternParts.size(); ++i)
    {
        if (!patternParts[i].empty() && patternParts[i][0] == ':')
        {
            // Parameter segment — extract the name without the leading ':'
            string paramName = patternParts[i].substr(1);
            params[paramName] = pathParts[i];
        }
        else
        {
            // Literal segment — must match exactly
            if (patternParts[i] != pathParts[i])
                return false;
        }
    }

    return true;
}


HttpResponse
HttpRouter::dispatch (const HttpRequest & request)
{
    // Handle OPTIONS preflight for CORS
    if (request.method == "OPTIONS")
    {
        return HttpResponse(200, "OK");
    }

    if (authMiddleware_) {
        HttpResponse authResp(401, "Unauthorized");
        if (!authMiddleware_(request, authResp))
            return authResp;
    }

    vector<string> pathParts;
    splitPath(request.path, pathParts);

    bool pathMatched = false;

    for (const auto& route : routes_)
    {
        vector<string> patternParts;
        splitPath(route.pattern, patternParts);

        std::unordered_map<string, string> params;

        if (matchRoute(patternParts, pathParts, params))
        {
            pathMatched = true;

            if (route.method == request.method)
            {
                return route.handler(request, params);
            }
        }
    }

    if (pathMatched)
        return HttpResponse::methodNotAllowed();

    return HttpResponse::notFound();
}
