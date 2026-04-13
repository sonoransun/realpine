/// Unit tests for SdrDemodulator radio-layer frame encoding/decoding

#include <SdrDemodulator.h>
#include <catch2/catch_test_macros.hpp>
#include <cstring>


TEST_CASE("SdrDemodulator CRC-16 is deterministic", "[SdrDemodulator]")
{
    byte data[] = {0x01, 0x02, 0x03, 0x04};

    uint16_t crc1 = SdrDemodulator::crc16(data, sizeof(data));
    uint16_t crc2 = SdrDemodulator::crc16(data, sizeof(data));

    REQUIRE(crc1 == crc2);
    REQUIRE(crc1 != 0);  // very unlikely for this input
}


TEST_CASE("SdrDemodulator CRC-16 differs for different data", "[SdrDemodulator]")
{
    byte data1[] = {0x01, 0x02, 0x03, 0x04};
    byte data2[] = {0x01, 0x02, 0x03, 0x05};

    REQUIRE(SdrDemodulator::crc16(data1, sizeof(data1)) != SdrDemodulator::crc16(data2, sizeof(data2)));
}


TEST_CASE("SdrDemodulator CRC-16 empty data", "[SdrDemodulator]")
{
    uint16_t crc = SdrDemodulator::crc16(nullptr, 0);
    REQUIRE(crc == 0xFFFF);  // initial value with no data processed
}


TEST_CASE("SdrDemodulator buildRadioFrame produces correct structure", "[SdrDemodulator]")
{
    byte payload[] = {0xDE, 0xAD, 0xBE, 0xEF};
    auto frame = SdrDemodulator::buildRadioFrame(payload, sizeof(payload));

    // Expected: preamble(4) + sync(2) + length(2) + payload(4) + crc(2) = 14
    REQUIRE(frame.size() == 14);

    // Preamble
    REQUIRE(frame[0] == 0xAA);
    REQUIRE(frame[1] == 0xAA);
    REQUIRE(frame[2] == 0xAA);
    REQUIRE(frame[3] == 0xAA);

    // Sync word
    REQUIRE(frame[4] == 0xA1);
    REQUIRE(frame[5] == 0xE1);

    // Length (big-endian, 4)
    REQUIRE(frame[6] == 0x00);
    REQUIRE(frame[7] == 0x04);

    // Payload
    REQUIRE(frame[8] == 0xDE);
    REQUIRE(frame[9] == 0xAD);
    REQUIRE(frame[10] == 0xBE);
    REQUIRE(frame[11] == 0xEF);
}


TEST_CASE("SdrDemodulator parseRadioFrame round-trips with buildRadioFrame", "[SdrDemodulator]")
{
    byte payload[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    auto frame = SdrDemodulator::buildRadioFrame(payload, sizeof(payload));

    vector<byte> extracted;
    REQUIRE(SdrDemodulator::parseRadioFrame(frame.data(), static_cast<uint>(frame.size()), extracted));

    REQUIRE(extracted.size() == sizeof(payload));
    REQUIRE(memcmp(extracted.data(), payload, sizeof(payload)) == 0);
}


TEST_CASE("SdrDemodulator parseRadioFrame rejects corrupt CRC", "[SdrDemodulator]")
{
    byte payload[] = {0xCA, 0xFE, 0xBA, 0xBE};
    auto frame = SdrDemodulator::buildRadioFrame(payload, sizeof(payload));

    // Corrupt the last byte (CRC)
    frame.back() ^= 0xFF;

    vector<byte> extracted;
    REQUIRE_FALSE(SdrDemodulator::parseRadioFrame(frame.data(), static_cast<uint>(frame.size()), extracted));
}


TEST_CASE("SdrDemodulator parseRadioFrame rejects truncated frame", "[SdrDemodulator]")
{
    byte payload[] = {0x01, 0x02};
    auto frame = SdrDemodulator::buildRadioFrame(payload, sizeof(payload));

    // Truncate: remove CRC
    vector<byte> truncated(frame.begin(), frame.end() - 2);

    vector<byte> extracted;
    REQUIRE_FALSE(SdrDemodulator::parseRadioFrame(truncated.data(), static_cast<uint>(truncated.size()), extracted));
}


TEST_CASE("SdrDemodulator parseRadioFrame rejects bad preamble", "[SdrDemodulator]")
{
    byte payload[] = {0x01, 0x02};
    auto frame = SdrDemodulator::buildRadioFrame(payload, sizeof(payload));

    frame[0] = 0x00;  // corrupt preamble

    vector<byte> extracted;
    REQUIRE_FALSE(SdrDemodulator::parseRadioFrame(frame.data(), static_cast<uint>(frame.size()), extracted));
}


TEST_CASE("SdrDemodulator parseRadioFrame rejects bad sync word", "[SdrDemodulator]")
{
    byte payload[] = {0x01, 0x02};
    auto frame = SdrDemodulator::buildRadioFrame(payload, sizeof(payload));

    frame[4] = 0x00;  // corrupt sync

    vector<byte> extracted;
    REQUIRE_FALSE(SdrDemodulator::parseRadioFrame(frame.data(), static_cast<uint>(frame.size()), extracted));
}


TEST_CASE("SdrDemodulator parseRadioFrame rejects oversized length", "[SdrDemodulator]")
{
    byte payload[] = {0x01};
    auto frame = SdrDemodulator::buildRadioFrame(payload, sizeof(payload));

    // Set length to MAX_FRAME_PAYLOAD + 1
    frame[6] = 0xFF;
    frame[7] = 0xFF;

    vector<byte> extracted;
    REQUIRE_FALSE(SdrDemodulator::parseRadioFrame(frame.data(), static_cast<uint>(frame.size()), extracted));
}


TEST_CASE("SdrDemodulator buildRadioFrame zero-length payload", "[SdrDemodulator]")
{
    auto frame = SdrDemodulator::buildRadioFrame(nullptr, 0);

    // preamble(4) + sync(2) + length(2) + crc(2) = 10
    REQUIRE(frame.size() == 10);

    // Length should be 0
    REQUIRE(frame[6] == 0x00);
    REQUIRE(frame[7] == 0x00);

    vector<byte> extracted;
    REQUIRE(SdrDemodulator::parseRadioFrame(frame.data(), static_cast<uint>(frame.size()), extracted));
    REQUIRE(extracted.empty());
}


TEST_CASE("SdrDemodulator parseRadioFrame minimum valid frame", "[SdrDemodulator]")
{
    // Minimum: preamble(4) + sync(2) + length(2) + crc(2) = 10
    // This should be the zero-payload case
    auto frame = SdrDemodulator::buildRadioFrame(nullptr, 0);
    REQUIRE(frame.size() == 10);

    vector<byte> extracted;
    REQUIRE(SdrDemodulator::parseRadioFrame(frame.data(), static_cast<uint>(frame.size()), extracted));
    REQUIRE(extracted.empty());
}
