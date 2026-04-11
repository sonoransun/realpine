/// Unit tests for HttpRouter

#include <HttpRouter.h>
#include <catch2/catch_test_macros.hpp>


static HttpResponse
echoHandler(const HttpRequest & request, const std::unordered_map<string, string> & params)
{
    string json = "{\"path\":\"" + request.path + "\"}";
    return HttpResponse::ok(json);
}


static HttpResponse
paramHandler(const HttpRequest & request, const std::unordered_map<string, string> & params)
{
    auto it = params.find("id");
    string id = (it != params.end()) ? it->second : "none";
    return HttpResponse::ok("{\"id\":\"" + id + "\"}");
}


TEST_CASE("HttpRouter route matching", "[HttpRouter]")
{
    HttpRouter router;
    router.addRoute("GET", "/api/status", echoHandler);

    SECTION("matching route returns handler result")
    {
        HttpRequest req;
        req.method = "GET";
        req.path = "/api/status";

        auto resp = router.dispatch(req);
        string built = resp.build();
        REQUIRE(built.contains("200 OK"));
        REQUIRE(built.contains("/api/status"));
    }

    SECTION("non-matching path returns 404")
    {
        HttpRequest req;
        req.method = "GET";
        req.path = "/api/missing";

        auto resp = router.dispatch(req);
        string built = resp.build();
        REQUIRE(built.contains("404"));
    }
}


TEST_CASE("HttpRouter parameter extraction", "[HttpRouter]")
{
    HttpRouter router;
    router.addRoute("GET", "/item/:id", paramHandler);

    HttpRequest req;
    req.method = "GET";
    req.path = "/item/42";

    auto resp = router.dispatch(req);
    string built = resp.build();

    REQUIRE(built.contains("200 OK"));
    REQUIRE(built.contains("\"id\":\"42\""));
}


TEST_CASE("HttpRouter method not allowed", "[HttpRouter]")
{
    HttpRouter router;
    router.addRoute("GET", "/resource", echoHandler);

    HttpRequest req;
    req.method = "POST";
    req.path = "/resource";

    auto resp = router.dispatch(req);
    string built = resp.build();
    REQUIRE(built.contains("405"));
}


TEST_CASE("HttpRouter not found", "[HttpRouter]")
{
    HttpRouter router;
    router.addRoute("GET", "/a", echoHandler);

    HttpRequest req;
    req.method = "GET";
    req.path = "/b";

    auto resp = router.dispatch(req);
    string built = resp.build();
    REQUIRE(built.contains("404"));
}


TEST_CASE("HttpRouter OPTIONS returns 200 for CORS preflight", "[HttpRouter]")
{
    HttpRouter router;
    router.addRoute("POST", "/api/data", echoHandler);

    HttpRequest req;
    req.method = "OPTIONS";
    req.path = "/api/data";

    auto resp = router.dispatch(req);
    string built = resp.build();
    REQUIRE(built.contains("200 OK"));
}


TEST_CASE("HttpRouter multiple routes", "[HttpRouter]")
{
    HttpRouter router;
    router.addRoute("GET", "/a", echoHandler);
    router.addRoute("GET", "/b", echoHandler);
    router.addRoute("POST", "/a", echoHandler);

    SECTION("GET /a matches first route")
    {
        HttpRequest req;
        req.method = "GET";
        req.path = "/a";

        auto resp = router.dispatch(req);
        REQUIRE(resp.build().contains("200 OK"));
    }

    SECTION("GET /b matches second route")
    {
        HttpRequest req;
        req.method = "GET";
        req.path = "/b";

        auto resp = router.dispatch(req);
        REQUIRE(resp.build().contains("200 OK"));
    }

    SECTION("POST /a matches third route")
    {
        HttpRequest req;
        req.method = "POST";
        req.path = "/a";

        auto resp = router.dispatch(req);
        REQUIRE(resp.build().contains("200 OK"));
    }
}
