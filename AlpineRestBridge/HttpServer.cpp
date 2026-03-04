/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <HttpServer.h>
#include <HttpRequest.h>
#include <HttpResponse.h>
#include <WebSocketSession.h>
#include <RateLimiter.h>
#include <RestBridgeConfig.h>
#include <Log.h>

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
    // Load configurable limits
    minThreads_          = RestBridgeConfig::getHttpMinThreads();
    maxThreads_          = RestBridgeConfig::getHttpMaxThreads();
    maxConnections_      = RestBridgeConfig::getHttpMaxConnections();
    maxConnectionsPerIp_ = RestBridgeConfig::getHttpMaxConnectionsPerIp();
    idleTimeoutSeconds_  = RestBridgeConfig::getHttpIdleTimeoutSeconds();

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

        // Start pool monitor timer
        poolMonitorTimer_ = std::make_unique<asio::steady_timer>(ioContext_);
        monitorPool();

        // Spawn initial worker threads
        for (int i = 0; i < minThreads_; ++i)
            spawnWorker();

        Log::Info("HttpServer started on port "s + std::to_string(port)
                  + " (threads "s + std::to_string(minThreads_) + "-"s + std::to_string(maxThreads_)
                  + ", max connections "s + std::to_string(maxConnections_) + ")"s);
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

    // Load configurable limits
    minThreads_          = RestBridgeConfig::getHttpMinThreads();
    maxThreads_          = RestBridgeConfig::getHttpMaxThreads();
    maxConnections_      = RestBridgeConfig::getHttpMaxConnections();
    maxConnectionsPerIp_ = RestBridgeConfig::getHttpMaxConnectionsPerIp();
    idleTimeoutSeconds_  = RestBridgeConfig::getHttpIdleTimeoutSeconds();

    try {
        tlsContext_ = &tlsCtx;

        sslContext_ = std::make_unique<asio::ssl::context>(asio::ssl::context::tlsv12);
        auto * nativeCtx = sslContext_->native_handle();

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

        // Start pool monitor timer
        poolMonitorTimer_ = std::make_unique<asio::steady_timer>(ioContext_);
        monitorPool();

        // Spawn initial worker threads
        for (int i = 0; i < minThreads_; ++i)
            spawnWorker();

        Log::Info("HttpServer (TLS) started on port "s + std::to_string(port)
                  + " (threads "s + std::to_string(minThreads_) + "-"s + std::to_string(maxThreads_)
                  + ", max connections "s + std::to_string(maxConnections_) + ")"s);
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

    if (poolMonitorTimer_) {
        poolMonitorTimer_->cancel();
    }

    if (acceptor_) {
        asio::error_code ec;
        acceptor_->close(ec);
    }
    ioContext_.stop();

    {
        std::lock_guard lock(workerMutex_);
        for (auto & w : workers_) {
            if (w.joinable())
                w.join();
        }
        workers_.clear();
    }

#ifdef ALPINE_TLS_ENABLED
    sslContext_.reset();
    tlsContext_ = nullptr;
#endif

    Log::Info("HttpServer stopped");
}


void
HttpServer::spawnWorker ()
{
    std::lock_guard lock(workerMutex_);
    if (activeWorkers_ >= maxThreads_)
        return;

    ++activeWorkers_;
    workers_.emplace_back([this]() {
        auto lastActivity = std::chrono::steady_clock::now();

        while (running_) {
            // Run handlers for up to 1 second, then check idle
            auto count = ioContext_.run_one_for(std::chrono::seconds(1));
            if (count > 0) {
                lastActivity = std::chrono::steady_clock::now();
            } else {
                // No work done — check idle timeout
                auto idle = std::chrono::steady_clock::now() - lastActivity;
                if (idle >= std::chrono::seconds(30) && activeWorkers_ > minThreads_) {
                    --activeWorkers_;
                    return;
                }
            }
        }
        --activeWorkers_;
    });
}


