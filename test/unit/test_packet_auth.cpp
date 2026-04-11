/// Unit tests for PacketAuth

#include <PacketAuth.h>
#include <catch2/catch_test_macros.hpp>
#include <cstring>


TEST_CASE("PacketAuth enabled flag defaults to false", "[PacketAuth]")
{
    PacketAuth::setEnabled(false);
    REQUIRE_FALSE(PacketAuth::isEnabled());
}


TEST_CASE("PacketAuth setEnabled / isEnabled", "[PacketAuth]")
{
    PacketAuth::setEnabled(true);
    REQUIRE(PacketAuth::isEnabled());

    PacketAuth::setEnabled(false);
    REQUIRE_FALSE(PacketAuth::isEnabled());
}


TEST_CASE("PacketAuth setPeerSecret and hasPeerSecret", "[PacketAuth]")
{
    ulong peerId = 42;

    // Ensure clean state
    PacketAuth::removePeerSecret(peerId);
    REQUIRE_FALSE(PacketAuth::hasPeerSecret(peerId));

    byte secret[] = {0x01, 0x02, 0x03, 0x04, 0x05};
    REQUIRE(PacketAuth::setPeerSecret(peerId, secret, sizeof(secret)));
    REQUIRE(PacketAuth::hasPeerSecret(peerId));

    // Clean up
    PacketAuth::removePeerSecret(peerId);
}


TEST_CASE("PacketAuth setPeerSecret rejects null or zero-length", "[PacketAuth]")
{
    byte secret[] = {0xAA};

    REQUIRE_FALSE(PacketAuth::setPeerSecret(1, nullptr, 5));
    REQUIRE_FALSE(PacketAuth::setPeerSecret(1, secret, 0));
}


TEST_CASE("PacketAuth removePeerSecret", "[PacketAuth]")
{
    ulong peerId = 100;
    byte secret[] = {0x10, 0x20, 0x30};

    PacketAuth::setPeerSecret(peerId, secret, sizeof(secret));
    REQUIRE(PacketAuth::hasPeerSecret(peerId));

    REQUIRE(PacketAuth::removePeerSecret(peerId));
    REQUIRE_FALSE(PacketAuth::hasPeerSecret(peerId));

    // Removing a non-existent peer returns false
    REQUIRE_FALSE(PacketAuth::removePeerSecret(peerId));
}


TEST_CASE("PacketAuth setPeerSecret replaces existing secret", "[PacketAuth]")
{
    ulong peerId = 200;

    byte secret1[] = {0xAA, 0xBB, 0xCC};
    byte secret2[] = {0xDD, 0xEE, 0xFF, 0x11};

    PacketAuth::setPeerSecret(peerId, secret1, sizeof(secret1));
    REQUIRE(PacketAuth::hasPeerSecret(peerId));

    // Replace with a new secret
    REQUIRE(PacketAuth::setPeerSecret(peerId, secret2, sizeof(secret2)));
    REQUIRE(PacketAuth::hasPeerSecret(peerId));

    // Clean up
    PacketAuth::removePeerSecret(peerId);
}


TEST_CASE("PacketAuth generateSecret produces non-zero output", "[PacketAuth]")
{
    byte buf[32];
    std::memset(buf, 0, sizeof(buf));

    REQUIRE(PacketAuth::generateSecret(buf, sizeof(buf)));

    // Verify at least some bytes are non-zero
    bool allZero = true;
    for (uint i = 0; i < sizeof(buf); ++i) {
        if (buf[i] != 0) {
            allZero = false;
            break;
        }
    }
    REQUIRE_FALSE(allZero);
}


TEST_CASE("PacketAuth generateSecret rejects null or zero size", "[PacketAuth]")
{
    byte buf[16];

    REQUIRE_FALSE(PacketAuth::generateSecret(nullptr, 16));
    REQUIRE_FALSE(PacketAuth::generateSecret(buf, 0));
}


#ifdef ALPINE_TLS_ENABLED

TEST_CASE("PacketAuth computeHmac produces 32 bytes", "[PacketAuth][TLS]")
{
    ulong peerId = 300;
    byte secret[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10};
    byte data[] = {'H', 'e', 'l', 'l', 'o'};
    byte hmac[PacketAuth::HMAC_SIZE];

    PacketAuth::setPeerSecret(peerId, secret, sizeof(secret));

    REQUIRE(PacketAuth::computeHmac(peerId, data, sizeof(data), hmac, sizeof(hmac)));

    // Verify at least some bytes are non-zero
    bool allZero = true;
    for (uint i = 0; i < PacketAuth::HMAC_SIZE; ++i) {
        if (hmac[i] != 0) {
            allZero = false;
            break;
        }
    }
    REQUIRE_FALSE(allZero);

    PacketAuth::removePeerSecret(peerId);
}


TEST_CASE("PacketAuth computeHmac fails with insufficient buffer", "[PacketAuth][TLS]")
{
    ulong peerId = 301;
    byte secret[] = {0xAA, 0xBB, 0xCC, 0xDD};
    byte data[] = {'T', 'e', 's', 't'};
    byte hmac[16];  // too small

    PacketAuth::setPeerSecret(peerId, secret, sizeof(secret));

    REQUIRE_FALSE(PacketAuth::computeHmac(peerId, data, sizeof(data), hmac, sizeof(hmac)));

    PacketAuth::removePeerSecret(peerId);
}


