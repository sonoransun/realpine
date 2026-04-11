/// Unit tests for AlpineRatingEngine
///
/// NOTE: AlpineRatingEngine stores per-peer scores in a process-wide static
/// std::unordered_map. Tests share that state — each test uses a unique, large
/// peerId to avoid cross-test pollution. This is adequate for coverage but
/// means the tests are not fully isolated from each other or from prior runs
/// in the same process.

#include <AlpinePeerProfile.h>
#include <AlpineRatingEngine.h>
#include <catch2/catch_test_macros.hpp>
#include <cmath>


namespace {


// Build a transient AlpinePeerProfile bound to a given peer ID.
//
AlpinePeerProfile
makeProfile(ulong peerId)
{
    return AlpinePeerProfile(peerId);
}


constexpr double kEpsilon = 1e-9;


bool
approxEqual(double a, double b, double tol = 1e-6)
{
    return std::fabs(a - b) < tol;
}


}  // namespace


TEST_CASE("AlpineRatingEngine unknown peer returns neutral score", "[AlpineRatingEngine]")
{
    // Use a peerId very unlikely to be touched by anything else.
    ulong peerId = 9'000'000'001ULL;
    double score = AlpineRatingEngine::getScore(peerId);
    REQUIRE(approxEqual(score, 0.5));
}


TEST_CASE("AlpineRatingEngine queryResponseEvent increases score", "[AlpineRatingEngine]")
{
    ulong peerId = 9'000'000'100ULL;
    AlpinePeerProfile profile = makeProfile(peerId);

    // Start at neutral (unknown).
    REQUIRE(approxEqual(AlpineRatingEngine::getScore(peerId), 0.5));

    short delta = 0;
    REQUIRE(AlpineRatingEngine::queryResponseEvent(&profile, delta));
    REQUIRE(delta == 1);

    // One success event applies +0.1 * alpha(0.1) = +0.01 increment.
    double score = AlpineRatingEngine::getScore(peerId);
    REQUIRE(score > 0.5);
    // Should be ~0.51 — give ample tolerance for decay between write and read.
    REQUIRE(score <= 0.52);
}


TEST_CASE("AlpineRatingEngine successive successes accumulate upward", "[AlpineRatingEngine]")
{
    ulong peerId = 9'000'000'200ULL;
    AlpinePeerProfile profile = makeProfile(peerId);

    short delta = 0;
    double prevScore = AlpineRatingEngine::getScore(peerId);  // 0.5

    for (int i = 0; i < 20; ++i) {
        REQUIRE(AlpineRatingEngine::queryResponseEvent(&profile, delta));
    }

    double finalScore = AlpineRatingEngine::getScore(peerId);
    REQUIRE(finalScore > prevScore);
    // Clamped at 1.0, but with 20 iterations at +0.01 we should be around 0.7.
    REQUIRE(finalScore <= 1.0);
    REQUIRE(finalScore >= 0.6);
}


TEST_CASE("AlpineRatingEngine badPacketEvent decreases score", "[AlpineRatingEngine]")
{
    ulong peerId = 9'000'000'300ULL;
    AlpinePeerProfile profile = makeProfile(peerId);

    short delta = 0;
    REQUIRE(AlpineRatingEngine::badPacketEvent(&profile, delta));
    REQUIRE(delta == -3);

    double score = AlpineRatingEngine::getScore(peerId);
    // -0.3 * alpha(0.1) = -0.03 → score ~0.47
    REQUIRE(score < 0.5);
    REQUIRE(score >= 0.46);
}


TEST_CASE("AlpineRatingEngine transferFailureEvent decreases score", "[AlpineRatingEngine]")
{
    ulong peerId = 9'000'000'400ULL;
    AlpinePeerProfile profile = makeProfile(peerId);

    short delta = 0;
    REQUIRE(AlpineRatingEngine::transferFailureEvent(&profile, delta));
    REQUIRE(delta == -2);

    double score = AlpineRatingEngine::getScore(peerId);
    // -0.2 * 0.1 = -0.02 → ~0.48
    REQUIRE(score < 0.5);
    REQUIRE(score >= 0.47);
}


