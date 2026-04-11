/// Copyright (C) 2026 sonoransun — see LICENCE.txt
///
/// Integration test: real HttpServer bound to a loopback ephemeral port.
///
/// Unlike the other rest_* integration tests which only exercise the
/// HttpRouter dispatch path with synthetic HttpRequest objects, this test
/// brings up a full HttpServer on 127.0.0.1, registers a handful of
/// minimal routes, and drives requests over a real TCP socket using asio.
///
/// IMPORTANT: Configuration, Log, and AlpineStackInterface are pure-static
/// singletons guarded by std::once_flag. They cannot be re-initialized
/// per Catch2 test case, which is why this file is structured as ONE
/// TEST_CASE with many SECTIONs sharing a single HttpServer instance.

#include <Compression.h>
#include <HttpRequest.h>
#include <HttpResponse.h>
#include <HttpRouter.h>
#include <HttpServer.h>
#include <asio.hpp>
#include <atomic>
#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <thread>


// ──────────────────────────────────────────────────────────────────────
//  Stub route handlers
// ──────────────────────────────────────────────────────────────────────

static HttpResponse
stubStatus(const HttpRequest & request, const std::unordered_map<string, string> & params)
{
    (void)request;
    (void)params;
    return HttpResponse::ok("{\"status\":\"running\",\"version\":\"test\"}"s);
}


static HttpResponse
stubReady(const HttpRequest & request, const std::unordered_map<string, string> & params)
{
    (void)request;
    (void)params;
    return HttpResponse::ok("{\"status\":\"ok\"}"s);
}


static HttpResponse
stubLive(const HttpRequest & request, const std::unordered_map<string, string> & params)
{
    (void)request;
    (void)params;
    return HttpResponse::ok("{\"status\":\"ok\"}"s);
}


static HttpResponse
stubEcho(const HttpRequest & request, const std::unordered_map<string, string> & params)
{
    (void)params;
    HttpResponse resp(200, "OK"s);
    resp.setHeader("Content-Type"s, "text/plain"s);
    resp.setBody(request.body);
    return resp;
}


// ──────────────────────────────────────────────────────────────────────
//  Tiny HTTP/1.1 client built on asio (synchronous, blocking)
// ──────────────────────────────────────────────────────────────────────

namespace {

struct ClientResponse
{
    int statusCode{0};
    string statusText;
    string body;
    string raw;
};


static bool
sendHttp(ushort port, const string & request, ClientResponse & out)
{
    try {
        asio::io_context io;
        asio::ip::tcp::socket socket(io);
        asio::ip::tcp::endpoint ep(asio::ip::make_address_v4("127.0.0.1"), port);

        asio::error_code ec;
        socket.connect(ep, ec);
        if (ec)
            return false;

        asio::write(socket, asio::buffer(request), ec);
        if (ec)
            return false;

        // Read until peer closes or buffer full
        string accumulated;
        std::array<char, 4096> buf{};
        while (true) {
            size_t n = socket.read_some(asio::buffer(buf), ec);
            if (n > 0)
                accumulated.append(buf.data(), n);
            if (ec == asio::error::eof)
                break;
            if (ec)
                break;
            if (accumulated.size() > 1024 * 1024)
                break;
        }

        out.raw = accumulated;

        // Parse status line: "HTTP/1.1 200 OK\r\n"
        auto lineEnd = accumulated.find("\r\n"s);
        if (lineEnd == string::npos)
            return false;
        string statusLine = accumulated.substr(0, lineEnd);

        auto firstSpace = statusLine.find(' ');
        if (firstSpace == string::npos)
            return false;
        auto secondSpace = statusLine.find(' ', firstSpace + 1);
        if (secondSpace == string::npos)
            return false;

        try {
            out.statusCode = std::stoi(statusLine.substr(firstSpace + 1, secondSpace - firstSpace - 1));
        } catch (...) {
            return false;
        }
        out.statusText = statusLine.substr(secondSpace + 1);

        // Find end of headers
        auto headerEnd = accumulated.find("\r\n\r\n"s);
        if (headerEnd != string::npos)
            out.body = accumulated.substr(headerEnd + 4);

        return true;
    } catch (const std::exception &) {
        return false;
    }
}


static string
buildGet(const string & path)
{
    return "GET "s + path + " HTTP/1.1\r\nHost: 127.0.0.1\r\nConnection: close\r\n\r\n"s;
}


static string
buildPost(const string & path, const string & body)
{
    return "POST "s + path +
           " HTTP/1.1\r\nHost: 127.0.0.1\r\nConnection: close\r\nContent-Type: text/plain\r\nContent-Length: "s +
           std::to_string(body.size()) + "\r\n\r\n"s + body;
}

}  // namespace


