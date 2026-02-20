///////
///
///  Copyright (C) 2026  sonoransun
///
///  Permission is hereby granted, free of charge, to any person obtaining a copy
///  of this software and associated documentation files (the "Software"), to deal
///  in the Software without restriction, including without limitation the rights
///  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
///  copies of the Software, and to permit persons to whom the Software is
///  furnished to do so, subject to the following conditions:
///
///  The above copyright notice and this permission notice shall be included in all
///  copies or substantial portions of the Software.
///
///  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
///  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
///  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
///  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
///  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
///  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
///  SOFTWARE.
///
///////


#pragma once
#include <Common.h>
#include <HttpRequest.h>
#include <HttpResponse.h>
#include <unordered_map>


typedef HttpResponse (*RouteHandler)(const HttpRequest & request,
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

};
