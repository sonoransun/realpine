/// Copyright (C) 2026 sonoransun — see LICENCE.txt

#include <catch2/catch_test_macros.hpp>
#include <Compression.h>
#include <zlib.h>


/// Helper: decompress gzip data using zlib
static string gzipDecompress (const string & compressed)
{
    if (compressed.empty())
        return ""s;

    z_stream zs{};
    // MAX_WBITS | 16 to decode gzip format
    if (inflateInit2(&zs, MAX_WBITS | 16) != Z_OK)
        return ""s;

    zs.next_in  = reinterpret_cast<Bytef *>(const_cast<char *>(compressed.data()));
    zs.avail_in = static_cast<uInt>(compressed.size());

    string output;
    char buffer[4096];

    int ret;
    do {
        zs.next_out  = reinterpret_cast<Bytef *>(buffer);
        zs.avail_out = sizeof(buffer);
        ret = inflate(&zs, Z_NO_FLUSH);
        if (ret != Z_OK && ret != Z_STREAM_END) {
            inflateEnd(&zs);
            return ""s;
        }
        output.append(buffer, sizeof(buffer) - zs.avail_out);
    } while (ret != Z_STREAM_END);

    inflateEnd(&zs);
    return output;
}


TEST_CASE("gzipCompress produces valid gzip", "[Compression]")
{
    auto input = "Hello, this is a test string for gzip compression."s;
    auto compressed = Compression::gzipCompress(input);

    REQUIRE(!compressed.empty());

    // Verify gzip magic bytes (1f 8b)
    REQUIRE(compressed.size() >= 2);
    REQUIRE(static_cast<uchar>(compressed[0]) == 0x1F);
    REQUIRE(static_cast<uchar>(compressed[1]) == 0x8B);

    // Decompress and verify match
    auto decompressed = gzipDecompress(compressed);
    REQUIRE(decompressed == input);
}


TEST_CASE("selectEncoding with various Accept-Encoding values", "[Compression]")
{
    SECTION("gzip alone")
    {
        REQUIRE(Compression::selectEncoding("gzip"s) == "gzip"s);
    }

    SECTION("gzip with deflate")
    {
        REQUIRE(Compression::selectEncoding("gzip, deflate"s) == "gzip"s);
    }

    SECTION("br with gzip quality parameter")
    {
        REQUIRE(Compression::selectEncoding("br, gzip;q=0.8"s) == "gzip"s);
    }

    SECTION("empty string returns identity")
    {
        REQUIRE(Compression::selectEncoding(""s) == "identity"s);
    }

    SECTION("identity only returns identity")
    {
        REQUIRE(Compression::selectEncoding("identity"s) == "identity"s);
    }

    SECTION("deflate only returns identity (gzip not present)")
    {
        REQUIRE(Compression::selectEncoding("deflate"s) == "identity"s);
    }

    SECTION("br only returns identity (gzip not present)")
    {
        REQUIRE(Compression::selectEncoding("br"s) == "identity"s);
    }

    SECTION("gzip with leading space")
    {
        REQUIRE(Compression::selectEncoding("deflate, gzip"s) == "gzip"s);
    }
}


TEST_CASE("empty and small inputs are not corrupted by compression", "[Compression]")
{
    SECTION("empty input returns empty string")
    {
        auto compressed = Compression::gzipCompress(""s);
        REQUIRE(compressed.empty());
    }

    SECTION("small input compresses and decompresses correctly")
    {
        auto input = "hi"s;
        auto compressed = Compression::gzipCompress(input);
        REQUIRE(!compressed.empty());

        auto decompressed = gzipDecompress(compressed);
        REQUIRE(decompressed == input);
    }

    SECTION("single byte input round-trips")
    {
        auto input = "x"s;
        auto compressed = Compression::gzipCompress(input);
        REQUIRE(!compressed.empty());

        auto decompressed = gzipDecompress(compressed);
        REQUIRE(decompressed == input);
    }
}


TEST_CASE("round-trip compress then decompress matches original", "[Compression]")
{
    SECTION("medium text")
    {
        string input;
        for (int i = 0; i < 100; ++i)
            input += "The quick brown fox jumps over the lazy dog. "s;

        auto compressed = Compression::gzipCompress(input);
        REQUIRE(!compressed.empty());
        REQUIRE(compressed.size() < input.size());  // Should actually compress

        auto decompressed = gzipDecompress(compressed);
        REQUIRE(decompressed == input);
    }

    SECTION("binary-like data")
    {
        string input;
        input.reserve(2048);
        for (int i = 0; i < 2048; ++i)
            input += static_cast<char>(i % 256);

        auto compressed = Compression::gzipCompress(input);
        REQUIRE(!compressed.empty());

        auto decompressed = gzipDecompress(compressed);
        REQUIRE(decompressed == input);
    }

    SECTION("large JSON-like payload")
    {
        string input = "["s;
        for (int i = 0; i < 500; ++i) {
            if (i > 0)
                input += ","s;
            input += "{\"id\":"s + std::to_string(i) + ",\"name\":\"item_"s + std::to_string(i) + "\"}"s;
        }
        input += "]"s;

        auto compressed = Compression::gzipCompress(input);
        REQUIRE(!compressed.empty());
        REQUIRE(compressed.size() < input.size());

        auto decompressed = gzipDecompress(compressed);
        REQUIRE(decompressed == input);
    }
}