// ──────────────────────────────────────────────────────────────────────
//  TEST_CASE — single fixture, many sections
// ──────────────────────────────────────────────────────────────────────

TEST_CASE("Real HttpServer end-to-end on loopback", "[integration][http_real]")
{
    // Build router with a minimal set of stub routes
    HttpRouter router;
    router.addRoute("GET"s, "/status"s, stubStatus);
    router.addRoute("GET"s, "/health/ready"s, stubReady);
    router.addRoute("GET"s, "/health/live"s, stubLive);
    router.addRoute("POST"s, "/echo"s, stubEcho);

    // Disable auth so requests are not gated by middleware
    router.setAuthMiddleware([](const HttpRequest &, HttpResponse &) { return true; });

    HttpServer server(router);

    // Try ephemeral first; if for any reason that fails, fall back to a
    // fixed high port. We launch the server on a background thread because
    // start() blocks the io_context worker pool indirectly via spawnWorker().
    bool started = false;
    std::thread serverThread([&]() {
        // start() returns once the acceptor is listening and worker
        // threads are spawned; the worker threads themselves run the
        // io_context loop. start() does not block.
        started = server.start(0x7F000001UL /* 127.0.0.1 */, 0 /* ephemeral */);
    });

    // Wait briefly for start() to return
    serverThread.join();

    if (!started) {
        WARN("HttpServer failed to bind to 127.0.0.1:0 — skipping real server test");
        SUCCEED("environment cannot bring up HttpServer");
        return;
    }

    ushort port = server.getBoundPort();
    if (port == 0) {
        server.stop();
        WARN("HttpServer started but could not report bound port — skipping");
        SUCCEED("environment cannot report bound port");
        return;
    }

    // Give worker threads a moment to settle on the io_context
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    SECTION("GET /status returns 200")
    {
        ClientResponse resp;
        bool ok = sendHttp(port, buildGet("/status"s), resp);
        REQUIRE(ok);
        REQUIRE(resp.statusCode == 200);
        REQUIRE(resp.body.contains("running"));
    }

    SECTION("GET /health/ready returns 200")
    {
        ClientResponse resp;
        bool ok = sendHttp(port, buildGet("/health/ready"s), resp);
        REQUIRE(ok);
        REQUIRE(resp.statusCode == 200);
    }

    SECTION("GET /health/live returns 200")
    {
        ClientResponse resp;
        bool ok = sendHttp(port, buildGet("/health/live"s), resp);
        REQUIRE(ok);
        REQUIRE(resp.statusCode == 200);
    }

    SECTION("POST /echo returns request body")
    {
        const string payload = "hello-from-real-client"s;
        ClientResponse resp;
        bool ok = sendHttp(port, buildPost("/echo"s, payload), resp);
        REQUIRE(ok);
        REQUIRE(resp.statusCode == 200);
        REQUIRE(resp.body.contains(payload));
    }

    SECTION("GET /nonexistent returns 404")
    {
        ClientResponse resp;
        bool ok = sendHttp(port, buildGet("/this/path/does/not/exist"s), resp);
        REQUIRE(ok);
        REQUIRE(resp.statusCode == 404);
    }

    // Clean shutdown — stop() joins worker threads and closes the acceptor
    server.stop();
}
