/// Copyright (C) 2026 sonoransun — see LICENCE.txt
///
/// Unit tests for ThreadUtils::setCurrentThreadName.

#include <ThreadUtils.h>
#include <atomic>
#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <string>
#include <thread>

#if defined(__linux__)
#include <fstream>
#include <sys/syscall.h>
#include <unistd.h>
#endif


TEST_CASE("ThreadUtils::setCurrentThreadName does not crash", "[ThreadUtils][ThreadName]")
{
    std::thread t([] { ThreadUtils::setCurrentThreadName("test-noop"s); });
    t.join();

    REQUIRE(true);
}


#if defined(__linux__)

namespace {

std::string
readCommForTid(pid_t tid)
{
    std::string path = "/proc/self/task/" + std::to_string(tid) + "/comm";
    std::ifstream in(path);
    if (!in.is_open())
        return {};
    std::string line;
    std::getline(in, line);
    return line;
}

}  // namespace


TEST_CASE("ThreadUtils::setCurrentThreadName sets /proc/self/task comm (Linux)", "[ThreadUtils][ThreadName][Linux]")
{
    std::atomic<pid_t> childTid{0};
    std::atomic<bool> nameSet{false};
    std::atomic<bool> canExit{false};

    std::thread t([&] {
        childTid.store(static_cast<pid_t>(::syscall(SYS_gettid)));
        ThreadUtils::setCurrentThreadName("test-name"s);
        nameSet.store(true);

        // Busy-wait (bounded) so the main thread can read /proc before we exit.
        //
        using clock = std::chrono::steady_clock;
        auto deadline = clock::now() + std::chrono::seconds(5);
        while (!canExit.load() && clock::now() < deadline) {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    });

    // Wait for the child thread to finish setting its name.
    //
    using clock = std::chrono::steady_clock;
    auto deadline = clock::now() + std::chrono::seconds(5);
    while (!nameSet.load() && clock::now() < deadline) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    REQUIRE(nameSet.load());
    REQUIRE(childTid.load() != 0);

    std::string comm = readCommForTid(childTid.load());
    canExit.store(true);
    t.join();

    REQUIRE(comm == "test-name");
}


TEST_CASE("ThreadUtils::setCurrentThreadName truncates to 15 bytes (Linux)", "[ThreadUtils][ThreadName][Linux]")
{
    std::atomic<pid_t> childTid{0};
    std::atomic<bool> nameSet{false};
    std::atomic<bool> canExit{false};

    // 20 characters → truncated to 15.
    const std::string longName = "alpine-very-long-name";

    std::thread t([&] {
        childTid.store(static_cast<pid_t>(::syscall(SYS_gettid)));
        ThreadUtils::setCurrentThreadName(longName);
        nameSet.store(true);

        using clock = std::chrono::steady_clock;
        auto deadline = clock::now() + std::chrono::seconds(5);
        while (!canExit.load() && clock::now() < deadline) {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    });

    using clock = std::chrono::steady_clock;
    auto deadline = clock::now() + std::chrono::seconds(5);
    while (!nameSet.load() && clock::now() < deadline) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    REQUIRE(nameSet.load());
    REQUIRE(childTid.load() != 0);

    std::string comm = readCommForTid(childTid.load());
    canExit.store(true);
    t.join();

    // Linux limits the kernel thread name to 15 bytes + NUL, so the truncated
    // form of the name should appear in /proc.
    //
    REQUIRE(comm == longName.substr(0, 15));
}

#endif  // __linux__
