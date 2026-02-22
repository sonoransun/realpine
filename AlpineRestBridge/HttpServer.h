/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <HttpRouter.h>
#include <TcpAcceptor.h>
#include <TcpTransport.h>
#include <memory>


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

};
