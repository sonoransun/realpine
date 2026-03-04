/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <HttpServer.h>
#include <HttpRequest.h>
#include <HttpResponse.h>
#include <WebSocketSession.h>
#include <RateLimiter.h>
#include <Log.h>
#include <format>

#ifdef ALPINE_TLS_ENABLED
#include <TlsContext.h>
#include <openssl/ssl.h>
#endif

static constexpr ulong MAX_BODY_SIZE = 65536;


HttpServer::HttpServer (HttpRouter & router)
    : router_(router)
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
    try {
        asio::ip::address addr;
        if (ipAddress == 0) {
            addr = asio::ip::address_v4::any();
        } else {
            addr = asio::ip::make_address_v4(static_cast<uint32_t>(ipAddress));
        }

        auto endpoint = asio::ip::tcp::endpoint(addr, port);
        acceptor_ = std::make_unique<asio::ip::tcp::acceptor>(ioContext_);
        acceptor_->open(endpoint.protocol());
        acceptor_->set_option(asio::ip::tcp::acceptor::reuse_address(true));
        acceptor_->bind(endpoint);
        acceptor_->listen(asio::socket_base::max_listen_connections);

        running_ = true;
        doAccept();

        for (int i = 0; i < THREAD_POOL_SIZE; ++i) {
            workers_.emplace_back([this]() {
                ioContext_.run();
            });
        }

        Log::Info(std::format("HttpServer started on port {}", port));
        return true;
    } catch (const std::exception & e) {
        Log::Error("HttpServer::start failed: "s + e.what());
        return false;
    }
}


#ifdef ALPINE_TLS_ENABLED
bool
HttpServer::startTls (ulong         ipAddress,
                      ushort        port,
                      TlsContext &  tlsCtx)
{
    if (!tlsCtx.isInitialized()) {
        Log::Error("HttpServer::startTls: TlsContext not initialized");
        return false;
    }

    try {
        tlsContext_ = &tlsCtx;

        // Create an asio ssl context backed by the same OpenSSL SSL_CTX.
        // We create a fresh context and replace its native handle.
        sslContext_ = std::make_unique<asio::ssl::context>(asio::ssl::context::tlsv12);
        auto * nativeCtx = sslContext_->native_handle();

        // Copy certificate and key from the TlsContext's SSL_CTX
        auto * srcCtx = tlsCtx.context();
        if (SSL_CTX_get0_certificate(srcCtx)) {
            X509 * cert = SSL_CTX_get0_certificate(srcCtx);
            SSL_CTX_use_certificate(nativeCtx, cert);
        }

        EVP_PKEY * pkey = SSL_CTX_get0_privatekey(srcCtx);
        if (pkey)
            SSL_CTX_use_PrivateKey(nativeCtx, pkey);

        sslContext_->set_options(asio::ssl::context::default_workarounds |
                                asio::ssl::context::no_sslv2 |
                                asio::ssl::context::no_sslv3);

        asio::ip::address addr;
        if (ipAddress == 0) {
            addr = asio::ip::address_v4::any();
        } else {
            addr = asio::ip::make_address_v4(static_cast<uint32_t>(ipAddress));
        }

        auto endpoint = asio::ip::tcp::endpoint(addr, port);
        acceptor_ = std::make_unique<asio::ip::tcp::acceptor>(ioContext_);
        acceptor_->open(endpoint.protocol());
        acceptor_->set_option(asio::ip::tcp::acceptor::reuse_address(true));
        acceptor_->bind(endpoint);
        acceptor_->listen(asio::socket_base::max_listen_connections);

        running_ = true;
        doAcceptTls();

        for (int i = 0; i < THREAD_POOL_SIZE; ++i) {
            workers_.emplace_back([this]() {
                ioContext_.run();
            });
        }

        Log::Info(std::format("HttpServer (TLS) started on port {}", port));
        return true;
    } catch (const std::exception & e) {
        Log::Error("HttpServer::startTls failed: "s + e.what());
        return false;
    }
}
#endif


void
HttpServer::stop ()
{
    if (!running_)
        return;
    running_ = false;

    if (acceptor_) {
        asio::error_code ec;
        acceptor_->close(ec);
    }
    ioContext_.stop();

    for (auto & w : workers_) {
        if (w.joinable())
            w.join();
    }
    workers_.clear();

#ifdef ALPINE_TLS_ENABLED
    sslContext_.reset();
    tlsContext_ = nullptr;
#endif

    Log::Info("HttpServer stopped");
}


void
HttpServer::doAccept ()
{
    if (!running_)
        return;
    acceptor_->async_accept([this](asio::error_code ec, asio::ip::tcp::socket socket) {
        if (!ec && running_) {
            if (activeConnections_ < MAX_CONNECTIONS) {
                ++activeConnections_;
                asio::post(ioContext_, [this, sock = std::move(socket)]() mutable {
                    handleConnection(std::move(sock));
                });
            } else {
                Log::Error("HttpServer: max connections reached, rejecting");
                asio::error_code closeEc;
                socket.close(closeEc);
            }
        }
        doAccept();
    });
}


#ifdef ALPINE_TLS_ENABLED
void
HttpServer::doAcceptTls ()
{
    if (!running_)
        return;
    acceptor_->async_accept([this](asio::error_code ec, asio::ip::tcp::socket socket) {
        if (!ec && running_) {
            if (activeConnections_ < MAX_CONNECTIONS) {
                ++activeConnections_;
                auto stream = std::make_shared<asio::ssl::stream<asio::ip::tcp::socket>>(
                    std::move(socket), *sslContext_);
                asio::post(ioContext_, [this, stream]() mutable {
                    handleTlsConnection(std::move(*stream));
                });
            } else {
                Log::Error("HttpServer: max connections reached, rejecting");
                asio::error_code closeEc;
                socket.close(closeEc);
            }
        }
        doAcceptTls();
    });
}
#endif


