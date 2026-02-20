///////
///
///  Copyright (C) 2026  sonoransun
///
///  Permission is hereby granted, free of charge, to any person obtaining a copy
///  of this software and associated documentation files (the "Software"), to deal
///  in the Software without restriction, including without limitation the rights
///  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
///  copies of the Software, and to permit persons to whom the Software is
///  furnished to do so, subject to the following conditions:
///
///  The above copyright notice and this permission notice shall be included in all
///  copies or substantial portions of the Software.
///
///  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
///  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
///  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
///  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
///  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
///  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
///  SOFTWARE.
///
///////


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
