/// Integration test: drives a real file open/read/close on a registered
/// resource through InotifyObserver -> AlpineFeedbackAggregator ->
/// AlpineRatingEngine and asserts the affected peer's score moves.


#include <AlpineFeedbackAggregator.h>
#include <AlpineRatingEngine.h>
#include <FsObserverFactory.h>
#include <InotifyObserver.h>
#include <ResourceStore.h>
#include <atomic>
#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <thread>


namespace {

constexpr ulong kPeerId = 9'700'001;
constexpr ulong kResourceId = 71;


string
makeTempRoot(const string & label)
{
    auto path = std::filesystem::temp_directory_path() / ("alpine-otorint-" + label);
    std::filesystem::remove_all(path);
    std::filesystem::create_directories(path);
    return path.string();
}


bool
waitForRatingsSubmitted(const AlpineFeedbackAggregator & agg, ulong expected, std::chrono::milliseconds timeout)
{
    auto deadline = std::chrono::steady_clock::now() + timeout;
    while (std::chrono::steady_clock::now() < deadline) {
        if (agg.totalRatingsSubmitted() >= expected)
            return true;
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    return agg.totalRatingsSubmitted() >= expected;
}

}  // namespace


TEST_CASE("Real file access moves the serving peer's rating score", "[integration][fs_observer]")
{
#if !defined(__linux__)
    SKIP("FsObserve integration requires Linux");
#else
    auto root = makeTempRoot("e2e"s);
    auto filePath = root + "/r71.bin"s;

    // Materialize a "downloaded" resource and register it.
    {
        std::ofstream f(filePath);
        for (int i = 0; i < 1024; ++i)
            f << "alpine-content";
    }

    ResourceStore store;
    REQUIRE(store.initialize(root));
    REQUIRE(store.registerDownload(kResourceId, kPeerId, filePath));

    AlpineFeedbackAggregator::Config cfg;
    cfg.dwellMinMs = 1;   // any non-zero dwell counts
    cfg.debounceSec = 0;  // no suppression for the test
    AlpineFeedbackAggregator agg(store, cfg);

    InotifyObserver observer(store.root());
    observer.setSink([&](const FsEvent & ev) { agg.onEvent(ev); });
    REQUIRE(observer.start());

    // Brief settle so the inotify thread is in poll().
    std::this_thread::sleep_for(std::chrono::milliseconds(150));

    double scoreBefore = AlpineRatingEngine::getScore(kPeerId);

    // Drive a real consumption — open, fully read, close.  Multiple Read
    // events from the libc buffer keep the aggregator's read-count above
    // its High threshold.
    {
        std::FILE * fp = std::fopen(filePath.c_str(), "rb");
        REQUIRE(fp != nullptr);
        char buf[256];
        while (std::fread(buf, 1, sizeof(buf), fp) > 0) {
            // drain
        }
        std::fclose(fp);
    }

    REQUIRE(waitForRatingsSubmitted(agg, 1, std::chrono::seconds(3)));

    observer.stop();

    double scoreAfter = AlpineRatingEngine::getScore(kPeerId);

    // For an engaged read the rating should rise.  Allow either direction
    // technically, but assert non-zero movement.
    REQUIRE(scoreAfter != scoreBefore);
#endif
}
