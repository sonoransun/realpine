/// Unit tests for RbacPolicy

#include <catch2/catch_test_macros.hpp>
#include <RbacPolicy.h>
#include <fstream>
#include <filesystem>


using namespace std::string_literals;


static string
writeTempPolicy (const string & content)
{
    auto path = std::filesystem::temp_directory_path() / "test_rbac_policy.json";
    std::ofstream out(path);
    out << content;
    out.close();
    return path.string();
}


static void
cleanupTempPolicy ()
{
    auto path = std::filesystem::temp_directory_path() / "test_rbac_policy.json";
    std::filesystem::remove(path);
}


TEST_CASE("Disabled RBAC grants all access", "[RbacPolicy]")
{
    RbacPolicy::initialize(""s);

    SECTION("isEnabled returns false when no policy file loaded")
    {
        REQUIRE_FALSE(RbacPolicy::isEnabled());
    }

    SECTION("hasPermission returns true for any key and permission")
    {
        REQUIRE(RbacPolicy::hasPermission("anykey"s, "anything"s));
        REQUIRE(RbacPolicy::hasPermission(""s, "query:start"s));
        REQUIRE(RbacPolicy::hasPermission("unknown-key"s, "admin:ban"s));
    }

    SECTION("getRoleForKey returns admin when disabled")
    {
        REQUIRE(RbacPolicy::getRoleForKey("anykey"s) == "admin"s);
    }
}


TEST_CASE("Valid policy file loads successfully", "[RbacPolicy]")
{
    auto path = writeTempPolicy(R"({
        "roles": {
            "admin": ["*"],
            "query": ["query:start", "query:cancel"],
            "viewer": ["query:view", "peer:view"]
        },
        "keys": {
            "admin-key-123": "admin",
            "query-key-456": "query",
            "viewer-key-789": "viewer"
        }
    })");

    REQUIRE(RbacPolicy::initialize(path));
    REQUIRE(RbacPolicy::isEnabled());

    cleanupTempPolicy();
    RbacPolicy::initialize(""s);
}


TEST_CASE("Admin wildcard grants all permissions", "[RbacPolicy]")
{
    auto path = writeTempPolicy(R"({
        "roles": {
            "admin": ["*"],
            "query": ["query:start", "query:cancel"]
        },
        "keys": {
            "admin-key-123": "admin",
            "query-key-456": "query"
        }
    })");

    REQUIRE(RbacPolicy::initialize(path));

    SECTION("admin key can access query:start")
    {
        REQUIRE(RbacPolicy::hasPermission("admin-key-123"s, "query:start"s));
    }

    SECTION("admin key can access admin:ban")
    {
        REQUIRE(RbacPolicy::hasPermission("admin-key-123"s, "admin:ban"s));
    }

    SECTION("admin key can access any arbitrary permission")
    {
        REQUIRE(RbacPolicy::hasPermission("admin-key-123"s, "some:random:permission"s));
    }

    cleanupTempPolicy();
    RbacPolicy::initialize(""s);
}


TEST_CASE("Specific role only grants listed permissions", "[RbacPolicy]")
{
    auto path = writeTempPolicy(R"({
        "roles": {
            "admin": ["*"],
            "query": ["query:start", "query:cancel"],
            "viewer": ["query:view", "peer:view"]
        },
        "keys": {
            "admin-key-123": "admin",
            "query-key-456": "query",
            "viewer-key-789": "viewer"
        }
    })");

    REQUIRE(RbacPolicy::initialize(path));

    SECTION("query role can access query:start")
    {
        REQUIRE(RbacPolicy::hasPermission("query-key-456"s, "query:start"s));
    }

    SECTION("query role can access query:cancel")
    {
        REQUIRE(RbacPolicy::hasPermission("query-key-456"s, "query:cancel"s));
    }

    SECTION("query role cannot access admin:ban")
    {
        REQUIRE_FALSE(RbacPolicy::hasPermission("query-key-456"s, "admin:ban"s));
    }

    SECTION("viewer role can access query:view")
    {
        REQUIRE(RbacPolicy::hasPermission("viewer-key-789"s, "query:view"s));
    }

    SECTION("viewer role cannot access query:start")
    {
        REQUIRE_FALSE(RbacPolicy::hasPermission("viewer-key-789"s, "query:start"s));
    }

    cleanupTempPolicy();
    RbacPolicy::initialize(""s);
}


