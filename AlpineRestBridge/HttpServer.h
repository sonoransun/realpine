/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <HttpRouter.h>
#include <WebSocketSession.h>
#include <asio.hpp>
#include <memory>
#include <atomic>
#include <vector>
#include <thread>
#include <mutex>
#include <queue>
#include <unordered_map>
#include <chrono>
#include <condition_variable>

#ifdef ALPINE_TLS_ENABLED
#include <asio/ssl.hpp>
#endif

class TlsContext;


class HttpServer
{
  public:

    HttpServer (HttpRouter & router);
    ~HttpServer ();

    bool  start (ulong   ipAddress,
                 ushort  port);

#ifdef ALPINE_TLS_ENABLED
    bool  startTls (ulong         ipAddress,
                    ushort        port,
                    TlsContext &  tlsCtx);
#endif

    void  stop ();


  private:

    void  doAccept ();
    void  handleConnection (asio::ip::tcp::socket socket);

#ifdef ALPINE_TLS_ENABLED
    void  doAcceptTls ();
    void  handleTlsConnection (asio::ssl::stream<asio::ip::tcp::socket> stream);
#endif

    template <typename Stream>
    void  processRequest (Stream & stream);

    void  upgradeToWebSocket (asio::ip::tcp::socket  socket,
                              const HttpRequest &    request);

    // --- Dynamic thread pool ---
    void  spawnWorker ();
    void  monitorPool ();
    void  sendServiceUnavailable (asio::ip::tcp::socket & socket);

    // --- Per-IP tracking ---
    bool  acquireConnectionSlot (const string & ip);
    void  releaseConnectionSlot (const string & ip);

    // --- Connection queue ---
    void  drainQueue ();

    HttpRouter &   router_;
    asio::io_context  ioContext_;
    std::unique_ptr<asio::ip::tcp::acceptor>  acceptor_;
    std::unique_ptr<asio::steady_timer>       poolMonitorTimer_;

    // Worker threads
    std::vector<std::thread>  workers_;
    std::mutex                workerMutex_;
    std::atomic<int>          activeWorkers_{0};
    std::atomic<int>          busyWorkers_{0};

    bool  running_{false};

#ifdef ALPINE_TLS_ENABLED
    TlsContext *  tlsContext_{nullptr};
    std::unique_ptr<asio::ssl::context>  sslContext_;
#endif

    // Connection tracking
    std::atomic<int>  activeConnections_{0};

    // Per-IP connection counts
    std::unordered_map<string, int>  ipConnectionCounts_;
    std::mutex                       ipCountMutex_;

    // Connection queue for overflow
    std::queue<asio::ip::tcp::socket>  connectionQueue_;
    std::mutex                         queueMutex_;
    static constexpr int CONNECTION_QUEUE_CAPACITY = 128;

    // Configurable limits (loaded once at start)
    int  minThreads_{4};
    int  maxThreads_{32};
    int  maxConnections_{512};
    int  maxConnectionsPerIp_{16};
    int  idleTimeoutSeconds_{60};

};
