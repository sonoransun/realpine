/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <benchmark/benchmark.h>
#include <HttpRequest.h>
#include <cstring>


static const char * simpleGet =
    "GET /status HTTP/1.1\r\n"
    "Host: localhost:8080\r\n"
    "Accept: application/json\r\n"
    "\r\n";

static const char * postWithBody =
    "POST /query/start HTTP/1.1\r\n"
    "Host: localhost:8080\r\n"
    "Content-Type: application/json\r\n"
    "Content-Length: 42\r\n"
    "Authorization: Bearer abc123\r\n"
    "Accept: application/json\r\n"
    "\r\n"
    "{\"query\":\"test search\",\"options\":{\"ttl\":30}}";

static const char * manyHeaders =
    "GET /peer HTTP/1.1\r\n"
    "Host: localhost:8080\r\n"
    "Accept: application/json\r\n"
    "Accept-Encoding: gzip, deflate\r\n"
    "Accept-Language: en-US,en;q=0.9\r\n"
    "Cache-Control: no-cache\r\n"
    "Connection: keep-alive\r\n"
    "User-Agent: Alpine-Bench/1.0\r\n"
    "X-Request-Id: bench-12345\r\n"
    "X-Forwarded-For: 192.168.1.1\r\n"
    "X-Forwarded-Proto: https\r\n"
    "\r\n";


static void
BM_ParseSimpleGet (benchmark::State & state)
{
    auto len = static_cast<ulong>(std::strlen(simpleGet));
    for (auto _ : state) {
        HttpRequest request;
        auto result = HttpRequest::parse(
            reinterpret_cast<const byte *>(simpleGet), len, request);
        benchmark::DoNotOptimize(result);
    }
    state.SetBytesProcessed(state.iterations() * static_cast<long long>(len));
}
BENCHMARK(BM_ParseSimpleGet);


static void
BM_ParsePostWithBody (benchmark::State & state)
{
    auto len = static_cast<ulong>(std::strlen(postWithBody));
    for (auto _ : state) {
        HttpRequest request;
        auto result = HttpRequest::parse(
            reinterpret_cast<const byte *>(postWithBody), len, request);
        benchmark::DoNotOptimize(result);
    }
    state.SetBytesProcessed(state.iterations() * static_cast<long long>(len));
}
BENCHMARK(BM_ParsePostWithBody);


static void
BM_ParseManyHeaders (benchmark::State & state)
{
    auto len = static_cast<ulong>(std::strlen(manyHeaders));
    for (auto _ : state) {
        HttpRequest request;
        auto result = HttpRequest::parse(
            reinterpret_cast<const byte *>(manyHeaders), len, request);
        benchmark::DoNotOptimize(result);
    }
    state.SetBytesProcessed(state.iterations() * static_cast<long long>(len));
}
BENCHMARK(BM_ParseManyHeaders);
