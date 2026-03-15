/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <Common.h>

#include <asio.hpp>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <numeric>
#include <thread>
#include <vector>


using Clock = std::chrono::steady_clock;


struct Config
{
    string host        = "127.0.0.1"s;
    string port        = "8080"s;
    uint   connections = 10;
    uint   requestsPer = 100;
    string path        = "/status"s;
};


struct Stats
{
    std::vector<double>  latenciesUs;
    std::atomic<uint>    errors{0};
};


static Config
parseArgs (int argc, char * argv[])
{
    Config cfg;
    for (int i = 1; i < argc; ++i) {
        auto arg = string(argv[i]);
        if (arg == "--host" && i + 1 < argc)
            cfg.host = argv[++i];
        else if (arg == "--port" && i + 1 < argc)
            cfg.port = argv[++i];
        else if (arg == "--connections" && i + 1 < argc)
            cfg.connections = static_cast<uint>(std::stoi(argv[++i]));
        else if (arg == "--requests-per-conn" && i + 1 < argc)
            cfg.requestsPer = static_cast<uint>(std::stoi(argv[++i]));
        else if (arg == "--path" && i + 1 < argc)
            cfg.path = argv[++i];
        else if (arg == "--help") {
            std::cout << "Usage: rest_load_test [options]\n"
                      << "  --host <addr>              Target host (default: 127.0.0.1)\n"
                      << "  --port <port>              Target port (default: 8080)\n"
                      << "  --connections <n>           Concurrent connections (default: 10)\n"
                      << "  --requests-per-conn <n>    Requests per connection (default: 100)\n"
                      << "  --path <path>              HTTP path to request (default: /status)\n";
            std::exit(0);
        }
    }
    return cfg;
}


static void
runConnection (asio::io_context & ioc, const Config & cfg,
               Stats & stats, std::mutex & statsMtx)
{
    try {
        asio::ip::tcp::resolver resolver(ioc);
        auto endpoints = resolver.resolve(cfg.host, cfg.port);

        asio::ip::tcp::socket socket(ioc);
        asio::connect(socket, endpoints);

        auto request = "GET "s + cfg.path + " HTTP/1.1\r\n"
                     + "Host: "s + cfg.host + ":"s + cfg.port + "\r\n"
                     + "Connection: keep-alive\r\n"
                     + "Accept: application/json\r\n"
                     + "\r\n"s;

        std::vector<double> localLatencies;
        localLatencies.reserve(cfg.requestsPer);

        std::array<char, 8192> buf{};

        for (uint i = 0; i < cfg.requestsPer; ++i) {
            auto start = Clock::now();

            asio::error_code ec;
            asio::write(socket, asio::buffer(request), ec);
            if (ec) {
                stats.errors.fetch_add(1, std::memory_order_relaxed);
                continue;
            }

            auto n = socket.read_some(asio::buffer(buf), ec);
            if (ec && ec != asio::error::eof) {
                stats.errors.fetch_add(1, std::memory_order_relaxed);
                continue;
            }

            auto end = Clock::now();
            auto us  = std::chrono::duration<double, std::micro>(end - start).count();
            localLatencies.push_back(us);

            // Suppress unused warning
            static_cast<void>(n);
        }

        asio::error_code ignored;
        socket.shutdown(asio::ip::tcp::socket::shutdown_both, ignored);
        socket.close(ignored);

        std::lock_guard lock(statsMtx);
        stats.latenciesUs.insert(stats.latenciesUs.end(),
                                 localLatencies.begin(), localLatencies.end());
    }
    catch (const std::exception & e) {
        std::cerr << "Connection error: " << e.what() << "\n";
        stats.errors.fetch_add(cfg.requestsPer, std::memory_order_relaxed);
    }
}


static double
percentile (std::vector<double> & sorted, double p)
{
    if (sorted.empty())
        return 0.0;
    auto idx = static_cast<size_t>(p / 100.0 * static_cast<double>(sorted.size() - 1));
    return sorted[std::min(idx, sorted.size() - 1)];
}


int
main (int argc, char * argv[])
{
    auto cfg = parseArgs(argc, argv);

    auto totalRequests = cfg.connections * cfg.requestsPer;

    std::cout << "Load test: " << cfg.connections << " connections x "
              << cfg.requestsPer << " requests = " << totalRequests << " total\n"
              << "Target: " << cfg.host << ":" << cfg.port << cfg.path << "\n\n";

    Stats stats;
    stats.latenciesUs.reserve(totalRequests);
    std::mutex statsMtx;

    asio::io_context ioc;

    auto wallStart = Clock::now();

    std::vector<std::thread> threads;
    threads.reserve(cfg.connections);
    for (uint i = 0; i < cfg.connections; ++i) {
        threads.emplace_back([&]() {
            runConnection(ioc, cfg, stats, statsMtx);
        });
    }

    for (auto & t : threads)
        t.join();

    auto wallEnd  = Clock::now();
    auto wallSecs = std::chrono::duration<double>(wallEnd - wallStart).count();

    // Sort latencies for percentile calculation
    std::sort(stats.latenciesUs.begin(), stats.latenciesUs.end());

    auto successful = static_cast<uint>(stats.latenciesUs.size());
    auto errors     = stats.errors.load(std::memory_order_relaxed);
    auto rps        = wallSecs > 0.0 ? static_cast<double>(successful) / wallSecs : 0.0;

    double avgUs = 0.0;
    if (!stats.latenciesUs.empty()) {
        avgUs = std::accumulate(stats.latenciesUs.begin(),
                                stats.latenciesUs.end(), 0.0)
              / static_cast<double>(stats.latenciesUs.size());
    }

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "=== Results ===\n"
              << "  Total time:     " << wallSecs << " s\n"
              << "  Successful:     " << successful << "\n"
              << "  Errors:         " << errors << "\n"
              << "  Requests/sec:   " << rps << "\n"
              << "\n"
              << "=== Latency (microseconds) ===\n"
              << "  Mean:           " << avgUs << "\n"
              << "  p50:            " << percentile(stats.latenciesUs, 50) << "\n"
              << "  p95:            " << percentile(stats.latenciesUs, 95) << "\n"
              << "  p99:            " << percentile(stats.latenciesUs, 99) << "\n";

    if (!stats.latenciesUs.empty()) {
        std::cout << "  Min:            " << stats.latenciesUs.front() << "\n"
                  << "  Max:            " << stats.latenciesUs.back() << "\n";
    }

    return errors > 0 ? 1 : 0;
}
