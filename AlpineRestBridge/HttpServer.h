/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <HttpRouter.h>
#include <TcpAcceptor.h>
#include <TcpTransport.h>
#include <memory>
#include <atomic>
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

    void  handleConnection (TcpTransport * transport);

    HttpRouter &   router_;
    TcpAcceptor    acceptor_;
    bool           running_;

    std::atomic<int> activeConnections_{0};
    static constexpr int MAX_CONNECTIONS = 16;

};
