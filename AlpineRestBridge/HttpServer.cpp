/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <HttpServer.h>
#include <Log.h>
#include <cstdio>
#include <thread>
#include <sys/socket.h>
#include <sys/time.h>


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

        if (activeConnections_.load() >= MAX_CONNECTIONS)
        {
            HttpResponse resp(503, "Service Unavailable");
            resp.setJsonBody("{\"error\":\"Too many connections\"}");
            string responseStr = resp.build();
            transport->send((const byte*)responseStr.c_str(), responseStr.length());
            continue;
        }

        TcpTransport* rawTransport = transport.release();
        activeConnections_.fetch_add(1);
        std::thread([this, rawTransport]() {
            handleConnection(rawTransport);
            delete rawTransport;
            activeConnections_.fetch_sub(1);
        }).detach();
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
    // Set socket receive timeout
    struct timeval tv;
    tv.tv_sec = 30;
    tv.tv_usec = 0;
    setsockopt(transport->getFd(), SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    static constexpr ulong MAX_BODY_SIZE = 65536;
    byte buffer[8192];
    ulong totalRead = 0;
    string rawData;

    // First read to get headers
    ulong bytesRead = 0;
    if (!transport->receive(buffer, sizeof(buffer), bytesRead))
        return;
    rawData.append((const char*)buffer, bytesRead);
    totalRead = bytesRead;

    // Check for Content-Length and read more if needed
    ulong headerEnd = rawData.find("\r\n\r\n");
    if (headerEnd != string::npos) {
        // Extract Content-Length from raw headers
        ulong clPos = rawData.find("content-length:");
        if (clPos == string::npos)
            clPos = rawData.find("Content-Length:");

        ulong contentLength = 0;
        if (clPos != string::npos) {
            ulong valStart = clPos + 15; // length of "content-length:"
            while (valStart < rawData.length() && rawData[valStart] == ' ')
                ++valStart;
            string clStr;
            while (valStart < rawData.length() && rawData[valStart] >= '0' && rawData[valStart] <= '9') {
                clStr += rawData[valStart];
                ++valStart;
            }
            if (!clStr.empty())
                contentLength = strtoul(clStr.c_str(), nullptr, 10);
        }

        if (contentLength > MAX_BODY_SIZE) {
            HttpResponse resp(413, "Payload Too Large");
            resp.setJsonBody("{\"error\":\"Request body too large\"}");
            string responseStr = resp.build();
            transport->send((const byte*)responseStr.c_str(), responseStr.length());
            return;
        }

        ulong bodyStart = headerEnd + 4;
        ulong expectedTotal = bodyStart + contentLength;

        while (totalRead < expectedTotal && totalRead < MAX_BODY_SIZE + 8192) {
            bytesRead = 0;
            if (!transport->receive(buffer, sizeof(buffer), bytesRead))
                break;
            rawData.append((const char*)buffer, bytesRead);
            totalRead += bytesRead;
        }
    }

    HttpRequest request;
    if (!HttpRequest::parse((const byte*)rawData.c_str(), rawData.length(), request))
    {
        HttpResponse resp = HttpResponse::badRequest("Malformed request");
        string responseStr = resp.build();
        transport->send((const byte*)responseStr.c_str(), responseStr.length());
        return;
    }

    Log::Debug("HttpServer: "s + request.method + " " + request.path);

    HttpResponse response = router_.dispatch(request);
    string responseStr = response.build();
    transport->send((const byte *)responseStr.c_str(), responseStr.length());
}