TEST_CASE("AlpineRatingEngine naResourceEvent applies a small penalty", "[AlpineRatingEngine]")
{
    ulong peerId = 9'000'000'500ULL;
    AlpinePeerProfile profile = makeProfile(peerId);

    short delta = 0;
    REQUIRE(AlpineRatingEngine::naResourceEvent(&profile, delta));
    REQUIRE(delta == -1);

    double score = AlpineRatingEngine::getScore(peerId);
    // -0.05 * 0.1 = -0.005 → ~0.495
    REQUIRE(score < 0.5);
    REQUIRE(score >= 0.49);
}


TEST_CASE("AlpineRatingEngine deceptiveResourceEvent applies a severe penalty", "[AlpineRatingEngine]")
{
    ulong peerId = 9'000'000'600ULL;
    AlpinePeerProfile profile = makeProfile(peerId);

    short delta = 0;
    REQUIRE(AlpineRatingEngine::deceptiveResourceEvent(&profile, delta));
    REQUIRE(delta == -5);

    double score = AlpineRatingEngine::getScore(peerId);
    // -0.5 * 0.1 = -0.05 → ~0.45
    REQUIRE(score < 0.5);
    REQUIRE(score >= 0.44);
}


TEST_CASE("AlpineRatingEngine clientResourceEvaluation rating variants", "[AlpineRatingEngine]")
{
    short delta = 0;

    SECTION("High rating increases score")
    {
        ulong peerId = 9'000'000'700ULL;
        AlpinePeerProfile profile = makeProfile(peerId);
        REQUIRE(
            AlpineRatingEngine::clientResourceEvaluation(&profile, AlpineRatingEngine::t_ResourceRating::High, delta));
        REQUIRE(delta == static_cast<short>(0.15 * 10.0));  // 1
        double score = AlpineRatingEngine::getScore(peerId);
        REQUIRE(score > 0.5);
    }

    SECTION("Low rating decreases score")
    {
        ulong peerId = 9'000'000'701ULL;
        AlpinePeerProfile profile = makeProfile(peerId);
        REQUIRE(
            AlpineRatingEngine::clientResourceEvaluation(&profile, AlpineRatingEngine::t_ResourceRating::Low, delta));
        REQUIRE(delta == static_cast<short>(-0.2 * 10.0));  // -2
        double score = AlpineRatingEngine::getScore(peerId);
        REQUIRE(score < 0.5);
    }

    SECTION("Average rating leaves score at neutral")
    {
        ulong peerId = 9'000'000'702ULL;
        AlpinePeerProfile profile = makeProfile(peerId);
        REQUIRE(AlpineRatingEngine::clientResourceEvaluation(
            &profile, AlpineRatingEngine::t_ResourceRating::Average, delta));
        REQUIRE(delta == 0);
        double score = AlpineRatingEngine::getScore(peerId);
        REQUIRE(approxEqual(score, 0.5, 1e-3));
    }
}


TEST_CASE("AlpineRatingEngine all event APIs reject null profile", "[AlpineRatingEngine]")
{
    short delta = 0;
    REQUIRE_FALSE(AlpineRatingEngine::queryResponseEvent(nullptr, delta));
    REQUIRE_FALSE(AlpineRatingEngine::badPacketEvent(nullptr, delta));
    REQUIRE_FALSE(AlpineRatingEngine::transferFailureEvent(nullptr, delta));
    REQUIRE_FALSE(AlpineRatingEngine::naResourceEvent(nullptr, delta));
    REQUIRE_FALSE(AlpineRatingEngine::deceptiveResourceEvent(nullptr, delta));
    REQUIRE_FALSE(
        AlpineRatingEngine::clientResourceEvaluation(nullptr, AlpineRatingEngine::t_ResourceRating::High, delta));
}


TEST_CASE("AlpineRatingEngine getTopScores skips peers with zero interactions", "[AlpineRatingEngine]")
{
    // Use a peer that has had no interactions — it should not appear.
    ulong untouchedPeer = 9'100'000'999ULL;
    (void)AlpineRatingEngine::getScore(untouchedPeer);  // insert-via-lookup? No — getScore is read-only for unknowns.

    auto scores = AlpineRatingEngine::getTopScores(1000);
    for (const auto & entry : scores) {
        REQUIRE(entry.peerId != untouchedPeer);
        REQUIRE(entry.interactions > 0);
    }
}


