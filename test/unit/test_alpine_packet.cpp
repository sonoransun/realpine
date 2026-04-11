/// Unit tests for AlpinePacket

#include <AlpinePacket.h>
#include <DataBuffer.h>
#include <Platform.h>  // for htons/ntohs
#include <catch2/catch_test_macros.hpp>
#include <cstring>


TEST_CASE("AlpinePacket default construction", "[AlpinePacket]")
{
    AlpinePacket packet;

    REQUIRE(packet.getPacketType() == AlpinePacket::t_PacketType::none);
    REQUIRE(packet.getProtocolVersion() == AlpinePacket::PROTOCOL_VERSION);
}


TEST_CASE("AlpinePacket setPacketType / getPacketType", "[AlpinePacket]")
{
    AlpinePacket packet;

    REQUIRE(packet.setPacketType(AlpinePacket::t_PacketType::queryRequest));
    REQUIRE(packet.getPacketType() == AlpinePacket::t_PacketType::queryRequest);

    REQUIRE(packet.setPacketType(AlpinePacket::t_PacketType::peerListData));
    REQUIRE(packet.getPacketType() == AlpinePacket::t_PacketType::peerListData);
}


TEST_CASE("AlpinePacket packetTypeAsString known types", "[AlpinePacket]")
{
    string s;

    REQUIRE(AlpinePacket::packetTypeAsString(AlpinePacket::t_PacketType::none, s));
    REQUIRE(s == "no type set");

    REQUIRE(AlpinePacket::packetTypeAsString(AlpinePacket::t_PacketType::queryDiscover, s));
    REQUIRE(s == "query discover");

    REQUIRE(AlpinePacket::packetTypeAsString(AlpinePacket::t_PacketType::queryReply, s));
    REQUIRE(s == "query reply");

    REQUIRE(AlpinePacket::packetTypeAsString(AlpinePacket::t_PacketType::peerListData, s));
    REQUIRE(s == "peer list data");

    REQUIRE(AlpinePacket::packetTypeAsString(AlpinePacket::t_PacketType::queryCancellation, s));
    REQUIRE(s == "query cancellation");
}


TEST_CASE("AlpinePacket packetTypeAsString rejects out-of-range values", "[AlpinePacket]")
{
    string s;

    // 999 is outside the defined enum range
    REQUIRE_FALSE(AlpinePacket::packetTypeAsString(static_cast<AlpinePacket::t_PacketType>(999), s));
}


TEST_CASE("AlpinePacket writeData then readData round-trips packet type", "[AlpinePacket]")
{
    AlpinePacket writer;
    REQUIRE(writer.setPacketType(AlpinePacket::t_PacketType::queryOffer));

    DataBuffer buffer(1024);

    REQUIRE(writer.writeData(&buffer));

    // Now flip the buffer to read mode: extract what was written and create a
    // fresh buffer from those bytes.
    byte * rawData = nullptr;
    uint rawLen = 0;
    REQUIRE(buffer.getData(rawData, rawLen));
    REQUIRE(rawData != nullptr);
    // Versioned header is 2 + 2 + 2 = 6 bytes
    REQUIRE(rawLen == 6);

    DataBuffer readBuffer(rawData, rawLen);

    AlpinePacket reader;
    REQUIRE(reader.readData(&readBuffer));

    REQUIRE(reader.getPacketType() == AlpinePacket::t_PacketType::queryOffer);
    REQUIRE(reader.getProtocolVersion() == AlpinePacket::PROTOCOL_VERSION);
}


TEST_CASE("AlpinePacket round-trip across several packet types", "[AlpinePacket]")
{
    const AlpinePacket::t_PacketType types[] = {
        AlpinePacket::t_PacketType::queryDiscover,
        AlpinePacket::t_PacketType::queryOffer,
        AlpinePacket::t_PacketType::queryRequest,
        AlpinePacket::t_PacketType::queryReply,
        AlpinePacket::t_PacketType::peerListRequest,
        AlpinePacket::t_PacketType::peerListOffer,
        AlpinePacket::t_PacketType::peerListGet,
        AlpinePacket::t_PacketType::peerListData,
        AlpinePacket::t_PacketType::proxyRequest,
        AlpinePacket::t_PacketType::proxyAccepted,
        AlpinePacket::t_PacketType::proxyHalt,
        AlpinePacket::t_PacketType::versionHandshake,
        AlpinePacket::t_PacketType::versionAccept,
        AlpinePacket::t_PacketType::queryCancellation,
    };

    for (auto t : types) {
        AlpinePacket writer;
        REQUIRE(writer.setPacketType(t));

        DataBuffer buffer(64);
        REQUIRE(writer.writeData(&buffer));

        byte * rawData = nullptr;
        uint rawLen = 0;
        REQUIRE(buffer.getData(rawData, rawLen));
        REQUIRE(rawLen == 6);

        DataBuffer readBuffer(rawData, rawLen);
        AlpinePacket reader;
        REQUIRE(reader.readData(&readBuffer));
        REQUIRE(reader.getPacketType() == t);
    }
}