void
HttpServer::monitorPool ()
{
    if (!running_)
        return;

    poolMonitorTimer_->expires_after(std::chrono::seconds(1));
    poolMonitorTimer_->async_wait([this](asio::error_code ec) {
        if (ec || !running_)
            return;

        // If busy workers approach thread count, spawn more
        int current = activeWorkers_.load();
        int busy    = busyWorkers_.load();
        if (busy > 0 && busy >= current - 1 && current < maxThreads_) {
            int toSpawn = std::min(2, maxThreads_ - current);
            for (int i = 0; i < toSpawn; ++i)
                spawnWorker();
            Log::Debug("HttpServer: scaled thread pool to "s + std::to_string(activeWorkers_.load()));
        }

        // Drain any queued connections
        drainQueue();

        monitorPool();
    });
}


void
HttpServer::sendServiceUnavailable (asio::ip::tcp::socket & socket)
{
    auto resp = HttpResponse(503, "Service Unavailable");
    resp.setHeader("Retry-After"s, "5"s);
    resp.setJsonBody("{\"error\":\"Server at capacity, please retry\"}"s);
    auto respStr = resp.build();
    asio::error_code ec;
    asio::write(socket, asio::buffer(respStr), ec);
}


bool
HttpServer::acquireConnectionSlot (const string & ip)
{
    std::lock_guard lock(ipCountMutex_);
    auto & count = ipConnectionCounts_[ip];
    if (count >= maxConnectionsPerIp_)
        return false;
    ++count;
    return true;
}


void
HttpServer::releaseConnectionSlot (const string & ip)
{
    std::lock_guard lock(ipCountMutex_);
    auto it = ipConnectionCounts_.find(ip);
    if (it != ipConnectionCounts_.end()) {
        --it->second;
        if (it->second <= 0)
            ipConnectionCounts_.erase(it);
    }
}


void
HttpServer::drainQueue ()
{
    std::lock_guard lock(queueMutex_);
    while (!connectionQueue_.empty() && activeConnections_ < maxConnections_) {
        auto socket = std::move(connectionQueue_.front());
        connectionQueue_.pop();
        ++activeConnections_;
        asio::post(ioContext_, [this, sock = std::move(socket)]() mutable {
            handleConnection(std::move(sock));
        });
    }
}


