/// Unit tests for CircuitBreaker

#include <catch2/catch_test_macros.hpp>
#include <CircuitBreaker.h>
#include <thread>
#include <chrono>


TEST_CASE("Default state is Closed for unknown peers", "[CircuitBreaker]")
{
    CircuitBreaker::reset();

    REQUIRE(CircuitBreaker::getState(99999) == CircuitBreaker::State::Closed);
}


TEST_CASE("Unknown peer allows requests", "[CircuitBreaker]")
{
    CircuitBreaker::reset();

    REQUIRE(CircuitBreaker::allowRequest(99999));
}


TEST_CASE("Closed to Open after failure threshold", "[CircuitBreaker]")
{
    CircuitBreaker::reset();

    ulong peerId = 1;

    for (int i = 0; i < 5; ++i) {
        CircuitBreaker::recordFailure(peerId);
    }

    SECTION("state becomes Open")
    {
        REQUIRE(CircuitBreaker::getState(peerId) == CircuitBreaker::State::Open);
    }

    SECTION("allowRequest returns false")
    {
        REQUIRE_FALSE(CircuitBreaker::allowRequest(peerId));
    }
}


TEST_CASE("Below threshold stays Closed", "[CircuitBreaker]")
{
    CircuitBreaker::reset();

    ulong peerId = 2;

    for (int i = 0; i < 4; ++i) {
        CircuitBreaker::recordFailure(peerId);
    }

    SECTION("state remains Closed")
    {
        REQUIRE(CircuitBreaker::getState(peerId) == CircuitBreaker::State::Closed);
    }

    SECTION("allowRequest returns true")
    {
        REQUIRE(CircuitBreaker::allowRequest(peerId));
    }
}


TEST_CASE("Success resets failure count", "[CircuitBreaker]")
{
    CircuitBreaker::reset();

    ulong peerId = 3;

    // Accumulate 4 failures (one below threshold)
    for (int i = 0; i < 4; ++i) {
        CircuitBreaker::recordFailure(peerId);
    }

    // Success should reset the failure count
    CircuitBreaker::recordSuccess(peerId);

    // Another 4 failures should not trip the circuit
    for (int i = 0; i < 4; ++i) {
        CircuitBreaker::recordFailure(peerId);
    }

    REQUIRE(CircuitBreaker::getState(peerId) == CircuitBreaker::State::Closed);
}


TEST_CASE("Success in Open transitions to Closed", "[CircuitBreaker]")
{
    CircuitBreaker::reset();

    ulong peerId = 4;

    // Open the circuit
    for (int i = 0; i < 5; ++i) {
        CircuitBreaker::recordFailure(peerId);
    }
    REQUIRE(CircuitBreaker::getState(peerId) == CircuitBreaker::State::Open);

    // Record a success while Open
    CircuitBreaker::recordSuccess(peerId);

    SECTION("state transitions to Closed")
    {
        REQUIRE(CircuitBreaker::getState(peerId) == CircuitBreaker::State::Closed);
    }

    SECTION("allowRequest returns true")
    {
        REQUIRE(CircuitBreaker::allowRequest(peerId));
    }
}


TEST_CASE("HalfOpen to Open on failure", "[CircuitBreaker]")
{
    CircuitBreaker::reset();
    CircuitBreaker::configure(5, 1);

    ulong peerId = 5;

    // Open the circuit
    for (int i = 0; i < 5; ++i) {
        CircuitBreaker::recordFailure(peerId);
    }
    REQUIRE(CircuitBreaker::getState(peerId) == CircuitBreaker::State::Open);

    // Wait for open duration to elapse so it transitions to HalfOpen
    std::this_thread::sleep_for(std::chrono::milliseconds(1100));
    REQUIRE(CircuitBreaker::getState(peerId) == CircuitBreaker::State::HalfOpen);

    // A failure in HalfOpen should send it back to Open
    CircuitBreaker::recordFailure(peerId);
    REQUIRE(CircuitBreaker::getState(peerId) == CircuitBreaker::State::Open);
}


TEST_CASE("Open to HalfOpen after duration", "[CircuitBreaker]")
{
    CircuitBreaker::reset();
    CircuitBreaker::configure(5, 1);

    ulong peerId = 6;

    // Open the circuit
    for (int i = 0; i < 5; ++i) {
        CircuitBreaker::recordFailure(peerId);
    }
    REQUIRE(CircuitBreaker::getState(peerId) == CircuitBreaker::State::Open);

    // Wait for the open duration to elapse
    std::this_thread::sleep_for(std::chrono::milliseconds(1100));
    REQUIRE(CircuitBreaker::getState(peerId) == CircuitBreaker::State::HalfOpen);
}


TEST_CASE("HalfOpen allows limited probes", "[CircuitBreaker]")
{
    CircuitBreaker::reset();
    CircuitBreaker::configure(5, 1);

    ulong peerId = 7;

    // Open the circuit
    for (int i = 0; i < 5; ++i) {
        CircuitBreaker::recordFailure(peerId);
    }

    // Wait for HalfOpen transition
    std::this_thread::sleep_for(std::chrono::milliseconds(1100));
    REQUIRE(CircuitBreaker::getState(peerId) == CircuitBreaker::State::HalfOpen);

    // halfOpenMaxProbes_s defaults to 2, so first 2 calls should succeed
    REQUIRE(CircuitBreaker::allowRequest(peerId));
    REQUIRE(CircuitBreaker::allowRequest(peerId));

    // Third probe should be denied
    REQUIRE_FALSE(CircuitBreaker::allowRequest(peerId));
}


TEST_CASE("Multiple peers are independent", "[CircuitBreaker]")
{
    CircuitBreaker::reset();

    ulong peerA = 100;
    ulong peerB = 200;

    // Accumulate failures only on peerA
    for (int i = 0; i < 5; ++i) {
        CircuitBreaker::recordFailure(peerA);
    }

    SECTION("peerA is Open")
    {
        REQUIRE(CircuitBreaker::getState(peerA) == CircuitBreaker::State::Open);
    }

    SECTION("peerB is still Closed")
    {
        REQUIRE(CircuitBreaker::getState(peerB) == CircuitBreaker::State::Closed);
    }

    SECTION("peerB allows requests")
    {
        REQUIRE(CircuitBreaker::allowRequest(peerB));
    }
}
