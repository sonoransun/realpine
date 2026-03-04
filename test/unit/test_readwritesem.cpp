/// Unit tests for ReadWriteSem

#include <catch2/catch_test_macros.hpp>
#include <ReadWriteSem.h>
#include <thread>
#include <atomic>
#include <vector>
#include <chrono>


TEST_CASE("ReadWriteSem basic locking", "[ReadWriteSem]")
{
    ReadWriteSem sem;

    SECTION("acquireRead and releaseRead")
    {
        sem.acquireRead();
        sem.releaseRead();
        // No deadlock or error
        REQUIRE(true);
    }

    SECTION("acquireWrite and releaseWrite")
    {
        sem.acquireWrite();
        sem.releaseWrite();
        REQUIRE(true);
    }

    SECTION("tryAcquireRead succeeds when unlocked")
    {
        REQUIRE(sem.tryAcquireRead());
        sem.releaseRead();
    }

    SECTION("tryAcquireWrite succeeds when unlocked")
    {
        REQUIRE(sem.tryAcquireWrite());
        sem.releaseWrite();
    }
}


TEST_CASE("ReadWriteSem concurrent readers", "[ReadWriteSem]")
{
    ReadWriteSem sem;
    std::atomic<int> readersInside{0};
    std::atomic<int> maxReaders{0};
    constexpr int numReaders = 8;

    std::vector<std::thread> threads;
    for (int i = 0; i < numReaders; ++i)
    {
        threads.emplace_back([&]() {
            sem.acquireRead();
            int current = ++readersInside;

            // Track max concurrent readers
            int prev = maxReaders.load();
            while (prev < current && !maxReaders.compare_exchange_weak(prev, current))
                ;

            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            --readersInside;
            sem.releaseRead();
        });
    }

    for (auto& t : threads)
        t.join();

    // Multiple readers should have been inside simultaneously
    REQUIRE(maxReaders.load() > 1);
}


TEST_CASE("ReadWriteSem exclusive writer", "[ReadWriteSem]")
{
    ReadWriteSem sem;
    std::atomic<int> writersInside{0};
    std::atomic<bool> exclusionViolated{false};
    constexpr int numWriters = 4;

    std::vector<std::thread> threads;
    for (int i = 0; i < numWriters; ++i)
    {
        threads.emplace_back([&]() {
            sem.acquireWrite();

            int count = ++writersInside;
            if (count != 1)
                exclusionViolated.store(true);

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            --writersInside;
            sem.releaseWrite();
        });
    }

    for (auto& t : threads)
        t.join();

    REQUIRE_FALSE(exclusionViolated.load());
}


TEST_CASE("ReadWriteSem tryAcquireWrite fails during read lock", "[ReadWriteSem]")
{
    ReadWriteSem sem;

    sem.acquireRead();
    REQUIRE_FALSE(sem.tryAcquireWrite());
    sem.releaseRead();

    // Now it should succeed
    REQUIRE(sem.tryAcquireWrite());
    sem.releaseWrite();
}


TEST_CASE("ReadWriteSem tryAcquireRead fails during write lock", "[ReadWriteSem]")
{
    ReadWriteSem sem;

    sem.acquireWrite();
    REQUIRE_FALSE(sem.tryAcquireRead());
    sem.releaseWrite();

    // Now it should succeed
    REQUIRE(sem.tryAcquireRead());
    sem.releaseRead();
}