void
HttpServer::doAccept ()
{
    if (!running_)
        return;
    acceptor_->async_accept([this](asio::error_code ec, asio::ip::tcp::socket socket) {
        if (!ec && running_) {
            if (activeConnections_ < maxConnections_) {
                ++activeConnections_;
                asio::post(ioContext_, [this, sock = std::move(socket)]() mutable {
                    handleConnection(std::move(sock));
                });
            } else {
                // Try to queue the connection; if queue full, reject with 503
                bool queued = false;
                {
                    std::lock_guard lock(queueMutex_);
                    if (static_cast<int>(connectionQueue_.size()) < CONNECTION_QUEUE_CAPACITY) {
                        connectionQueue_.push(std::move(socket));
                        queued = true;
                    }
                }
                if (!queued) {
                    sendServiceUnavailable(socket);
                    asio::error_code closeEc;
                    socket.close(closeEc);
                }
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
            if (activeConnections_ < maxConnections_) {
                ++activeConnections_;
                auto stream = std::make_shared<asio::ssl::stream<asio::ip::tcp::socket>>(
                    std::move(socket), *sslContext_);
                asio::post(ioContext_, [this, stream]() mutable {
                    handleTlsConnection(std::move(*stream));
                });
            } else {
                bool queued = false;
                {
                    std::lock_guard lock(queueMutex_);
                    if (static_cast<int>(connectionQueue_.size()) < CONNECTION_QUEUE_CAPACITY) {
                        connectionQueue_.push(std::move(socket));
                        queued = true;
                    }
                }
                if (!queued) {
                    sendServiceUnavailable(socket);
                    asio::error_code closeEc;
                    socket.close(closeEc);
                }
            }
        }
        doAcceptTls();
    });
}
#endif


void
HttpServer::handleConnection (asio::ip::tcp::socket socket)
{
    string clientIp;
    try {
        auto remoteEp = socket.remote_endpoint();
        clientIp = remoteEp.address().to_string();

        // Per-IP connection limit
        if (!acquireConnectionSlot(clientIp)) {
            auto resp = HttpResponse(429, "Too Many Requests");
            resp.setJsonBody("{\"error\":\"Per-IP connection limit exceeded\"}"s);
            auto respStr = resp.build();
            asio::error_code ec;
            asio::write(socket, asio::buffer(respStr), ec);
            --activeConnections_;
            return;
        }

        ++busyWorkers_;

        // Rate limiting check
        if (!RateLimiter::allowRequest(clientIp)) {
            asio::error_code ec;
            auto resp = HttpResponse(429, "Too Many Requests");
            resp.setJsonBody("{\"error\":\"Rate limit exceeded\"}"s);
            auto respStr = resp.build();
            asio::write(socket, asio::buffer(respStr), ec);
            --busyWorkers_;
            releaseConnectionSlot(clientIp);
            --activeConnections_;
            return;
        }

        // Set idle timeout on the socket
        if (idleTimeoutSeconds_ > 0) {
            asio::socket_base::linger lingerOpt(true, idleTimeoutSeconds_);
            socket.set_option(lingerOpt);
            // Use SO_RCVTIMEO for read idle timeout
            struct timeval tv;
            tv.tv_sec  = idleTimeoutSeconds_;
            tv.tv_usec = 0;
            setsockopt(socket.native_handle(), SOL_SOCKET, SO_RCVTIMEO,
                       reinterpret_cast<const char *>(&tv), sizeof(tv));
        }

        // Peek at the request to check for WebSocket upgrade
        asio::error_code ec;
        std::vector<byte> buffer(65536);
        auto bytesRead = socket.read_some(asio::buffer(buffer.data(), buffer.size()), ec);
        if (ec || bytesRead == 0) {
            --busyWorkers_;
            releaseConnectionSlot(clientIp);
            --activeConnections_;
            return;
        }

        HttpRequest request;
        if (!HttpRequest::parse(buffer.data(), bytesRead, request)) {
            auto resp = HttpResponse::badRequest("Malformed HTTP request");
            auto respStr = resp.build();
            asio::write(socket, asio::buffer(respStr), ec);
            --busyWorkers_;
            releaseConnectionSlot(clientIp);
            --activeConnections_;
            return;
        }

        if (WebSocketSession::isWebSocketUpgrade(request.headers)) {
            --busyWorkers_;
            upgradeToWebSocket(std::move(socket), request);
            releaseConnectionSlot(clientIp);
            return;
        }

        // Normal HTTP: dispatch
        auto response = router_.dispatch(request);
        auto responseStr = response.build();
        asio::write(socket, asio::buffer(responseStr), ec);

    } catch (const std::exception & e) {
        Log::Error("HttpServer::handleConnection exception: "s + e.what());
    }

    --busyWorkers_;
    if (!clientIp.empty())
        releaseConnectionSlot(clientIp);
    --activeConnections_;
}


#ifdef ALPINE_TLS_ENABLED
void
HttpServer::handleTlsConnection (asio::ssl::stream<asio::ip::tcp::socket> stream)
{
    string clientIp;
    try {
        // Perform TLS handshake
        asio::error_code hsEc;
        stream.handshake(asio::ssl::stream_base::server, hsEc);
        if (hsEc) {
            Log::Error("HttpServer: TLS handshake failed: "s + hsEc.message());
            --activeConnections_;
            return;
        }

        auto remoteEp = stream.lowest_layer().remote_endpoint();
        clientIp = remoteEp.address().to_string();

        // Per-IP connection limit
        if (!acquireConnectionSlot(clientIp)) {
            auto resp = HttpResponse(429, "Too Many Requests");
            resp.setJsonBody("{\"error\":\"Per-IP connection limit exceeded\"}"s);
            auto respStr = resp.build();
            asio::error_code ec;
            asio::write(stream, asio::buffer(respStr), ec);
            --activeConnections_;
            return;
        }

        ++busyWorkers_;

        // Rate limiting check
        if (!RateLimiter::allowRequest(clientIp)) {
            asio::error_code ec;
            auto resp = HttpResponse(429, "Too Many Requests");
            resp.setJsonBody("{\"error\":\"Rate limit exceeded\"}"s);
            auto respStr = resp.build();
            asio::write(stream, asio::buffer(respStr), ec);
            --busyWorkers_;
            releaseConnectionSlot(clientIp);
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

    --busyWorkers_;
    if (!clientIp.empty())
        releaseConnectionSlot(clientIp);
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
