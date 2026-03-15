/// Copyright (C) 2026 sonoransun — see LICENCE.txt

/// Unit tests for WebhookDispatcher

#include <catch2/catch_test_macros.hpp>
#include <WebhookDispatcher.h>
#include <thread>
#include <chrono>


TEST_CASE("WebhookDispatcher URL parsing", "[WebhookDispatcher]")
{
    WebhookDispatcher::t_UrlParts parts;

    SECTION("simple HTTP URL with path")
    {
        REQUIRE(WebhookDispatcher::parseUrl("http://example.com/webhook/callback"s, parts));
        REQUIRE(parts.host == "example.com"s);
        REQUIRE(parts.port == "80"s);
        REQUIRE(parts.path == "/webhook/callback"s);
        REQUIRE_FALSE(parts.useTls);
    }

    SECTION("HTTPS URL with custom port")
    {
        REQUIRE(WebhookDispatcher::parseUrl("https://hooks.example.com:8443/events"s, parts));
        REQUIRE(parts.host == "hooks.example.com"s);
        REQUIRE(parts.port == "8443"s);
        REQUIRE(parts.path == "/events"s);
        REQUIRE(parts.useTls);
    }

    SECTION("HTTP URL with port and no path")
    {
        REQUIRE(WebhookDispatcher::parseUrl("http://localhost:9000"s, parts));
        REQUIRE(parts.host == "localhost"s);
        REQUIRE(parts.port == "9000"s);
        REQUIRE(parts.path == "/"s);
        REQUIRE_FALSE(parts.useTls);
    }

    SECTION("HTTPS URL with default port")
    {
        REQUIRE(WebhookDispatcher::parseUrl("https://api.example.com/hook"s, parts));
        REQUIRE(parts.host == "api.example.com"s);
        REQUIRE(parts.port == "443"s);
        REQUIRE(parts.path == "/hook"s);
        REQUIRE(parts.useTls);
    }

    SECTION("URL with nested path")
    {
        REQUIRE(WebhookDispatcher::parseUrl("http://host:3000/api/v1/webhooks/receive"s, parts));
        REQUIRE(parts.host == "host"s);
        REQUIRE(parts.port == "3000"s);
        REQUIRE(parts.path == "/api/v1/webhooks/receive"s);
    }

    SECTION("invalid URL - no scheme")
    {
        REQUIRE_FALSE(WebhookDispatcher::parseUrl("example.com/webhook"s, parts));
    }

    SECTION("invalid URL - unsupported scheme")
    {
        REQUIRE_FALSE(WebhookDispatcher::parseUrl("ftp://example.com/webhook"s, parts));
    }

    SECTION("invalid URL - empty host")
    {
        REQUIRE_FALSE(WebhookDispatcher::parseUrl("http:///path"s, parts));
    }
}


TEST_CASE("WebhookDispatcher HMAC signature computation", "[WebhookDispatcher]")
{
    SECTION("empty secret returns empty signature")
    {
        WebhookDispatcher::setSecret(""s);
        string sig = WebhookDispatcher::computeHmacSignature("test payload"s);
        REQUIRE(sig.empty());
    }

#ifdef ALPINE_TLS_ENABLED
    SECTION("known HMAC-SHA256 test vector")
    {
        // RFC 4231 Test Case 2:
        // Key  = "Jefe"
        // Data = "what do ya want for nothing?"
        // HMAC-SHA256 = 5bdcc146bf60754e6a042426089575c75a003f089d2739839dec58b964ec3843
        WebhookDispatcher::setSecret("Jefe"s);
        string sig = WebhookDispatcher::computeHmacSignature("what do ya want for nothing?"s);
        REQUIRE(sig == "sha256=5bdcc146bf60754e6a042426089575c75a003f089d2739839dec58b964ec3843"s);
    }

    SECTION("different payloads produce different signatures")
    {
        WebhookDispatcher::setSecret("test-secret"s);
        string sig1 = WebhookDispatcher::computeHmacSignature("payload-one"s);
        string sig2 = WebhookDispatcher::computeHmacSignature("payload-two"s);
        REQUIRE_FALSE(sig1.empty());
        REQUIRE_FALSE(sig2.empty());
        REQUIRE(sig1 != sig2);
    }

    SECTION("signature starts with sha256= prefix")
    {
        WebhookDispatcher::setSecret("my-secret"s);
        string sig = WebhookDispatcher::computeHmacSignature("hello"s);
        REQUIRE(sig.starts_with("sha256="s));
        // sha256= prefix (7 chars) + 64 hex chars = 71 total
        REQUIRE(sig.size() == 71);
    }
#endif

    // Reset secret to avoid interfering with other tests
    WebhookDispatcher::setSecret(""s);
}


TEST_CASE("WebhookDispatcher configuration", "[WebhookDispatcher]")
{
    SECTION("setMaxRetries clamps to valid range")
    {
        WebhookDispatcher::setMaxRetries(5);
        // No direct getter, but verify no crash for boundary values
        WebhookDispatcher::setMaxRetries(0);
        WebhookDispatcher::setMaxRetries(20);
        WebhookDispatcher::setMaxRetries(-1);    // should be ignored
        WebhookDispatcher::setMaxRetries(100);   // should be ignored
    }

    SECTION("setTimeoutSeconds clamps to valid range")
    {
        WebhookDispatcher::setTimeoutSeconds(15);
        WebhookDispatcher::setTimeoutSeconds(1);
        WebhookDispatcher::setTimeoutSeconds(120);
        WebhookDispatcher::setTimeoutSeconds(0);    // should be ignored
        WebhookDispatcher::setTimeoutSeconds(200);   // should be ignored
    }
}


TEST_CASE("WebhookDispatcher dispatch queues work", "[WebhookDispatcher]")
{
    // Dispatch should not crash even when the worker is not running
    // (before initialize or after shutdown)

    SECTION("dispatch before initialize does not crash")
    {
        WebhookDispatcher::dispatch("http://localhost:9999/hook"s,
                                    "{\"test\":true}"s,
                                    "query.completed"s);
    }

    SECTION("initialize and shutdown lifecycle")
    {
        WebhookDispatcher::initialize();

        // Dispatch a webhook to an unreachable host (delivery will fail, but queueing should work)
        WebhookDispatcher::setMaxRetries(0);
        WebhookDispatcher::dispatch("http://192.0.2.1:1/hook"s,
                                    "{\"queryId\":42}"s,
                                    "query.completed"s);

        // Give the worker thread a moment to process
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        WebhookDispatcher::shutdown();
    }

    SECTION("double initialize is safe")
    {
        WebhookDispatcher::initialize();
        WebhookDispatcher::initialize();  // should be no-op
        WebhookDispatcher::shutdown();
    }

    SECTION("double shutdown is safe")
    {
        WebhookDispatcher::initialize();
        WebhookDispatcher::shutdown();
        WebhookDispatcher::shutdown();  // should be no-op
    }
}
