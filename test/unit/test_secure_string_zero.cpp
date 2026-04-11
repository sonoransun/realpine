/// Unit tests for SecureString::clear() actually zeroing the backing buffer.

#include <SecureString.h>
#include <catch2/catch_test_macros.hpp>


TEST_CASE("SecureString::clear zeroes backing storage", "[SecureString][secureErase]")
{
    // Construct a string from data that has no byte set to zero, so a successful
    // wipe is unambiguous (every byte must flip to '\0').
    const string plaintext = "correct-horse-battery-staple"s;
    SecureString ss(plaintext);

    // Snapshot the backing pointer and size via value() before clear().
    const string & before = ss.value();
    const char * backing = before.data();
    const size_t backingSize = before.size();
    REQUIRE(backingSize == plaintext.size());

    // Capture a copy of the bytes prior to clear() and sanity-check they
    // match the original plaintext (i.e. the SecureString actually holds it).
    vector<byte> snapshotBefore(backingSize);
    for (size_t i = 0; i < backingSize; ++i)
        snapshotBefore[i] = static_cast<byte>(backing[i]);
    for (size_t i = 0; i < backingSize; ++i)
        REQUIRE(static_cast<char>(snapshotBefore[i]) == plaintext[i]);

    // Wipe.
    ss.clear();
    REQUIRE(ss.empty());
    REQUIRE(ss.value().empty());

    // The bytes observed at the pre-clear address must all be zero now.
    // NB: after std::string::clear() the backing storage is still owned by
    // the string object (capacity is preserved), so reading backing[] is
    // well-defined for indices < backingSize.
    for (size_t i = 0; i < backingSize; ++i) {
        REQUIRE(backing[i] == '\0');
    }
}