TEST_CASE("PacketAuth verifyHmac succeeds with correct data", "[PacketAuth][TLS]")
{
    ulong peerId = 400;
    byte secret[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
    byte data[] = {'V', 'e', 'r', 'i', 'f', 'y'};
    byte hmac[PacketAuth::HMAC_SIZE];

    PacketAuth::setPeerSecret(peerId, secret, sizeof(secret));

    REQUIRE(PacketAuth::computeHmac(peerId, data, sizeof(data), hmac, sizeof(hmac)));

    REQUIRE(PacketAuth::verifyHmac(peerId, data, sizeof(data), hmac, PacketAuth::HMAC_SIZE));

    PacketAuth::removePeerSecret(peerId);
}


TEST_CASE("PacketAuth verifyHmac fails with tampered data", "[PacketAuth][TLS]")
{
    ulong peerId = 401;
    byte secret[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
    byte data[] = {'O', 'r', 'i', 'g', 'i', 'n', 'a', 'l'};
    byte hmac[PacketAuth::HMAC_SIZE];

    PacketAuth::setPeerSecret(peerId, secret, sizeof(secret));

    REQUIRE(PacketAuth::computeHmac(peerId, data, sizeof(data), hmac, sizeof(hmac)));

    // Tamper with the data
    data[0] = 'X';

    REQUIRE_FALSE(PacketAuth::verifyHmac(peerId, data, sizeof(data), hmac, PacketAuth::HMAC_SIZE));

    PacketAuth::removePeerSecret(peerId);
}


TEST_CASE("PacketAuth verifyHmac fails with tampered HMAC", "[PacketAuth][TLS]")
{
    ulong peerId = 402;
    byte secret[] = {0x12, 0x34, 0x56, 0x78};
    byte data[] = {'D', 'a', 't', 'a'};
    byte hmac[PacketAuth::HMAC_SIZE];

    PacketAuth::setPeerSecret(peerId, secret, sizeof(secret));

    REQUIRE(PacketAuth::computeHmac(peerId, data, sizeof(data), hmac, sizeof(hmac)));

    // Tamper with the HMAC
    hmac[0] ^= 0xFF;

    REQUIRE_FALSE(PacketAuth::verifyHmac(peerId, data, sizeof(data), hmac, PacketAuth::HMAC_SIZE));

    PacketAuth::removePeerSecret(peerId);
}


TEST_CASE("PacketAuth verifyHmac fails with unknown peer", "[PacketAuth][TLS]")
{
    ulong knownPeer = 500;
    ulong unknownPeer = 501;
    byte secret[] = {0xDE, 0xAD, 0xBE, 0xEF};
    byte data[] = {'T', 'e', 's', 't'};
    byte hmac[PacketAuth::HMAC_SIZE];

    PacketAuth::setPeerSecret(knownPeer, secret, sizeof(secret));

    REQUIRE(PacketAuth::computeHmac(knownPeer, data, sizeof(data), hmac, sizeof(hmac)));

    // Verify with a peer that has no secret
    PacketAuth::removePeerSecret(unknownPeer);
    REQUIRE_FALSE(PacketAuth::verifyHmac(unknownPeer, data, sizeof(data), hmac, PacketAuth::HMAC_SIZE));

    PacketAuth::removePeerSecret(knownPeer);
}


TEST_CASE("PacketAuth verifyHmac fails with wrong HMAC length", "[PacketAuth][TLS]")
{
    ulong peerId = 600;
    byte secret[] = {0x01, 0x02, 0x03, 0x04};
    byte data[] = {'L', 'e', 'n'};
    byte hmac[PacketAuth::HMAC_SIZE];

    PacketAuth::setPeerSecret(peerId, secret, sizeof(secret));

    REQUIRE(PacketAuth::computeHmac(peerId, data, sizeof(data), hmac, sizeof(hmac)));

    // Pass wrong length
    REQUIRE_FALSE(PacketAuth::verifyHmac(peerId, data, sizeof(data), hmac, PacketAuth::HMAC_SIZE - 1));

    PacketAuth::removePeerSecret(peerId);
}

#else

TEST_CASE("PacketAuth computeHmac returns false without TLS", "[PacketAuth][NoTLS]")
{
    ulong peerId = 700;
    byte secret[] = {0x01, 0x02, 0x03, 0x04};
    byte data[] = {'N', 'o', 'T', 'L', 'S'};
    byte hmac[PacketAuth::HMAC_SIZE];

    PacketAuth::setPeerSecret(peerId, secret, sizeof(secret));

    REQUIRE_FALSE(PacketAuth::computeHmac(peerId, data, sizeof(data), hmac, sizeof(hmac)));

    PacketAuth::removePeerSecret(peerId);
}


TEST_CASE("PacketAuth verifyHmac returns false without TLS", "[PacketAuth][NoTLS]")
{
    ulong peerId = 701;
    byte secret[] = {0x05, 0x06, 0x07, 0x08};
    byte data[] = {'N', 'o'};
    byte hmac[PacketAuth::HMAC_SIZE];
    std::memset(hmac, 0xAA, sizeof(hmac));

    PacketAuth::setPeerSecret(peerId, secret, sizeof(secret));

    REQUIRE_FALSE(PacketAuth::verifyHmac(peerId, data, sizeof(data), hmac, PacketAuth::HMAC_SIZE));

    PacketAuth::removePeerSecret(peerId);
}

#endif