void
HttpServer::handleConnection (asio::ip::tcp::socket socket)
{
    try {
        // Rate limiting check
        auto remoteEp = socket.remote_endpoint();
        auto clientIp = remoteEp.address().to_string();
        if (!RateLimiter::allowRequest(clientIp)) {
            asio::error_code ec;
            auto resp = HttpResponse(429, "Too Many Requests");
            resp.setJsonBody("{\"error\":\"Rate limit exceeded\"}");
            auto respStr = resp.build();
            asio::write(socket, asio::buffer(respStr), ec);
            --activeConnections_;
            return;
        }

        // Peek at the request to check for WebSocket upgrade
        asio::error_code ec;
        std::vector<byte> buffer(65536);
        auto bytesRead = socket.read_some(asio::buffer(buffer.data(), buffer.size()), ec);
        if (ec || bytesRead == 0) {
            --activeConnections_;
            return;
        }

        HttpRequest request;
        if (!HttpRequest::parse(buffer.data(), bytesRead, request)) {
            auto resp = HttpResponse::badRequest("Malformed HTTP request");
            auto respStr = resp.build();
            asio::write(socket, asio::buffer(respStr), ec);
            --activeConnections_;
            return;
        }

        if (WebSocketSession::isWebSocketUpgrade(request.headers)) {
            upgradeToWebSocket(std::move(socket), request);
            return;
        }

        // Normal HTTP: dispatch
        auto response = router_.dispatch(request);
        auto responseStr = response.build();
        asio::write(socket, asio::buffer(responseStr), ec);

    } catch (const std::exception & e) {
        Log::Error("HttpServer::handleConnection exception: "s + e.what());
    }

    --activeConnections_;
}


#ifdef ALPINE_TLS_ENABLED
void
HttpServer::handleTlsConnection (asio::ssl::stream<asio::ip::tcp::socket> stream)
{
    try {
        // Perform TLS handshake
        asio::error_code hsEc;
        stream.handshake(asio::ssl::stream_base::server, hsEc);
        if (hsEc) {
            Log::Error("HttpServer: TLS handshake failed: "s + hsEc.message());
            --activeConnections_;
            return;
        }

        // Rate limiting check
        auto remoteEp = stream.lowest_layer().remote_endpoint();
        auto clientIp = remoteEp.address().to_string();
        if (!RateLimiter::allowRequest(clientIp)) {
            asio::error_code ec;
            auto resp = HttpResponse(429, "Too Many Requests");
            resp.setJsonBody("{\"error\":\"Rate limit exceeded\"}");
            auto respStr = resp.build();
            asio::write(stream, asio::buffer(respStr), ec);
            --activeConnections_;
            return;
        }

        processRequest(stream);

        // Graceful TLS shutdown
        asio::error_code shutdownEc;
        stream.shutdown(shutdownEc);

    } catch (const std::exception & e) {
        Log::Error("HttpServer::handleTlsConnection exception: "s + e.what());
    }

    --activeConnections_;
}
#endif


void
HttpServer::upgradeToWebSocket (asio::ip::tcp::socket  socket,
                                const HttpRequest &    request)
{
    auto keyIt = request.headers.find("sec-websocket-key");
    if (keyIt == request.headers.end()) {
        asio::error_code ec;
        auto resp = HttpResponse::badRequest("Missing Sec-WebSocket-Key header");
        auto respStr = resp.build();
        asio::write(socket, asio::buffer(respStr), ec);
        --activeConnections_;
        return;
    }

    // Send handshake response
    auto handshake = WebSocketSession::buildHandshakeResponse(keyIt->second);
    asio::error_code ec;
    asio::write(socket, asio::buffer(handshake), ec);
    if (ec) {
        Log::Error("WebSocket handshake write failed: "s + ec.message());
        --activeConnections_;
        return;
    }

    Log::Info("WebSocket connection established"s);

    // Create session and run it (blocking on this thread until close)
    auto session = std::make_shared<WebSocketSession>(std::move(socket));
    session->start();

    --activeConnections_;
}


template <typename Stream>
void
HttpServer::processRequest (Stream & stream)
{
    asio::error_code ec;

    std::vector<byte> buffer(65536);
    ulong totalRead = 0;

    // Read headers first
    auto bytesRead = stream.read_some(asio::buffer(buffer.data(), buffer.size()), ec);
    if (ec || bytesRead == 0)
        return;
    totalRead = bytesRead;

    // Parse the request
    HttpRequest request;
    if (!HttpRequest::parse(buffer.data(), totalRead, request)) {
        auto resp = HttpResponse::badRequest("Malformed HTTP request");
        auto respStr = resp.build();
        asio::write(stream, asio::buffer(respStr), ec);
        return;
    }

    // Check for Content-Length and read body if needed
    auto clIt = request.headers.find("content-length");
    if (clIt != request.headers.end()) {
        ulong contentLength = std::stoul(clIt->second);
        if (contentLength > MAX_BODY_SIZE) {
            auto resp = HttpResponse(413, "Payload Too Large");
            resp.setBody("Request body too large");
            auto respStr = resp.build();
            asio::write(stream, asio::buffer(respStr), ec);
            return;
        }

        while (request.body.length() < contentLength) {
            bytesRead = stream.read_some(asio::buffer(buffer.data(), buffer.size()), ec);
            if (ec || bytesRead == 0)
                break;
            request.body.append(reinterpret_cast<const char *>(buffer.data()), bytesRead);
        }
    }

    // Dispatch
    auto response = router_.dispatch(request);
    auto responseStr = response.build();
    asio::write(stream, asio::buffer(responseStr), ec);
}
