/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <HttpRouter.h>
#include <asio.hpp>
#include <memory>
#include <atomic>
#include <vector>
#include <thread>


class HttpServer
{
  public:

    HttpServer (HttpRouter & router);
    ~HttpServer ();

    bool  start (ulong   ipAddress,
                 ushort  port);

    void  stop ();


  private:

    void  doAccept ();
    void  handleConnection (asio::ip::tcp::socket socket);

    HttpRouter &   router_;
    asio::io_context  ioContext_;
    std::unique_ptr<asio::ip::tcp::acceptor>  acceptor_;
    std::vector<std::thread>  workers_;
    bool  running_{false};

    std::atomic<int>  activeConnections_{0};
    static constexpr int MAX_CONNECTIONS = 64;
    static constexpr int THREAD_POOL_SIZE = 4;

};
