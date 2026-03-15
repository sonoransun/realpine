/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <QueryHandler.h>
#include <AlpineStackInterface.h>
#include <JsonReader.h>
#include <JsonWriter.h>
#include <WebhookDispatcher.h>
#include <MutexLock.h>
#include <Log.h>
#include <SafeParse.h>

#ifdef ALPINE_TRACING_ENABLED
#include <Tracing.h>
#endif


std::unordered_map<ulong, string, OptHash<ulong>>  QueryHandler::callbackUrls_s;
Mutex                                                QueryHandler::callbackUrlsMutex_s;


void
QueryHandler::registerRoutes (HttpRouter & router)
{
    router.addRoute("POST",   "/query",              startQuery,         "Start query"s,          true);
    router.addRoute("GET",    "/query/:id",          getQuery,           "Get query status"s,     false);
    router.addRoute("GET",    "/query/:id/results",  getQueryResults,    "Get query results"s,    true);
    router.addRoute("GET",    "/query/:id/stream",   streamQueryResults, "Stream query results"s, true);
    router.addRoute("DELETE", "/query/:id",          cancelQuery,        "Cancel query"s,         false);
}


HttpResponse
QueryHandler::startQuery (const HttpRequest & request,
                          const std::unordered_map<string, string> & params)
{
#ifdef ALPINE_TRACING_ENABLED
    ScopedSpan span("query.start"s);
#endif

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

    // Parse optional priority (0-255, default 128)
    ulong priorityValue = 128;
    if (reader.getUlong("priority", priorityValue)) {
        if (priorityValue > 255)
            return HttpResponse::badRequest("priority must be 0-255");
        options.priority = static_cast<uint8_t>(priorityValue);
    }

    // Parse optional webhook callback URL
    string callbackUrl;
    reader.getString("callbackUrl", callbackUrl);

    auto result = AlpineStackInterface::startQuery2(options, queryString);
    if (!result)
        return HttpResponse::serverError("Failed to start query");

    ulong queryId = *result;

    // If a callback URL was provided, register a result callback for webhook delivery
    if (!callbackUrl.empty()) {
        registerWebhookCallback(queryId, callbackUrl);

        AlpineStackInterface::registerQueryResultCallback(queryId,
            [](ulong qId, ulong pId) {
                onQueryCompleted(qId, pId);
            });
    }

    JsonWriter writer;
    writer.beginObject();
    writer.key("queryId");
    writer.value(queryId);
    writer.key("priority");
    writer.value(static_cast<ulong>(options.priority));
    if (!callbackUrl.empty()) {
        writer.key("callbackUrl");
        writer.value(callbackUrl);
    }
    writer.endObject();

    auto response = HttpResponse::accepted(writer.result());
    response.setHeader("Location", "/query/"s + std::to_string(queryId));
    return response;
}


