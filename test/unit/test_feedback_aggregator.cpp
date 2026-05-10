/// Unit tests for AlpineFeedbackAggregator — exercises path resolution,
/// session state machine, debouncing, and the RatingEngine submission path
/// using synthetic FsEvents.


#include <AlpineFeedbackAggregator.h>
#include <AlpineRatingEngine.h>
#include <FsObserver.h>
#include <ResourceStore.h>
#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <thread>


namespace {

string
makeTempRoot(const string & label)
{
    auto path = std::filesystem::temp_directory_path() / ("alpine-aggtest-" + label);
    std::filesystem::remove_all(path);
    std::filesystem::create_directories(path);
    return path.string();
}


void
touchFile(const string & path)
{
    std::ofstream f(path);
    f << "x";
}


FsEvent
makeEvent(FsEvent::Op op, const string & path, int pid, ulong bytes = 0)
{
    FsEvent ev;
    ev.op = op;
    ev.fsPath = path;
    ev.pid = pid;
    ev.bytes = bytes;
    ev.ts = std::chrono::steady_clock::now();
    return ev;
}

}  // namespace


TEST_CASE("Aggregator ignores events for unregistered paths", "[feedback_aggregator]")
{
    auto root = makeTempRoot("unregistered"s);
    ResourceStore store;
    REQUIRE(store.initialize(root));

    AlpineFeedbackAggregator agg(store);

    auto path = root + "/junk.bin"s;
    touchFile(path);

    agg.onEvent(makeEvent(FsEvent::Op::Open, path, 100));
    agg.onEvent(makeEvent(FsEvent::Op::Close, path, 100));

    REQUIRE(agg.totalEventsObserved() == 2);
    REQUIRE(agg.totalRatingsSubmitted() == 0);
}


TEST_CASE("Aggregator emits Low rating for abandoned (Open then Close, no Read)", "[feedback_aggregator]")
{
    constexpr ulong kPeerId = 9'500'001;
    constexpr ulong kResourceId = 7;

    auto root = makeTempRoot("abandoned"s);
    ResourceStore store;
    REQUIRE(store.initialize(root));

    auto path = root + "/r7.bin"s;
    touchFile(path);
    REQUIRE(store.registerDownload(kResourceId, kPeerId, path));

    double scoreBefore = AlpineRatingEngine::getScore(kPeerId);

    AlpineFeedbackAggregator::Config cfg;
    cfg.dwellMinMs = 100000;  // unreachable — force Low classification
    AlpineFeedbackAggregator agg(store, cfg);

    agg.onEvent(makeEvent(FsEvent::Op::Open, path, 200));
    agg.onEvent(makeEvent(FsEvent::Op::Close, path, 200));

    REQUIRE(agg.totalRatingsSubmitted() == 1);
    double scoreAfter = AlpineRatingEngine::getScore(kPeerId);
    REQUIRE(scoreAfter < scoreBefore);
}


TEST_CASE("Aggregator emits High rating for engaged session", "[feedback_aggregator]")
{
    constexpr ulong kPeerId = 9'500'002;
    constexpr ulong kResourceId = 11;

    auto root = makeTempRoot("engaged"s);
    ResourceStore store;
    REQUIRE(store.initialize(root));

    auto path = root + "/r11.bin"s;
    touchFile(path);
    REQUIRE(store.registerDownload(kResourceId, kPeerId, path));

    double scoreBefore = AlpineRatingEngine::getScore(kPeerId);

    AlpineFeedbackAggregator::Config cfg;
    cfg.dwellMinMs = 1;  // any dwell counts
    AlpineFeedbackAggregator agg(store, cfg);

    auto path1 = path;
    agg.onEvent(makeEvent(FsEvent::Op::Open, path1, 300));
    for (int i = 0; i < 5; ++i)
        agg.onEvent(makeEvent(FsEvent::Op::Read, path1, 300, 1024));
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    agg.onEvent(makeEvent(FsEvent::Op::Close, path1, 300));

    REQUIRE(agg.totalRatingsSubmitted() == 1);
    double scoreAfter = AlpineRatingEngine::getScore(kPeerId);
    REQUIRE(scoreAfter > scoreBefore);
}


TEST_CASE("Aggregator debounces repeated finalizations", "[feedback_aggregator]")
{
    constexpr ulong kPeerId = 9'500'003;
    constexpr ulong kResourceId = 13;

    auto root = makeTempRoot("debounce"s);
    ResourceStore store;
    REQUIRE(store.initialize(root));

    auto path = root + "/r13.bin"s;
    touchFile(path);
    REQUIRE(store.registerDownload(kResourceId, kPeerId, path));

    AlpineFeedbackAggregator::Config cfg;
    cfg.debounceSec = 60;  // big window — second close should be suppressed
    AlpineFeedbackAggregator agg(store, cfg);

    agg.onEvent(makeEvent(FsEvent::Op::Open, path, 400));
    agg.onEvent(makeEvent(FsEvent::Op::Close, path, 400));
    auto firstCount = agg.totalRatingsSubmitted();

    agg.onEvent(makeEvent(FsEvent::Op::Open, path, 400));
    agg.onEvent(makeEvent(FsEvent::Op::Close, path, 400));
    auto secondCount = agg.totalRatingsSubmitted();

    REQUIRE(firstCount == 1);
    REQUIRE(secondCount == 1);  // debounce suppressed the second
}


TEST_CASE("Aggregator config from environment", "[feedback_aggregator]")
{
    ::setenv("ALPINE_FEEDBACK_DWELL_MIN_MS", "750", 1);
    ::setenv("ALPINE_FEEDBACK_COMPLETION_HIGH", "0.95", 1);
    ::setenv("ALPINE_FEEDBACK_DEBOUNCE_SEC", "12", 1);

    auto cfg = AlpineFeedbackAggregator::configFromEnvironment();

    REQUIRE(cfg.dwellMinMs == 750);
    REQUIRE(cfg.completionHigh == 0.95);
    REQUIRE(cfg.debounceSec == 12);

    ::unsetenv("ALPINE_FEEDBACK_DWELL_MIN_MS");
    ::unsetenv("ALPINE_FEEDBACK_COMPLETION_HIGH");
    ::unsetenv("ALPINE_FEEDBACK_DEBOUNCE_SEC");
}
