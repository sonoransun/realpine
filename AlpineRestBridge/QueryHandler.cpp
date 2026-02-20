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


#include <QueryHandler.h>
#include <AlpineStackInterface.h>
#include <JsonReader.h>
#include <JsonWriter.h>
#include <Log.h>
#include <cstdlib>


void
QueryHandler::registerRoutes (HttpRouter & router)
{
    router.addRoute("POST",   "/query",              startQuery);
    router.addRoute("GET",    "/query/:id",          getQuery);
    router.addRoute("GET",    "/query/:id/results",  getQueryResults);
    router.addRoute("DELETE", "/query/:id",          cancelQuery);
}


HttpResponse
QueryHandler::startQuery (const HttpRequest & request,
                          const std::unordered_map<string, string> & params)
{
    JsonReader reader(request.body);

    string queryString;
    if (!reader.getString("queryString", queryString))
        return HttpResponse::badRequest("Missing queryString");

    AlpineStackInterface::t_QueryOptions options;

    reader.getString("groupName", options.groupName);

    ulong autoHaltLimit = 0;
    if (reader.getUlong("autoHaltLimit", autoHaltLimit))
        options.autoHaltLimit = autoHaltLimit;
    else
        options.autoHaltLimit = 0;

    ulong peerDescMax = 0;
    if (reader.getUlong("peerDescMax", peerDescMax))
        options.peerDescMax = peerDescMax;
    else
        options.peerDescMax = 0;

    options.autoDownload = false;
    options.optionId = 0;

    ulong queryId = 0;

    if (!AlpineStackInterface::startQuery(options, queryString, queryId))
        return HttpResponse::serverError("Failed to start query");

    JsonWriter writer;
    writer.beginObject();
    writer.key("queryId");
    writer.value(queryId);
    writer.endObject();

    return HttpResponse::ok(writer.result());
}


HttpResponse
QueryHandler::getQuery (const HttpRequest & request,
                        const std::unordered_map<string, string> & params)
{
    std::unordered_map<string, string>::const_iterator it = params.find("id");
    if (it == params.end())
        return HttpResponse::badRequest("Missing query id");

    ulong queryId = strtoul(it->second.c_str(), 0, 10);

    bool inProgress = AlpineStackInterface::queryInProgress(queryId);

    AlpineStackInterface::t_QueryStatus status;
    if (!AlpineStackInterface::getQueryStatus(queryId, status))
        return HttpResponse::notFound();

    JsonWriter writer;
    writer.beginObject();
    writer.key("queryId");
    writer.value(queryId);
    writer.key("inProgress");
    writer.value(inProgress);
    writer.key("totalPeers");
    writer.value(status.totalPeers);
    writer.key("peersQueried");
    writer.value(status.peersQueried);
    writer.key("numPeerResponses");
    writer.value(status.numPeerResponses);
    writer.key("totalHits");
    writer.value(status.totalHits);
    writer.endObject();

    return HttpResponse::ok(writer.result());
}


HttpResponse
QueryHandler::getQueryResults (const HttpRequest & request,
                               const std::unordered_map<string, string> & params)
{
    std::unordered_map<string, string>::const_iterator it = params.find("id");
    if (it == params.end())
        return HttpResponse::badRequest("Missing query id");

    ulong queryId = strtoul(it->second.c_str(), 0, 10);

    AlpineStackInterface::t_PeerResourcesIndex results;
    if (!AlpineStackInterface::getQueryResults(queryId, results))
        return HttpResponse::notFound();

    JsonWriter writer;
    writer.beginObject();
    writer.key("queryId");
    writer.value(queryId);
    writer.key("peers");
    writer.beginArray();

    for (const auto& result : results)
    {
        writer.beginObject();
        writer.key("peerId");
        writer.value(result.first);
        writer.key("resources");
        writer.beginArray();

        for (const auto& res : result.second.resourceDescList)
        {
            writer.beginObject();
            writer.key("resourceId");
            writer.value(res.resourceId);
            writer.key("size");
            writer.value(res.size);
            writer.key("description");
            writer.value(res.description);
            writer.key("locators");
            writer.beginArray();

            for (const auto& loc : res.locators)
            {
                writer.value(loc);
            }

            writer.endArray();
            writer.endObject();
        }

        writer.endArray();
        writer.endObject();
    }

    writer.endArray();
    writer.endObject();

    return HttpResponse::ok(writer.result());
}


HttpResponse
QueryHandler::cancelQuery (const HttpRequest & request,
                           const std::unordered_map<string, string> & params)
{
    std::unordered_map<string, string>::const_iterator it = params.find("id");
    if (it == params.end())
        return HttpResponse::badRequest("Missing query id");

    ulong queryId = strtoul(it->second.c_str(), 0, 10);

    if (!AlpineStackInterface::cancelQuery(queryId))
        return HttpResponse::notFound();

    JsonWriter writer;
    writer.beginObject();
    writer.key("cancelled");
    writer.value(true);
    writer.key("queryId");
    writer.value(queryId);
    writer.endObject();

    return HttpResponse::ok(writer.result());
}
