/// Unit tests for ResourceStore — the on-disk path -> {resourceId, peerId}
/// registry that backs the FsObserve subsystem.


#include <ResourceStore.h>
#include <catch2/catch_test_macros.hpp>
#include <cstdlib>
#include <filesystem>
#include <fstream>


namespace {

string
makeTempRoot(const string & label)
{
    auto path = std::filesystem::temp_directory_path() / ("alpine-rstest-" + label);
    std::filesystem::remove_all(path);
    std::filesystem::create_directories(path);
    return path.string();
}


void
writeFile(const string & path, const string & content)
{
    std::ofstream f(path);
    f << content;
}

}  // namespace


TEST_CASE("ResourceStore initialize and shutdown round-trip", "[resource_store]")
{
    auto root = makeTempRoot("init"s);

    ResourceStore store;
    REQUIRE(store.initialize(root));
    REQUIRE_FALSE(store.root().empty());
    REQUIRE(store.size() == 0);

    store.shutdown();
    REQUIRE(store.size() == 0);
}


TEST_CASE("ResourceStore registers, resolves, and unregisters paths", "[resource_store]")
{
    auto root = makeTempRoot("register"s);
    ResourceStore store;
    REQUIRE(store.initialize(root));

    auto filePath = root + "/resource_42.bin"s;
    writeFile(filePath, "hello"s);

    REQUIRE(store.registerDownload(42, 1001, filePath));
    REQUIRE(store.size() == 1);

    ulong resolvedRid = 0;
    ulong resolvedPid = 0;
    REQUIRE(store.resolvePath(filePath, resolvedRid, resolvedPid));
    REQUIRE(resolvedRid == 42);
    REQUIRE(resolvedPid == 1001);

    REQUIRE(store.unregister(42));
    REQUIRE(store.size() == 0);
    REQUIRE_FALSE(store.resolvePath(filePath, resolvedRid, resolvedPid));
}


TEST_CASE("ResourceStore rejects paths outside root", "[resource_store]")
{
    auto root = makeTempRoot("outside"s);
    ResourceStore store;
    REQUIRE(store.initialize(root));

    auto outside = std::filesystem::temp_directory_path().string() + "/elsewhere.bin"s;
    REQUIRE_FALSE(store.registerDownload(1, 1, outside));
    REQUIRE(store.size() == 0);
}


TEST_CASE("ResourceStore overwrites prior path on re-register", "[resource_store]")
{
    auto root = makeTempRoot("rebind"s);
    ResourceStore store;
    REQUIRE(store.initialize(root));

    auto firstPath = root + "/r99_a.bin"s;
    auto secondPath = root + "/r99_b.bin"s;
    writeFile(firstPath, "a"s);
    writeFile(secondPath, "b"s);

    REQUIRE(store.registerDownload(99, 7, firstPath));
    REQUIRE(store.registerDownload(99, 7, secondPath));
    REQUIRE(store.size() == 1);

    ulong rid = 0, pid = 0;
    REQUIRE_FALSE(store.resolvePath(firstPath, rid, pid));
    REQUIRE(store.resolvePath(secondPath, rid, pid));
    REQUIRE(rid == 99);
}


TEST_CASE("ResourceStore rejects empty arguments", "[resource_store]")
{
    auto root = makeTempRoot("empty"s);
    ResourceStore store;
    REQUIRE(store.initialize(root));

    REQUIRE_FALSE(store.registerDownload(0, 1, root + "/a.bin"s));
    REQUIRE_FALSE(store.registerDownload(1, 0, root + "/a.bin"s));
    REQUIRE_FALSE(store.registerDownload(1, 1, ""s));
}
