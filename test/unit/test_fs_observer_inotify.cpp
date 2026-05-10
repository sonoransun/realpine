/// Unit tests for InotifyObserver — drives a tmpdir with real file
/// open/read/close syscalls and asserts the events propagate to the sink.


#include <FsObserver.h>
#include <InotifyObserver.h>
#include <atomic>
#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <thread>
#include <vector>


namespace {

string
makeTempRoot(const string & label)
{
    auto path = std::filesystem::temp_directory_path() / ("alpine-inotest-" + label);
    std::filesystem::remove_all(path);
    std::filesystem::create_directories(path);
    return path.string();
}


bool
waitFor(std::atomic<bool> & flag, std::chrono::milliseconds timeout)
{
    auto deadline = std::chrono::steady_clock::now() + timeout;
    while (std::chrono::steady_clock::now() < deadline) {
        if (flag.load())
            return true;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return flag.load();
}

}  // namespace


TEST_CASE("InotifyObserver delivers Open / Read / Close for a real file", "[fs_observer][inotify]")
{
#if !defined(__linux__)
    SKIP("inotify requires Linux");
#else
    auto root = makeTempRoot("basic"s);
    auto filePath = root + "/sample.bin"s;
    {
        std::ofstream f(filePath);
        f << "abcdefghij";
    }

    std::mutex eventsMutex;
    std::vector<FsEvent> events;
    std::atomic<bool> sawClose{false};

    InotifyObserver observer(root);
    observer.setSink([&](const FsEvent & ev) {
        std::lock_guard<std::mutex> g(eventsMutex);
        events.push_back(ev);
        if (ev.op == FsEvent::Op::Close)
            sawClose.store(true);
    });

    REQUIRE(observer.start());
    REQUIRE(observer.isRunning());

    // Give the watcher a beat to hit the poll() loop.
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    {
        std::FILE * fp = std::fopen(filePath.c_str(), "rb");
        REQUIRE(fp != nullptr);
        char buf[16];
        std::fread(buf, 1, sizeof(buf), fp);
        std::fclose(fp);
    }

    REQUIRE(waitFor(sawClose, std::chrono::seconds(2)));

    observer.stop();
    REQUIRE_FALSE(observer.isRunning());

    std::lock_guard<std::mutex> g(eventsMutex);
    bool sawOpen = false;
    bool sawRead = false;
    bool sawCloseEv = false;
    for (const auto & ev : events) {
        if (ev.fsPath != filePath)
            continue;
        if (ev.op == FsEvent::Op::Open)
            sawOpen = true;
        if (ev.op == FsEvent::Op::Read)
            sawRead = true;
        if (ev.op == FsEvent::Op::Close)
            sawCloseEv = true;
    }
    REQUIRE(sawOpen);
    REQUIRE(sawRead);
    REQUIRE(sawCloseEv);
#endif
}


TEST_CASE("InotifyObserver start fails without a sink", "[fs_observer][inotify]")
{
#if !defined(__linux__)
    SKIP("inotify requires Linux");
#else
    auto root = makeTempRoot("nosink"s);
    InotifyObserver observer(root);
    REQUIRE_FALSE(observer.start());
#endif
}


TEST_CASE("InotifyObserver start fails on missing root", "[fs_observer][inotify]")
{
#if !defined(__linux__)
    SKIP("inotify requires Linux");
#else
    InotifyObserver observer("/nonexistent/alpine-inotest-missing"s);
    observer.setSink([](const FsEvent &) {});
    REQUIRE_FALSE(observer.start());
#endif
}