TEST_CASE("AlpinePacket readData on empty buffer returns false", "[AlpinePacket]")
{
    DataBuffer emptyBuffer(0);

    AlpinePacket reader;
    REQUIRE_FALSE(reader.readData(&emptyBuffer));
}


TEST_CASE("AlpinePacket readData on truncated versioned header returns false", "[AlpinePacket]")
{
    // Build a buffer that has the 0x0000 version marker only, but is too short
    // to contain the rest of the versioned header.
    //
    byte raw[2];
    uint16_t marker = htons(static_cast<uint16_t>(0));
    std::memcpy(raw, &marker, 2);

    DataBuffer buffer(raw, sizeof(raw));

    AlpinePacket reader;
    REQUIRE_FALSE(reader.readData(&buffer));
}


TEST_CASE("AlpinePacket readData accepts legacy (unversioned) packet format", "[AlpinePacket]")
{
    // Legacy format: a single uint16 packet type with no 0x0000 marker prefix.
    //
    byte raw[2];
    uint16_t legacyType = htons(static_cast<uint16_t>(AlpinePacket::t_PacketType::queryReply));
    std::memcpy(raw, &legacyType, 2);

    DataBuffer buffer(raw, sizeof(raw));

    AlpinePacket reader;
    REQUIRE(reader.readData(&buffer));
    REQUIRE(reader.getPacketType() == AlpinePacket::t_PacketType::queryReply);
    // Legacy packets carry no version field; implementation records version 0.
    REQUIRE(reader.getProtocolVersion() == 0);
}


TEST_CASE("AlpinePacket readData accepts versioned packet format", "[AlpinePacket]")
{
    // Versioned format: [0x0000][version][type]
    //
    byte raw[6];
    uint16_t marker = htons(static_cast<uint16_t>(0));
    uint16_t version = htons(static_cast<uint16_t>(AlpinePacket::PROTOCOL_VERSION));
    uint16_t type = htons(static_cast<uint16_t>(AlpinePacket::t_PacketType::proxyRequest));

    std::memcpy(raw, &marker, 2);
    std::memcpy(raw + 2, &version, 2);
    std::memcpy(raw + 4, &type, 2);

    DataBuffer buffer(raw, sizeof(raw));

    AlpinePacket reader;
    REQUIRE(reader.readData(&buffer));
    REQUIRE(reader.getPacketType() == AlpinePacket::t_PacketType::proxyRequest);
    REQUIRE(reader.getProtocolVersion() == AlpinePacket::PROTOCOL_VERSION);
}


TEST_CASE("AlpinePacket copy constructor copies state", "[AlpinePacket]")
{
    AlpinePacket original;
    original.setPacketType(AlpinePacket::t_PacketType::versionHandshake);

    AlpinePacket copy(original);
    REQUIRE(copy.getPacketType() == AlpinePacket::t_PacketType::versionHandshake);
    REQUIRE(copy.getProtocolVersion() == original.getProtocolVersion());
}


TEST_CASE("AlpinePacket assignment operator copies state", "[AlpinePacket]")
{
    AlpinePacket a;
    a.setPacketType(AlpinePacket::t_PacketType::proxyAccepted);

    AlpinePacket b;
    b = a;
    REQUIRE(b.getPacketType() == AlpinePacket::t_PacketType::proxyAccepted);

    // Self-assignment is safe
    b = b;
    REQUIRE(b.getPacketType() == AlpinePacket::t_PacketType::proxyAccepted);
}


TEST_CASE("AlpinePacket setParent / unsetParent", "[AlpinePacket]")
{
    AlpinePacket packet;

    // Passing nullptr is allowed by setParent (stored as-is).
    REQUIRE(packet.setParent(nullptr));

    packet.unsetParent();  // returns void; just verify it doesn't crash
    REQUIRE(true);
}
