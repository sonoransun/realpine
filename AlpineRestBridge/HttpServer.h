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

    HttpRouter &   router_;
    asio::io_context  ioContext_;
    std::unique_ptr<asio::ip::tcp::acceptor>  acceptor_;
    std::vector<std::thread>  workers_;
    bool  running_{false};

#ifdef ALPINE_TLS_ENABLED
    TlsContext *  tlsContext_{nullptr};
    std::unique_ptr<asio::ssl::context>  sslContext_;
#endif

    std::atomic<int>  activeConnections_{0};
    static constexpr int MAX_CONNECTIONS = 64;
    static constexpr int THREAD_POOL_SIZE = 4;

};
