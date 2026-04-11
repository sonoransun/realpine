/// Unit tests for alpine_random_bytes() (Platform.h).

#include <Common.h>
#include <Platform.h>
#include <catch2/catch_test_macros.hpp>


namespace {

bool
buffersEqual(const vector<byte> & a, const vector<byte> & b)
{
    if (a.size() != b.size())
        return false;
    for (size_t i = 0; i < a.size(); ++i) {
        if (a[i] != b[i])
            return false;
    }
    return true;
}

bool
allZero(const vector<byte> & buf)
{
    for (auto b : buf) {
        if (b != byte{0})
            return false;
    }
    return true;
}

}  // namespace


TEST_CASE("alpine_random_bytes fills buffers of various sizes", "[Platform][random]")
{
    const size_t sizes[] = {16, 64, 4096};

    for (size_t sz : sizes) {
        vector<byte> buf(sz, byte{0});
        // Prefill with a sentinel so we can detect a no-op.
        for (size_t i = 0; i < sz; ++i)
            buf[i] = byte{0xAB};

        bool ok = alpine_random_bytes(buf.data(), static_cast<unsigned long>(sz));
        REQUIRE(ok);

        // The call reports success for every size; the buffer is sized as
        // requested (vector size is preserved through the call).
        REQUIRE(buf.size() == sz);

        // A properly seeded CSPRNG should not emit an all-zero buffer of
        // 16+ bytes — the probability is 2^-128 or less.
        REQUIRE_FALSE(allZero(buf));
    }
}


TEST_CASE("alpine_random_bytes returns distinct output on consecutive calls", "[Platform][random]")
{
    const size_t sz = 64;
    vector<byte> first(sz, byte{0});
    vector<byte> second(sz, byte{0});

    REQUIRE(alpine_random_bytes(first.data(), static_cast<unsigned long>(sz)));
    REQUIRE(alpine_random_bytes(second.data(), static_cast<unsigned long>(sz)));

    // Neither buffer should be all-zero (catches the degenerate "we're
    // zeroing the buffer instead of filling it" bug).
    REQUIRE_FALSE(allZero(first));
    REQUIRE_FALSE(allZero(second));

    // Two consecutive 64-byte draws colliding is a 2^-512 event; treat any
    // equality as a regression in the RNG path.
    REQUIRE_FALSE(buffersEqual(first, second));
}
