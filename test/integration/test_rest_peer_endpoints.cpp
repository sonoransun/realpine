/// Copyright (C) 2026 sonoransun — see LICENCE.txt
///
/// Integration test: REST API peer endpoints
///
/// Tests the peer handler routing, parameter extraction, and response
/// format for peer list and peer-by-id endpoints.

#include <HttpRequest.h>
#include <HttpResponse.h>
#include <HttpRouter.h>
#include <catch2/catch_test_macros.hpp>


static HttpResponse
stubGetAllPeers(const HttpRequest & request, const std::unordered_map<string, string> & params)
{
    string json = "{\"data\":["
                  "{\"peerId\":1,\"ipAddress\":\"192.168.1.10\",\"port\":9000},"
                  "{\"peerId\":2,\"ipAddress\":\"192.168.1.11\",\"port\":9000}"
                  "],\"total\":2,\"limit\":100,\"offset\":0}";
    return HttpResponse::ok(json);
}


static HttpResponse
stubGetPeer(const HttpRequest & request, const std::unordered_map<string, string> & params)
{
    auto it = params.find("id");
    if (it == params.end())
        return HttpResponse::badRequest("Missing peer id");

    if (it->second == "999")
        return HttpResponse::notFound();

    string json = "{\"peerId\":"s + it->second + ",\"ipAddress\":\"192.168.1.10\",\"port\":9000}";
    return HttpResponse::ok(json);
}


static void
setupPeerRouter(HttpRouter & router)
{
    router.addRoute("GET", "/peers", stubGetAllPeers);
    router.addRoute("GET", "/peers/:id", stubGetPeer);
}


TEST_CASE("Peer endpoints: list all peers", "[integration][peer]")
{
    HttpRouter router;
    setupPeerRouter(router);

    HttpRequest req;
    req.method = "GET";
    req.path = "/peers";

    auto resp = router.dispatch(req);
    string built = resp.build();

    REQUIRE(built.contains("200 OK"));
    REQUIRE(built.contains("\"data\":["));
    REQUIRE(built.contains("\"total\":2"));
}


TEST_CASE("Peer endpoints: get peer by id", "[integration][peer]")
{
    HttpRouter router;
    setupPeerRouter(router);

    HttpRequest req;
    req.method = "GET";
    req.path = "/peers/1";

    auto resp = router.dispatch(req);
    string built = resp.build();

    REQUIRE(built.contains("200 OK"));
    REQUIRE(built.contains("\"peerId\":1"));
    REQUIRE(built.contains("ipAddress"));
}


TEST_CASE("Peer endpoints: peer not found", "[integration][peer]")
{
    HttpRouter router;
    setupPeerRouter(router);

    HttpRequest req;
    req.method = "GET";
    req.path = "/peers/999";

    auto resp = router.dispatch(req);
    string built = resp.build();

    REQUIRE(built.contains("404"));
}


TEST_CASE("Peer endpoints: wrong method", "[integration][peer]")
{
    HttpRouter router;
    setupPeerRouter(router);

    HttpRequest req;
    req.method = "DELETE";
    req.path = "/peers";

    auto resp = router.dispatch(req);
    string built = resp.build();

    REQUIRE(built.contains("405"));
}
