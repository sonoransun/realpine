/// Unit tests for ApiKeyAuth

#include <catch2/catch_test_macros.hpp>
#include <ApiKeyAuth.h>
#include <chrono>


TEST_CASE("ApiKeyAuth initialization and validation", "[ApiKeyAuth]")
{
    // Initialize generates or loads a key
    ApiKeyAuth::initialize();
    const string key = ApiKeyAuth::getKey();
    REQUIRE_FALSE(key.empty());

    SECTION("valid Bearer token passes validation")
    {
        HttpRequest req;
        req.method = "POST";
        req.path = "/query/start";
        req.headers["authorization"] = "Bearer " + key;

        HttpResponse resp(401, "Unauthorized");
        REQUIRE(ApiKeyAuth::validate(req, resp));
    }

    SECTION("wrong key fails validation")
    {
        HttpRequest req;
        req.method = "POST";
        req.path = "/query/start";
        req.headers["authorization"] = "Bearer wrong-key-value";

        HttpResponse resp(401, "Unauthorized");
        REQUIRE_FALSE(ApiKeyAuth::validate(req, resp));
    }

    SECTION("missing authorization header fails")
    {
        HttpRequest req;
        req.method = "POST";
        req.path = "/query/start";

        HttpResponse resp(401, "Unauthorized");
        REQUIRE_FALSE(ApiKeyAuth::validate(req, resp));
    }

    SECTION("GET /status is exempt from auth")
    {
        HttpRequest req;
        req.method = "GET";
        req.path = "/status";

        HttpResponse resp(401, "Unauthorized");
        REQUIRE(ApiKeyAuth::validate(req, resp));
    }

    SECTION("GET /status/details is also exempt")
    {
        HttpRequest req;
        req.method = "GET";
        req.path = "/status/details";

        HttpResponse resp(401, "Unauthorized");
        REQUIRE(ApiKeyAuth::validate(req, resp));
    }
}


TEST_CASE("ApiKeyAuth constantTimeCompare via validate", "[ApiKeyAuth]")
{
    ApiKeyAuth::initialize();
    const string key = ApiKeyAuth::getKey();

    SECTION("equal strings pass")
    {
        HttpRequest req;
        req.method = "POST";
        req.path = "/test";
        req.headers["authorization"] = "Bearer " + key;

        HttpResponse resp(401, "Unauthorized");
        REQUIRE(ApiKeyAuth::validate(req, resp));
    }

    SECTION("different strings fail")
    {
        HttpRequest req;
        req.method = "POST";
        req.path = "/test";
        req.headers["authorization"] = "Bearer aaaa";

        HttpResponse resp(401, "Unauthorized");
        REQUIRE_FALSE(ApiKeyAuth::validate(req, resp));
    }

    SECTION("empty bearer value fails")
    {
        HttpRequest req;
        req.method = "POST";
        req.path = "/test";
        req.headers["authorization"] = "Bearer ";

        HttpResponse resp(401, "Unauthorized");
        REQUIRE_FALSE(ApiKeyAuth::validate(req, resp));
    }

    SECTION("malformed auth header (no Bearer prefix) fails")
    {
        HttpRequest req;
        req.method = "POST";
        req.path = "/test";
        req.headers["authorization"] = key;

        HttpResponse resp(401, "Unauthorized");
        REQUIRE_FALSE(ApiKeyAuth::validate(req, resp));
    }
}


TEST_CASE("ApiKeyAuth key rotation", "[ApiKeyAuth]")
{
    ApiKeyAuth::initialize();
    const string originalKey = ApiKeyAuth::getKey();
    REQUIRE_FALSE(originalKey.empty());

    SECTION("new key works immediately after rotation")
    {
        string newKey = ApiKeyAuth::rotateKey(std::chrono::seconds(3600));
        REQUIRE_FALSE(newKey.empty());
        REQUIRE(newKey != originalKey);

        HttpRequest req;
        req.method = "POST";
        req.path = "/test";
        req.headers["authorization"] = "Bearer " + newKey;

        HttpResponse resp(401, "Unauthorized");
        REQUIRE(ApiKeyAuth::validate(req, resp));
    }

    SECTION("old key still works during grace period")
    {
        string newKey = ApiKeyAuth::rotateKey(std::chrono::seconds(3600));
        REQUIRE_FALSE(newKey.empty());

        // The original key should still validate during grace period
        HttpRequest req;
        req.method = "POST";
        req.path = "/test";
        req.headers["authorization"] = "Bearer " + originalKey;

        HttpResponse resp(401, "Unauthorized");
        REQUIRE(ApiKeyAuth::validate(req, resp));
    }

    SECTION("old key is rejected after zero grace period")
    {
        string newKey = ApiKeyAuth::rotateKey(std::chrono::seconds(0));
        REQUIRE_FALSE(newKey.empty());

        // With zero grace period, the old key should be expired immediately
        HttpRequest req;
        req.method = "POST";
        req.path = "/test";
        req.headers["authorization"] = "Bearer " + originalKey;

        HttpResponse resp(401, "Unauthorized");
        REQUIRE_FALSE(ApiKeyAuth::validate(req, resp));
    }

    SECTION("activeKeyCount reflects current state")
    {
        ulong countBefore = ApiKeyAuth::activeKeyCount();
        REQUIRE(countBefore >= 1);

        // Rotate with a long grace period — both old and new should be active
        ApiKeyAuth::rotateKey(std::chrono::seconds(3600));
        ulong countAfter = ApiKeyAuth::activeKeyCount();
        REQUIRE(countAfter >= 2);
    }
}
