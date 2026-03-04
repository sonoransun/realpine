/// Unit tests for HttpResponse

#include <catch2/catch_test_macros.hpp>
#include <HttpResponse.h>


TEST_CASE("HttpResponse factory methods", "[HttpResponse]")
{
    SECTION("ok() sets 200 status and JSON body")
    {
        auto resp = HttpResponse::ok("{\"status\":\"ok\"}");
        string built = resp.build();

        REQUIRE(built.contains("HTTP/1.1 200 OK"));
        REQUIRE(built.contains("application/json"));
        REQUIRE(built.contains("{\"status\":\"ok\"}"));
    }

    SECTION("notFound() sets 404 status")
    {
        auto resp = HttpResponse::notFound();
        string built = resp.build();

        REQUIRE(built.contains("HTTP/1.1 404 Not Found"));
        REQUIRE(built.contains("\"error\":\"Not Found\""));
    }

    SECTION("badRequest() sets 400 status with message")
    {
        auto resp = HttpResponse::badRequest("missing field");
        string built = resp.build();

        REQUIRE(built.contains("HTTP/1.1 400 Bad Request"));
        REQUIRE(built.contains("missing field"));
    }

    SECTION("methodNotAllowed() sets 405 status")
    {
        auto resp = HttpResponse::methodNotAllowed();
        string built = resp.build();

        REQUIRE(built.contains("HTTP/1.1 405 Method Not Allowed"));
    }

    SECTION("serverError() sets 500 status with message")
    {
        auto resp = HttpResponse::serverError("something broke");
        string built = resp.build();

        REQUIRE(built.contains("HTTP/1.1 500 Internal Server Error"));
        REQUIRE(built.contains("something broke"));
    }
}


TEST_CASE("HttpResponse header setting", "[HttpResponse]")
{
    HttpResponse resp(200, "OK");
    resp.setHeader("X-Test", "hello");
    string built = resp.build();

    REQUIRE(built.contains("X-Test: hello"));
}


TEST_CASE("HttpResponse body building", "[HttpResponse]")
{
    SECTION("setBody sets plain body")
    {
        HttpResponse resp(200, "OK");
        resp.setBody("plain text");
        string built = resp.build();

        REQUIRE(built.contains("plain text"));
        REQUIRE(built.contains("Content-Length: 10"));
    }

    SECTION("setJsonBody sets content-type and body")
    {
        HttpResponse resp(200, "OK");
        resp.setJsonBody("{\"key\":\"val\"}");
        string built = resp.build();

        REQUIRE(built.contains("application/json"));
        REQUIRE(built.contains("{\"key\":\"val\"}"));
    }

    SECTION("empty body produces Content-Length: 0")
    {
        HttpResponse resp(204, "No Content");
        string built = resp.build();

        REQUIRE(built.contains("Content-Length: 0"));
    }
}


TEST_CASE("HttpResponse includes CORS and security headers", "[HttpResponse]")
{
    HttpResponse resp(200, "OK");
    string built = resp.build();

    REQUIRE(built.contains("Access-Control-Allow-Methods"));
    REQUIRE(built.contains("Access-Control-Allow-Headers"));
    REQUIRE(built.contains("X-Content-Type-Options: nosniff"));
    REQUIRE(built.contains("X-Frame-Options: DENY"));
    REQUIRE(built.contains("Connection: close"));
}


TEST_CASE("HttpResponse build format", "[HttpResponse]")
{
    HttpResponse resp(200, "OK");
    resp.setBody("data");
    string built = resp.build();

    // Status line followed by CRLF
    REQUIRE(built.starts_with("HTTP/1.1 200 OK\r\n"));

    // Blank line separates headers from body
    REQUIRE(built.contains("\r\n\r\ndata"));
}
