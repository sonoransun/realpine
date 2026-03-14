/// Unit tests for SecureString

#include <catch2/catch_test_macros.hpp>
#include <SecureString.h>


TEST_CASE("SecureString default construction", "[SecureString]")
{
    SecureString ss;
    REQUIRE(ss.empty());
    REQUIRE(ss.value().empty());
}


TEST_CASE("SecureString construction from string", "[SecureString]")
{
    SECTION("from const lvalue")
    {
        const string src = "secret_password"s;
        SecureString ss(src);
        REQUIRE_FALSE(ss.empty());
        REQUIRE(ss.value() == "secret_password"s);
    }

    SECTION("from rvalue")
    {
        SecureString ss("temporary_secret"s);
        REQUIRE_FALSE(ss.empty());
        REQUIRE(ss.value() == "temporary_secret"s);
    }
}


TEST_CASE("SecureString move construction", "[SecureString]")
{
    SecureString original("move_me"s);
    REQUIRE(original.value() == "move_me"s);

    SecureString moved(std::move(original));
    REQUIRE(moved.value() == "move_me"s);

    // Source should be empty after move
    REQUIRE(original.empty());
}


TEST_CASE("SecureString move assignment", "[SecureString]")
{
    SecureString a("first"s);
    SecureString b("second"s);

    b = std::move(a);
    REQUIRE(b.value() == "first"s);

    // Source should be empty after move
    REQUIRE(a.empty());
}


TEST_CASE("SecureString assign methods", "[SecureString]")
{
    SecureString ss;

    SECTION("assign from const lvalue")
    {
        const string src = "assigned_value"s;
        ss.assign(src);
        REQUIRE(ss.value() == "assigned_value"s);
    }

    SECTION("assign from rvalue")
    {
        ss.assign("rvalue_assigned"s);
        REQUIRE(ss.value() == "rvalue_assigned"s);
    }

    SECTION("assign replaces previous value")
    {
        ss.assign("first_value"s);
        REQUIRE(ss.value() == "first_value"s);

        ss.assign("second_value"s);
        REQUIRE(ss.value() == "second_value"s);
    }
}


TEST_CASE("SecureString clear", "[SecureString]")
{
    SecureString ss("to_be_cleared"s);
    REQUIRE_FALSE(ss.empty());

    ss.clear();
    REQUIRE(ss.empty());
    REQUIRE(ss.value().empty());
}


TEST_CASE("SecureString value access", "[SecureString]")
{
    SecureString ss("access_me"s);
    const string & ref = ss.value();
    REQUIRE(ref == "access_me"s);

    // Verify the reference stays valid
    REQUIRE(ss.value().size() == 9);
}


TEST_CASE("SecureString equals", "[SecureString]")
{
    SECTION("same string returns true")
    {
        SecureString ss("match_this"s);
        REQUIRE(ss.equals("match_this"s));
    }

    SECTION("different string returns false")
    {
        SecureString ss("one_thing"s);
        REQUIRE_FALSE(ss.equals("another_thing"s));
    }

    SECTION("different length returns false")
    {
        SecureString ss("short"s);
        REQUIRE_FALSE(ss.equals("much_longer_string"s));
    }

    SECTION("empty equals empty")
    {
        SecureString ss;
        REQUIRE(ss.equals(""s));
    }

    SECTION("empty does not equal non-empty")
    {
        SecureString ss;
        REQUIRE_FALSE(ss.equals("something"s));
    }

    SECTION("non-empty does not equal empty")
    {
        SecureString ss("something"s);
        REQUIRE_FALSE(ss.equals(""s));
    }
}


// Copy construction and copy assignment are deleted (compile-time check).
// Uncommenting either of the following lines should produce a compilation error:
//
//   SecureString copy(ss);               // deleted copy constructor
//   SecureString copy2 = ss;             // deleted copy constructor
//   SecureString other; other = ss;      // deleted copy assignment
