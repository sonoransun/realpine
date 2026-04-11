/// Unit tests for DataBlock

#include <DataBlock.h>
#include <catch2/catch_test_macros.hpp>


TEST_CASE("DataBlock allocation", "[DataBlock]")
{
    SECTION("allocates buffer of requested size")
    {
        DataBlock block(256);
        REQUIRE(block.buffer_ != nullptr);
        REQUIRE(block.length_ == 256);
    }

    SECTION("buffer is writable")
    {
        DataBlock block(64);
        REQUIRE(block.buffer_ != nullptr);

        // Write and read back
        for (uint i = 0; i < block.length_; ++i)
            block.buffer_[i] = static_cast<byte>(i & 0xFF);

        for (uint i = 0; i < block.length_; ++i)
            REQUIRE(block.buffer_[i] == static_cast<byte>(i & 0xFF));
    }

    SECTION("large allocation succeeds")
    {
        DataBlock block(1024 * 1024);
        REQUIRE(block.buffer_ != nullptr);
        REQUIRE(block.length_ == 1024 * 1024);
    }
}


TEST_CASE("DataBlock zero-length", "[DataBlock]")
{
    DataBlock block(0);
    // Zero-length new[] returns a valid non-null pointer
    REQUIRE(block.buffer_ != nullptr);
    REQUIRE(block.length_ == 0);
}


TEST_CASE("DataBlock deallocation", "[DataBlock]")
{
    SECTION("destructor runs without error")
    {
        auto * block = new DataBlock(128);
        REQUIRE(block->buffer_ != nullptr);
        delete block;
        // If we get here, deallocation succeeded
        REQUIRE(true);
    }

    SECTION("multiple allocations and deallocations")
    {
        for (int i = 0; i < 100; ++i) {
            DataBlock block(512);
            REQUIRE(block.buffer_ != nullptr);
        }
        // No leaks or crashes across many cycles
        REQUIRE(true);
    }
}
