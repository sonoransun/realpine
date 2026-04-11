/// Copyright (C) 2026 sonoransun — see LICENCE.txt
///
/// Integration test: REST API authentication flow
///
/// Tests the API key authentication middleware integrated with the
/// HTTP router, verifying that auth is enforced and that valid keys
/// pass through.

#include <ApiKeyAuth.h>
#include <HttpRequest.h>
#include <HttpResponse.h>
#include <HttpRouter.h>
#include <catch2/catch_test_macros.hpp>


static HttpResponse
protectedHandler(const HttpRequest & request, const std::unordered_map<string, string> & params)
{
    return HttpResponse::ok("{\"message\":\"authorized\"}");
}


static void
setupAuthRouter(HttpRouter & router)
{
    ApiKeyAuth::initialize();

    router.setAuthMiddleware(
        [](const HttpRequest & req, HttpResponse & resp) -> bool { return ApiKeyAuth::validate(req, resp); });

    router.addRoute("GET", "/protected", protectedHandler);
}


TEST_CASE("Auth flow: request without API key is rejected", "[integration][auth]")
{
    HttpRouter router;
    setupAuthRouter(router);

    HttpRequest req;
    req.method = "GET";
    req.path = "/protected";

    auto resp = router.dispatch(req);
    string built = resp.build();

    REQUIRE(built.contains("401"));
}


TEST_CASE("Auth flow: request with valid API key succeeds", "[integration][auth]")
{
    HttpRouter router;
    setupAuthRouter(router);

    HttpRequest req;
    req.method = "GET";
    req.path = "/protected";
    req.headers["Authorization"] = "Bearer "s + ApiKeyAuth::getKey();

    auto resp = router.dispatch(req);
    string built = resp.build();

    REQUIRE(built.contains("200 OK"));
    REQUIRE(built.contains("authorized"));
}


TEST_CASE("Auth flow: request with wrong API key is rejected", "[integration][auth]")
{
    HttpRouter router;
    setupAuthRouter(router);

    HttpRequest req;
    req.method = "GET";
    req.path = "/protected";
    req.headers["Authorization"] = "Bearer invalid-key-12345";

    auto resp = router.dispatch(req);
    string built = resp.build();

    REQUIRE(built.contains("401"));
}


TEST_CASE("Auth flow: OPTIONS request bypasses auth (CORS preflight)", "[integration][auth]")
{
    HttpRouter router;
    setupAuthRouter(router);

    HttpRequest req;
    req.method = "OPTIONS";
    req.path = "/protected";

    auto resp = router.dispatch(req);
    string built = resp.build();

    REQUIRE(built.contains("200 OK"));
}
