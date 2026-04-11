/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <DataBlock.h>
#include <benchmark/benchmark.h>
#include <cstring>
#include <vector>


static void
BM_DataBlockCreate(benchmark::State & state)
{
    auto size = static_cast<uint>(state.range(0));
    for (auto _ : state) {
        DataBlock block(size);
        benchmark::DoNotOptimize(block.buffer_.get());
    }
    state.SetBytesProcessed(state.iterations() * size);
}
BENCHMARK(BM_DataBlockCreate)->Range(64, 1 << 20);


static void
BM_DataBlockMove(benchmark::State & state)
{
    auto size = static_cast<uint>(state.range(0));
    for (auto _ : state) {
        DataBlock src(size);
        std::memset(src.buffer_.get(), 0xAB, size);
        DataBlock dst(std::move(src));
        benchmark::DoNotOptimize(dst.buffer_.get());
    }
}
BENCHMARK(BM_DataBlockMove)->Range(64, 1 << 20);


static void
BM_DataBlockBatchCreate(benchmark::State & state)
{
    auto count = static_cast<uint>(state.range(0));
    for (auto _ : state) {
        std::vector<DataBlock> blocks;
        blocks.reserve(count);
        for (uint i = 0; i < count; ++i)
            blocks.emplace_back(256);
        benchmark::DoNotOptimize(blocks.data());
    }
}
BENCHMARK(BM_DataBlockBatchCreate)->Range(10, 10000);
