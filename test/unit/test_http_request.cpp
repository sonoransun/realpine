/// Unit tests for HttpRequest::parse

#include <catch2/catch_test_macros.hpp>
#include <HttpRequest.h>


static bool parseString(const string& raw, HttpRequest& req)
{
    return HttpRequest::parse(reinterpret_cast<const byte*>(raw.data()),
                              static_cast<ulong>(raw.size()),
                              req);
}


TEST_CASE("HttpRequest::parse valid GET request", "[HttpRequest]")
{
    string raw = "GET /api/status HTTP/1.1\r\n"
                 "Host: localhost:8080\r\n"
                 "Accept: application/json\r\n"
                 "\r\n";

    HttpRequest req;
    REQUIRE(parseString(raw, req));
    REQUIRE(req.method == "GET");
    REQUIRE(req.path == "/api/status");
    REQUIRE(req.headers.contains("host"));
    REQUIRE(req.headers["host"] == "localhost:8080");
    REQUIRE(req.headers.contains("accept"));
    REQUIRE(req.headers["accept"] == "application/json");
    REQUIRE(req.body.empty());
}


TEST_CASE("HttpRequest::parse valid POST with body", "[HttpRequest]")
{
    string body = "{\"query\":\"test\"}";
    string raw = "POST /query/start HTTP/1.1\r\n"
                 "Content-Type: application/json\r\n"
                 "Content-Length: " + std::to_string(body.size()) + "\r\n"
                 "\r\n" + body;

    HttpRequest req;
    REQUIRE(parseString(raw, req));
    REQUIRE(req.method == "POST");
    REQUIRE(req.path == "/query/start");
    REQUIRE(req.headers.contains("content-type"));
    REQUIRE(req.headers["content-type"] == "application/json");
    REQUIRE(req.body == body);
}


TEST_CASE("HttpRequest::parse malformed input", "[HttpRequest]")
{
    SECTION("null data returns false")
    {
        HttpRequest req;
        REQUIRE_FALSE(HttpRequest::parse(nullptr, 100, req));
    }

    SECTION("zero length returns false")
    {
        byte data[] = {0};
        HttpRequest req;
        REQUIRE_FALSE(HttpRequest::parse(data, 0, req));
    }

    SECTION("no CRLF returns false")
    {
        string raw = "GET /path HTTP/1.1";
        HttpRequest req;
        REQUIRE_FALSE(parseString(raw, req));
    }

    SECTION("missing HTTP version (no second space) returns false")
    {
        string raw = "GET\r\n\r\n";
        HttpRequest req;
        REQUIRE_FALSE(parseString(raw, req));
    }
}


TEST_CASE("HttpRequest::parse missing method", "[HttpRequest]")
{
    // A line like " /path HTTP/1.1" has a leading space, so method is empty
    string raw = " /path HTTP/1.1\r\n\r\n";

    HttpRequest req;
    bool ok = parseString(raw, req);
    // Parses but method is empty
    REQUIRE(ok);
    REQUIRE(req.method.empty());
}


TEST_CASE("HttpRequest::parse empty body", "[HttpRequest]")
{
    string raw = "DELETE /item/42 HTTP/1.1\r\n"
                 "Host: example.com\r\n"
                 "\r\n";

    HttpRequest req;
    REQUIRE(parseString(raw, req));
    REQUIRE(req.method == "DELETE");
    REQUIRE(req.path == "/item/42");
    REQUIRE(req.body.empty());
}


TEST_CASE("HttpRequest::parse headers are case-insensitive", "[HttpRequest]")
{
    string raw = "GET / HTTP/1.1\r\n"
                 "Content-Type: text/html\r\n"
                 "X-Custom-Header: CustomValue\r\n"
                 "\r\n";

    HttpRequest req;
    REQUIRE(parseString(raw, req));

    // All header names are lowercased by the parser
    REQUIRE(req.headers.contains("content-type"));
    REQUIRE(req.headers["content-type"] == "text/html");
    REQUIRE(req.headers.contains("x-custom-header"));
    REQUIRE(req.headers["x-custom-header"] == "CustomValue");
}


TEST_CASE("HttpRequest::parse trims header value whitespace", "[HttpRequest]")
{
    string raw = "GET / HTTP/1.1\r\n"
                 "Host:   example.com\r\n"
                 "\r\n";

    HttpRequest req;
    REQUIRE(parseString(raw, req));
    REQUIRE(req.headers["host"] == "example.com");
}