TEST_CASE("Unknown API key denied", "[RbacPolicy]")
{
    auto path = writeTempPolicy(R"({
        "roles": {
            "admin": ["*"]
        },
        "keys": {
            "admin-key-123": "admin"
        }
    })");

    REQUIRE(RbacPolicy::initialize(path));

    SECTION("unknown key is denied all permissions")
    {
        REQUIRE_FALSE(RbacPolicy::hasPermission("unknown-key"s, "query:start"s));
    }

    SECTION("getRoleForKey returns empty for unknown key")
    {
        REQUIRE(RbacPolicy::getRoleForKey("unknown-key"s).empty());
    }

    cleanupTempPolicy();
    RbacPolicy::initialize(""s);
}


TEST_CASE("Key mapped to nonexistent role denied", "[RbacPolicy]")
{
    auto path = writeTempPolicy(R"({
        "roles": {
            "admin": ["*"]
        },
        "keys": {
            "ghost-key-000": "ghost"
        }
    })");

    REQUIRE(RbacPolicy::initialize(path));

    SECTION("key with nonexistent role is denied")
    {
        REQUIRE_FALSE(RbacPolicy::hasPermission("ghost-key-000"s, "query:start"s));
    }

    SECTION("getRoleForKey returns the mapped role name even if role does not exist")
    {
        REQUIRE(RbacPolicy::getRoleForKey("ghost-key-000"s) == "ghost"s);
    }

    cleanupTempPolicy();
    RbacPolicy::initialize(""s);
}


TEST_CASE("Malformed JSON returns false", "[RbacPolicy]")
{
    auto path = writeTempPolicy("not json{}");

    REQUIRE_FALSE(RbacPolicy::initialize(path));

    cleanupTempPolicy();
    RbacPolicy::initialize(""s);
}


TEST_CASE("Missing file returns false", "[RbacPolicy]")
{
    REQUIRE_FALSE(RbacPolicy::initialize("/nonexistent/path.json"s));
}


TEST_CASE("Reload refreshes policy", "[RbacPolicy]")
{
    // Initial policy: query role has only query:start
    auto path = writeTempPolicy(R"({
        "roles": {
            "query": ["query:start"]
        },
        "keys": {
            "query-key-456": "query"
        }
    })");

    REQUIRE(RbacPolicy::initialize(path));
    REQUIRE(RbacPolicy::hasPermission("query-key-456"s, "query:start"s));
    REQUIRE_FALSE(RbacPolicy::hasPermission("query-key-456"s, "query:cancel"s));

    // Update the policy file to add query:cancel
    writeTempPolicy(R"({
        "roles": {
            "query": ["query:start", "query:cancel"],
            "viewer": ["query:view"]
        },
        "keys": {
            "query-key-456": "query",
            "viewer-key-789": "viewer"
        }
    })");

    REQUIRE(RbacPolicy::reload());

    SECTION("existing permission still works after reload")
    {
        REQUIRE(RbacPolicy::hasPermission("query-key-456"s, "query:start"s));
    }

    SECTION("newly added permission works after reload")
    {
        REQUIRE(RbacPolicy::hasPermission("query-key-456"s, "query:cancel"s));
    }

    SECTION("newly added role and key work after reload")
    {
        REQUIRE(RbacPolicy::hasPermission("viewer-key-789"s, "query:view"s));
    }

    cleanupTempPolicy();
    RbacPolicy::initialize(""s);
}