TEST_CASE("AlpineRatingEngine getTopScores returns interacted peer", "[AlpineRatingEngine]")
{
    ulong peerId = 9'200'000'001ULL;
    AlpinePeerProfile profile = makeProfile(peerId);

    short delta = 0;
    // Give this peer 5 successful interactions so it shows up in top scores.
    for (int i = 0; i < 5; ++i) {
        REQUIRE(AlpineRatingEngine::queryResponseEvent(&profile, delta));
    }

    auto scores = AlpineRatingEngine::getTopScores(1000);
    bool found = false;
    for (const auto & entry : scores) {
        if (entry.peerId == peerId) {
            found = true;
            REQUIRE(entry.interactions == 5);
            REQUIRE(entry.score > 0.5);
        }
    }
    REQUIRE(found);
}


TEST_CASE("AlpineRatingEngine mergeRemoteScores adopts new peer from remote", "[AlpineRatingEngine]")
{
    ulong remotePeerId = 9'300'000'001ULL;

    // Unknown peer locally — should return neutral.
    REQUIRE(approxEqual(AlpineRatingEngine::getScore(remotePeerId), 0.5));

    vector<AlpineRatingEngine::t_ScoreEntry> remote;
    AlpineRatingEngine::t_ScoreEntry entry;
    entry.peerId = remotePeerId;
    entry.score = 0.85;
    entry.interactions = 100;
    remote.push_back(entry);

    AlpineRatingEngine::mergeRemoteScores(remote, /*minInteractions=*/10);

    double score = AlpineRatingEngine::getScore(remotePeerId);
    // No local data → adopt remote score directly (0.85), modulo negligible decay.
    REQUIRE(score > 0.8);
    REQUIRE(score <= 0.85 + kEpsilon);
}


TEST_CASE("AlpineRatingEngine mergeRemoteScores ignores low-interaction remote entries", "[AlpineRatingEngine]")
{
    ulong remotePeerId = 9'300'000'100ULL;

    vector<AlpineRatingEngine::t_ScoreEntry> remote;
    AlpineRatingEngine::t_ScoreEntry entry;
    entry.peerId = remotePeerId;
    entry.score = 0.95;
    entry.interactions = 5;  // below minInteractions default (10)
    remote.push_back(entry);

    AlpineRatingEngine::mergeRemoteScores(remote, /*minInteractions=*/10);

    // Should not have been adopted.
    double score = AlpineRatingEngine::getScore(remotePeerId);
    REQUIRE(approxEqual(score, 0.5));
}


TEST_CASE("AlpineRatingEngine mergeRemoteScores combines with existing peer", "[AlpineRatingEngine]")
{
    ulong peerId = 9'300'000'200ULL;
    AlpinePeerProfile profile = makeProfile(peerId);

    short delta = 0;
    // Build up a local score by recording a handful of successes.
    for (int i = 0; i < 15; ++i) {
        REQUIRE(AlpineRatingEngine::queryResponseEvent(&profile, delta));
    }
    double localBefore = AlpineRatingEngine::getScore(peerId);
    REQUIRE(localBefore > 0.5);

    // Remote reports a much higher score with more interactions.
    vector<AlpineRatingEngine::t_ScoreEntry> remote;
    AlpineRatingEngine::t_ScoreEntry entry;
    entry.peerId = peerId;
    entry.score = 0.95;
    entry.interactions = 50;
    remote.push_back(entry);

    AlpineRatingEngine::mergeRemoteScores(remote, /*minInteractions=*/10);

    double merged = AlpineRatingEngine::getScore(peerId);
    // Weighted average: localInteractions=15, remoteInteractions=50,
    // merged ~= local*(15/65) + 0.95*(50/65) which must be higher than the
    // original local score since remote is much higher.
    REQUIRE(merged > localBefore);
    REQUIRE(merged <= 0.95 + kEpsilon);
}
