/// Unit tests for AlpineError, Result<T>, and Status

#include <catch2/catch_test_macros.hpp>
#include <Error.h>

#include <string>


TEST_CASE("errorToString returns non-empty strings for all error codes", "[Error]")
{
    SECTION("original error codes")
    {
        REQUIRE(!errorToString(AlpineError::NotFound).empty());
        REQUIRE(!errorToString(AlpineError::InvalidArgument).empty());
        REQUIRE(!errorToString(AlpineError::NotInitialized).empty());
        REQUIRE(!errorToString(AlpineError::Timeout).empty());
        REQUIRE(!errorToString(AlpineError::ConnectionFailed).empty());
        REQUIRE(!errorToString(AlpineError::PermissionDenied).empty());
        REQUIRE(!errorToString(AlpineError::AlreadyExists).empty());
        REQUIRE(!errorToString(AlpineError::NotSupported).empty());
        REQUIRE(!errorToString(AlpineError::InternalError).empty());
        REQUIRE(!errorToString(AlpineError::AuthenticationFailed).empty());
        REQUIRE(!errorToString(AlpineError::AuthenticationRequired).empty());
        REQUIRE(!errorToString(AlpineError::DeviceNotEnrolled).empty());
        REQUIRE(!errorToString(AlpineError::ChallengeExpired).empty());
    }

    SECTION("new error codes")
    {
        REQUIRE(!errorToString(AlpineError::ResourceExhausted).empty());
        REQUIRE(!errorToString(AlpineError::NetworkError).empty());
        REQUIRE(!errorToString(AlpineError::SerializationError).empty());
        REQUIRE(!errorToString(AlpineError::ConfigurationError).empty());
        REQUIRE(!errorToString(AlpineError::RateLimited).empty());
        REQUIRE(!errorToString(AlpineError::ShutdownInProgress).empty());
    }
}


TEST_CASE("errorToString returns expected descriptive strings", "[Error]")
{
    REQUIRE(errorToString(AlpineError::ResourceExhausted) == "Resource exhausted");
    REQUIRE(errorToString(AlpineError::NetworkError) == "Network error");
    REQUIRE(errorToString(AlpineError::SerializationError) == "Serialization error");
    REQUIRE(errorToString(AlpineError::ConfigurationError) == "Configuration error");
    REQUIRE(errorToString(AlpineError::RateLimited) == "Rate limited");
    REQUIRE(errorToString(AlpineError::ShutdownInProgress) == "Shutdown in progress");
}


TEST_CASE("Result<T> works with new error codes", "[Error]")
{
    SECTION("Result holds a value on success")
    {
        Result<int> result{42};
        REQUIRE(result.has_value());
        REQUIRE(result.value() == 42);
    }

    SECTION("Result holds an error on failure")
    {
        Result<int> result{std::unexpected(AlpineError::ResourceExhausted)};
        REQUIRE(!result.has_value());
        REQUIRE(result.error() == AlpineError::ResourceExhausted);
    }

    SECTION("Result with each new error code")
    {
        Result<std::string> r1{std::unexpected(AlpineError::NetworkError)};
        REQUIRE(r1.error() == AlpineError::NetworkError);

        Result<std::string> r2{std::unexpected(AlpineError::SerializationError)};
        REQUIRE(r2.error() == AlpineError::SerializationError);

        Result<std::string> r3{std::unexpected(AlpineError::ConfigurationError)};
        REQUIRE(r3.error() == AlpineError::ConfigurationError);

        Result<std::string> r4{std::unexpected(AlpineError::RateLimited)};
        REQUIRE(r4.error() == AlpineError::RateLimited);

        Result<std::string> r5{std::unexpected(AlpineError::ShutdownInProgress)};
        REQUIRE(r5.error() == AlpineError::ShutdownInProgress);
    }
}


TEST_CASE("Status works with new error codes", "[Error]")
{
    SECTION("Status indicates success")
    {
        Status status{};
        REQUIRE(status.has_value());
    }

    SECTION("Status holds an error on failure")
    {
        Status status{std::unexpected(AlpineError::ShutdownInProgress)};
        REQUIRE(!status.has_value());
        REQUIRE(status.error() == AlpineError::ShutdownInProgress);
    }

    SECTION("Status with each new error code")
    {
        Status s1{std::unexpected(AlpineError::ResourceExhausted)};
        REQUIRE(s1.error() == AlpineError::ResourceExhausted);

        Status s2{std::unexpected(AlpineError::NetworkError)};
        REQUIRE(s2.error() == AlpineError::NetworkError);

        Status s3{std::unexpected(AlpineError::SerializationError)};
        REQUIRE(s3.error() == AlpineError::SerializationError);

        Status s4{std::unexpected(AlpineError::ConfigurationError)};
        REQUIRE(s4.error() == AlpineError::ConfigurationError);

        Status s5{std::unexpected(AlpineError::RateLimited)};
        REQUIRE(s5.error() == AlpineError::RateLimited);

        Status s6{std::unexpected(AlpineError::ShutdownInProgress)};
        REQUIRE(s6.error() == AlpineError::ShutdownInProgress);
    }
}
