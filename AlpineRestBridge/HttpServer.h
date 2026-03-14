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
#include <array>
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

    [[nodiscard]] int  getActiveConnections () const;


  private:

    // --- RAII guard for connection resource cleanup ---
    class ConnectionGuard
    {
      public:

        ConnectionGuard (HttpServer & server, string clientIp);
        ~ConnectionGuard ();

        ConnectionGuard (const ConnectionGuard &)             = delete;
        ConnectionGuard & operator= (const ConnectionGuard &) = delete;

        void  setBusy ();
        void  clearBusy ();
        void  setSlotAcquired ();

      private:

        HttpServer &  server_;
        string        clientIp_;
        bool          busy_{false};
        bool          slotAcquired_{false};
    };

    void  doAccept ();
    void  handleConnection (asio::ip::tcp::socket socket);

#ifdef ALPINE_TLS_ENABLED
    void  doAcceptTls ();
    void  handleTlsConnection (asio::ssl::stream<asio::ip::tcp::socket> stream);
#endif

    template <typename Stream>
    void  processRequest (Stream &          stream,
                          const string &    clientIp,
                          ConnectionGuard & guard);

    void  upgradeToWebSocket (asio::ip::tcp::socket  socket,
                              const HttpRequest &    request);

    // --- Dynamic thread pool ---
    void  spawnWorker ();
    void  monitorPool ();
    void  sendServiceUnavailable (asio::ip::tcp::socket & socket);

    // --- Per-IP tracking (sharded) ---
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

    // Per-IP connection counts — sharded to reduce lock contention
    static constexpr size_t IP_SHARD_COUNT = 16;

    struct t_IpShard {
        std::mutex                           mutex;
        std::unordered_map<string, int>      counts;
    };

    std::array<t_IpShard, IP_SHARD_COUNT>    ipShards_;

    [[nodiscard]] size_t  ipShardIndex (const string & ip) const;

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
    int  keepAliveMaxRequests_{100};
    int  writeTimeoutSeconds_{10};

};