HttpResponse
QueryHandler::getQuery (const HttpRequest & request,
                        const std::unordered_map<string, string> & params)
{
#ifdef ALPINE_TRACING_ENABLED
    ScopedSpan span("query.get"s);
#endif

    std::unordered_map<string, string>::const_iterator it = params.find("id");
    if (it == params.end())
        return HttpResponse::badRequest("Missing query id");

    auto parsedId = parseUlong(it->second);
    if (!parsedId)
        return HttpResponse::badRequest("Invalid query id");
    ulong queryId = *parsedId;

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
#ifdef ALPINE_TRACING_ENABLED
    ScopedSpan span("query.getResults"s);
#endif

    std::unordered_map<string, string>::const_iterator it = params.find("id");
    if (it == params.end())
        return HttpResponse::badRequest("Missing query id");

    auto parsedId = parseUlong(it->second);
    if (!parsedId)
        return HttpResponse::badRequest("Invalid query id");
    ulong queryId = *parsedId;

    // Parse pagination parameters
    ulong limit  = 100;
    ulong offset = 0;

    auto limitIt = request.queryParams.find("limit");
    if (limitIt != request.queryParams.end()) {
        auto parsed = parseUlong(limitIt->second);
        if (parsed)
            limit = std::min(*parsed, 1000UL);
    }

    auto offsetIt = request.queryParams.find("offset");
    if (offsetIt != request.queryParams.end()) {
        auto parsed = parseUlong(offsetIt->second);
        if (parsed)
            offset = *parsed;
    }

    AlpineStackInterface::t_PeerResourcesIndex results;
    if (!AlpineStackInterface::getQueryResults(queryId, results))
        return HttpResponse::notFound();

    // Flatten all resources across peers for pagination
    struct FlatResult {
        ulong   peerId;
        const AlpineStackInterface::t_ResourceDesc * res;
    };

    vector<FlatResult> allResults;
    for (const auto& result : results)
        for (const auto& res : result.second.resourceDescList)
            allResults.push_back({result.first, &res});

    ulong total = allResults.size();
    ulong startIdx = std::min(offset, total);
    ulong endIdx   = std::min(startIdx + limit, total);
    bool  hasMore  = endIdx < total;

    JsonWriter writer;
    writer.beginObject();
    writer.key("queryId");
    writer.value(queryId);
    writer.key("data");
    writer.beginArray();

    for (ulong i = startIdx; i < endIdx; ++i)
    {
        const auto& entry = allResults[i];

        writer.beginObject();
        writer.key("peerId");
        writer.value(entry.peerId);
        writer.key("resourceId");
        writer.value(entry.res->resourceId);
        writer.key("size");
        writer.value(entry.res->size);
        writer.key("description");
        writer.value(entry.res->description);
        writer.key("locators");
        writer.beginArray();

        for (const auto& loc : entry.res->locators)
            writer.value(loc);

        writer.endArray();
        writer.endObject();
    }

    writer.endArray();
    writer.key("total");
    writer.value(total);
    writer.key("limit");
    writer.value(limit);
    writer.key("offset");
    writer.value(offset);
    writer.key("hasMore");
    writer.value(hasMore);
    writer.endObject();

    return HttpResponse::ok(writer.result());
}


