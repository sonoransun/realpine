/// Copyright (C) 2026 sonoransun — see LICENCE.txt
///
/// Integration test: REST API query lifecycle
///
/// Tests the query handler request/response flow through the HTTP router,
/// verifying route matching, parameter extraction, and response format
/// for the full query lifecycle (start -> get -> results -> cancel).

#include <HttpRequest.h>
#include <HttpResponse.h>
#include <HttpRouter.h>
#include <QueryHandler.h>
#include <catch2/catch_test_macros.hpp>


// Stub handler that simulates QueryHandler responses for testing
// the routing and request/response format without the full stack.

static ulong nextQueryId = 1000;

static HttpResponse
stubStartQuery(const HttpRequest & request, const std::unordered_map<string, string> & params)
{
    // Verify body has queryString
    if (request.body.empty())
        return HttpResponse::badRequest("Missing queryString");

    ulong queryId = nextQueryId++;

    string json = "{\"queryId\":"s + std::to_string(queryId) + "}";
    auto response = HttpResponse::accepted(json);
    response.setHeader("Location", "/query/"s + std::to_string(queryId));
    return response;
}


static HttpResponse
stubGetQuery(const HttpRequest & request, const std::unordered_map<string, string> & params)
{
    auto it = params.find("id");
    if (it == params.end())
        return HttpResponse::badRequest("Missing query id");

    string json = "{\"queryId\":"s + it->second + ",\"inProgress\":true,\"totalHits\":0}";
    return HttpResponse::ok(json);
}


static HttpResponse
stubGetResults(const HttpRequest & request, const std::unordered_map<string, string> & params)
{
    auto it = params.find("id");
    if (it == params.end())
        return HttpResponse::badRequest("Missing query id");

    string json = "{\"queryId\":"s + it->second + ",\"peers\":[]}";
    return HttpResponse::ok(json);
}


static HttpResponse
stubCancelQuery(const HttpRequest & request, const std::unordered_map<string, string> & params)
{
    auto it = params.find("id");
    if (it == params.end())
        return HttpResponse::badRequest("Missing query id");

    string json = "{\"cancelled\":true,\"queryId\":"s + it->second + "}";
    return HttpResponse::ok(json);
}


static HttpResponse
stubStreamResults(const HttpRequest & request, const std::unordered_map<string, string> & params)
{
    auto it = params.find("id");
    if (it == params.end())
        return HttpResponse::badRequest("Missing query id");

    string sseBody = "event: complete\ndata: {\"queryId\":"s + it->second + "}\n\n";
    HttpResponse response(200, "OK");
    response.setHeader("Content-Type", "text/event-stream");
    response.setHeader("Cache-Control", "no-cache");
    response.setBody(std::move(sseBody));
    return response;
}


static void
setupQueryRouter(HttpRouter & router)
{
    router.addRoute("POST", "/query", stubStartQuery);
    router.addRoute("GET", "/query/:id", stubGetQuery);
    router.addRoute("GET", "/query/:id/results", stubGetResults);
    router.addRoute("GET", "/query/:id/stream", stubStreamResults);
    router.addRoute("DELETE", "/query/:id", stubCancelQuery);
}


TEST_CASE("Query lifecycle: start query", "[integration][query]")
{
    HttpRouter router;
    setupQueryRouter(router);

    HttpRequest req;
    req.method = "POST";
    req.path = "/query";
    req.body = "{\"queryString\":\"test search\"}";

    auto resp = router.dispatch(req);
    string built = resp.build();

    REQUIRE(built.contains("202"));
    REQUIRE(built.contains("queryId"));
    REQUIRE(built.contains("Location"));
}


TEST_CASE("Query lifecycle: get query status", "[integration][query]")
{
    HttpRouter router;
    setupQueryRouter(router);

    HttpRequest req;
    req.method = "GET";
    req.path = "/query/42";

    auto resp = router.dispatch(req);
    string built = resp.build();

    REQUIRE(built.contains("200 OK"));
    REQUIRE(built.contains("\"queryId\":42"));
    REQUIRE(built.contains("inProgress"));
}


TEST_CASE("Query lifecycle: get query results", "[integration][query]")
{
    HttpRouter router;
    setupQueryRouter(router);

    HttpRequest req;
    req.method = "GET";
    req.path = "/query/42/results";

    auto resp = router.dispatch(req);
    string built = resp.build();

    REQUIRE(built.contains("200 OK"));
    REQUIRE(built.contains("\"queryId\":42"));
    REQUIRE(built.contains("peers"));
}


TEST_CASE("Query lifecycle: cancel query", "[integration][query]")
{
    HttpRouter router;
    setupQueryRouter(router);

    HttpRequest req;
    req.method = "DELETE";
    req.path = "/query/42";

    auto resp = router.dispatch(req);
    string built = resp.build();

    REQUIRE(built.contains("200 OK"));
    REQUIRE(built.contains("\"cancelled\":true"));
    REQUIRE(built.contains("\"queryId\":42"));
}


TEST_CASE("Query lifecycle: stream results (SSE)", "[integration][query]")
{
    HttpRouter router;
    setupQueryRouter(router);

    HttpRequest req;
    req.method = "GET";
    req.path = "/query/42/stream";

    auto resp = router.dispatch(req);
    string built = resp.build();

    REQUIRE(built.contains("200 OK"));
    REQUIRE(built.contains("text/event-stream"));
    REQUIRE(built.contains("event: complete"));
}


TEST_CASE("Query lifecycle: missing query returns 404", "[integration][query]")
{
    HttpRouter router;
    setupQueryRouter(router);

    HttpRequest req;
    req.method = "GET";
    req.path = "/query/nonexistent/bogus";

    auto resp = router.dispatch(req);
    string built = resp.build();

    REQUIRE(built.contains("404"));
}


TEST_CASE("Query lifecycle: wrong method returns 405", "[integration][query]")
{
    HttpRouter router;
    setupQueryRouter(router);

    HttpRequest req;
    req.method = "PUT";
    req.path = "/query";

    auto resp = router.dispatch(req);
    string built = resp.build();

    REQUIRE(built.contains("405"));
}


TEST_CASE("Query lifecycle: start with empty body returns 400", "[integration][query]")
{
    HttpRouter router;
    setupQueryRouter(router);

    HttpRequest req;
    req.method = "POST";
    req.path = "/query";
    req.body = "";

    auto resp = router.dispatch(req);
    string built = resp.build();

    REQUIRE(built.contains("400"));
}
