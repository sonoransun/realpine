/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <HttpServer.h>
#include <Log.h>
#include <cstdio>


HttpServer::HttpServer (HttpRouter & router)
    : router_(router),
      running_(false)
{
}


HttpServer::~HttpServer ()
{
    stop();
}


bool
HttpServer::start (ulong   ipAddress,
                   ushort  port)
{
    if (!acceptor_.create(ipAddress, port))
    {
        Log::Error("HttpServer: Failed to create acceptor");
        return false;
    }

    Log::Info("HttpServer: Listening on port "s + std::to_string((uint)port));

    running_ = true;

    while (running_)
    {
        std::unique_ptr<TcpTransport> transport;

        if (!acceptor_.accept(transport))
        {
            Log::Error("HttpServer: Accept failed");
            continue;
        }

        handleConnection(transport.get());
    }

    return true;
}


void
HttpServer::stop ()
{
    running_ = false;
    acceptor_.close();
}


void
HttpServer::handleConnection (TcpTransport * transport)
{
    byte buffer[8192];
    ulong bytesRead = 0;

    if (!transport->receive(buffer, sizeof(buffer), bytesRead))
        return;

    HttpRequest request;

    if (!HttpRequest::parse(buffer, bytesRead, request))
    {
        HttpResponse resp = HttpResponse::badRequest("Malformed request");
        string responseStr = resp.build();
        transport->send((const byte *)responseStr.c_str(), responseStr.length());
    }

    Log::Debug("HttpServer: "s + request.method + " " + request.path);

    HttpResponse response = router_.dispatch(request);
    string responseStr = response.build();
    transport->send((const byte *)responseStr.c_str(), responseStr.length());
}