HttpResponse
QueryHandler::cancelQuery (const HttpRequest & request,
                           const std::unordered_map<string, string> & params)
{
#ifdef ALPINE_TRACING_ENABLED
    ScopedSpan span("query.cancel"s);
#endif

    std::unordered_map<string, string>::const_iterator it = params.find("id");
    if (it == params.end())
        return HttpResponse::badRequest("Missing query id");

    auto parsedId = parseUlong(it->second);
    if (!parsedId)
        return HttpResponse::badRequest("Invalid query id");
    ulong queryId = *parsedId;

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



HttpResponse
QueryHandler::streamQueryResults (const HttpRequest & request,
                                   const std::unordered_map<string, string> & params)
{
    auto it = params.find("id");
    if (it == params.end())
        return HttpResponse::badRequest("Missing query id");

    auto parsedId = parseUlong(it->second);
    if (!parsedId)
        return HttpResponse::badRequest("Invalid query id");
    ulong queryId = *parsedId;

    bool inProgress = AlpineStackInterface::queryInProgress(queryId);

    auto resultsResult = AlpineStackInterface::getQueryResults2(queryId);
    if (!resultsResult)
        return HttpResponse::notFound();

    auto & results = *resultsResult;

    // Build per-peer result events — one SSE "result" event per peer with full resource data
    //
    string sseBody;

    for (const auto& [peerId, peerRes] : results) {
        JsonWriter peerWriter;
        peerWriter.beginObject();
        peerWriter.key("queryId");
        peerWriter.value(queryId);
        peerWriter.key("peerId");
        peerWriter.value(peerId);
        peerWriter.key("resources");
        peerWriter.beginArray();

        for (const auto& res : peerRes.resourceDescList) {
            peerWriter.beginObject();
            peerWriter.key("resourceId");
            peerWriter.value(res.resourceId);
            peerWriter.key("size");
            peerWriter.value(res.size);
            peerWriter.key("description");
            peerWriter.value(res.description);
            peerWriter.key("locators");
            peerWriter.beginArray();
            for (const auto& loc : res.locators) {
                peerWriter.value(loc);
            }
            peerWriter.endArray();
            peerWriter.endObject();
        }

        peerWriter.endArray();
        peerWriter.endObject();

        sseBody += "event: result\ndata: "s + peerWriter.result() + "\n\n"s;
    }

    // Append a progress or complete event depending on query state
    //
    if (inProgress) {
        auto statusResult = AlpineStackInterface::getQueryStatus2(queryId);
        AlpineStackInterface::t_QueryStatus status{};
        if (statusResult)
            status = *statusResult;

        JsonWriter statusWriter;
        statusWriter.beginObject();
        statusWriter.key("queryId");
        statusWriter.value(queryId);
        statusWriter.key("totalPeers");
        statusWriter.value(status.totalPeers);
        statusWriter.key("peersQueried");
        statusWriter.value(status.peersQueried);
        statusWriter.key("totalHits");
        statusWriter.value(status.totalHits);
        statusWriter.endObject();

        sseBody += "event: progress\ndata: "s + statusWriter.result() + "\n\n"s;
    }
    else {
        JsonWriter completeWriter;
        completeWriter.beginObject();
        completeWriter.key("queryId");
        completeWriter.value(queryId);
        completeWriter.key("totalPeers");
        completeWriter.value(static_cast<ulong>(results.size()));
        completeWriter.endObject();

        sseBody += "event: complete\ndata: "s + completeWriter.result() + "\n\n"s;
    }

    HttpResponse response(200, "OK");
    response.setHeader("Content-Type", "text/event-stream");
    response.setHeader("Cache-Control", "no-cache");
    response.setHeader("Connection", "keep-alive");
    response.setBody(std::move(sseBody));
    return response;
}



void
QueryHandler::registerWebhookCallback (ulong queryId, const string & callbackUrl)
{
    MutexLock lock(callbackUrlsMutex_s);
    callbackUrls_s[queryId] = callbackUrl;
}



void
QueryHandler::onQueryCompleted (ulong queryId, ulong /* peerId */)
{
    // Check if the query is still in progress — only fire webhook when complete
    if (AlpineStackInterface::queryInProgress(queryId))
        return;

    // Retrieve and remove the callback URL
    string callbackUrl;
    {
        MutexLock lock(callbackUrlsMutex_s);
        auto it = callbackUrls_s.find(queryId);
        if (it == callbackUrls_s.end())
            return;
        callbackUrl = it->second;
        callbackUrls_s.erase(it);
    }

    // Build the results JSON payload
    auto resultsResult = AlpineStackInterface::getQueryResults2(queryId);

    JsonWriter writer;
    writer.beginObject();
    writer.key("event");
    writer.value("query.completed"s);
    writer.key("queryId");
    writer.value(queryId);

    if (resultsResult) {
        auto & results = *resultsResult;

        writer.key("totalPeers");
        writer.value(static_cast<ulong>(results.size()));

        ulong totalHits = 0;
        for (const auto& [pid, peerRes] : results)
            totalHits += peerRes.resourceDescList.size();

        writer.key("totalHits");
        writer.value(totalHits);

        writer.key("data");
        writer.beginArray();

        for (const auto& [pid, peerRes] : results) {
            for (const auto& res : peerRes.resourceDescList) {
                writer.beginObject();
                writer.key("peerId");
                writer.value(pid);
                writer.key("resourceId");
                writer.value(res.resourceId);
                writer.key("size");
                writer.value(res.size);
                writer.key("description");
                writer.value(res.description);
                writer.key("locators");
                writer.beginArray();
                for (const auto& loc : res.locators)
                    writer.value(loc);
                writer.endArray();
                writer.endObject();
            }
        }

        writer.endArray();
    }

    writer.endObject();

    WebhookDispatcher::dispatch(callbackUrl, writer.result(), "query.completed"s);
}
