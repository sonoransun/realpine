/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <HttpServer.h>
#include <HttpRequest.h>
#include <HttpResponse.h>
#include <Compression.h>
#include <SafeParse.h>
#include <WebSocketSession.h>
#include <RateLimiter.h>
#include <RestBridgeConfig.h>
#include <MetricsHandler.h>
#include <StringUtils.h>
#include <Log.h>
#include <Platform.h>

#ifdef ALPINE_TLS_ENABLED
#include <TlsContext.h>
#include <openssl/ssl.h>
#endif

#ifdef ALPINE_TRACING_ENABLED
#include <Tracing.h>
#endif

static constexpr ulong MAX_BODY_SIZE = 65536;
static constexpr ulong READ_BUFFER_SIZE = 65536;


static string
generateRequestId ()
{
    byte buf[8];
    if (!alpine_random_bytes(buf, sizeof(buf)))
        return "00000000"s;

    static const char hexChars[] = "0123456789abcdef";
    string result;
    result.reserve(16);
    for (auto b : buf) {
        result += hexChars[(b >> 4) & 0x0F];
        result += hexChars[b & 0x0F];
    }
    return result;
}


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
    minThreads_            = RestBridgeConfig::getHttpMinThreads();
    maxThreads_            = RestBridgeConfig::getHttpMaxThreads();
    maxConnections_        = RestBridgeConfig::getHttpMaxConnections();
    maxConnectionsPerIp_   = RestBridgeConfig::getHttpMaxConnectionsPerIp();
    idleTimeoutSeconds_    = RestBridgeConfig::getHttpIdleTimeoutSeconds();
    keepAliveMaxRequests_  = RestBridgeConfig::getHttpKeepAliveMaxRequests();
    writeTimeoutSeconds_   = RestBridgeConfig::getHttpWriteTimeoutSeconds();

    if (minThreads_ > maxThreads_) {
        Log::Error("HTTP Min Threads ("s + std::to_string(minThreads_) + ") > Max Threads ("s + std::to_string(maxThreads_) + "). Adjusting max to match min."s);
        maxThreads_ = minThreads_;
    }

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
    minThreads_            = RestBridgeConfig::getHttpMinThreads();
    maxThreads_            = RestBridgeConfig::getHttpMaxThreads();
    maxConnections_        = RestBridgeConfig::getHttpMaxConnections();
    maxConnectionsPerIp_   = RestBridgeConfig::getHttpMaxConnectionsPerIp();
    idleTimeoutSeconds_    = RestBridgeConfig::getHttpIdleTimeoutSeconds();
    keepAliveMaxRequests_  = RestBridgeConfig::getHttpKeepAliveMaxRequests();
    writeTimeoutSeconds_   = RestBridgeConfig::getHttpWriteTimeoutSeconds();

    if (minThreads_ > maxThreads_) {
        Log::Error("HTTP Min Threads ("s + std::to_string(minThreads_) + ") > Max Threads ("s + std::to_string(maxThreads_) + "). Adjusting max to match min."s);
        maxThreads_ = minThreads_;
    }

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

        // Harden cipher suites: modern AEAD ciphers only
        SSL_CTX_set_ciphersuites(nativeCtx,
            "TLS_AES_256_GCM_SHA384:TLS_AES_128_GCM_SHA256:TLS_CHACHA20_POLY1305_SHA256");
        SSL_CTX_set_cipher_list(nativeCtx,
            "ECDHE+AESGCM:ECDHE+CHACHA20:DHE+AESGCM:!RC4:!DES:!3DES:!MD5:!aNULL:!eNULL:!EXPORT");
        SSL_CTX_set_options(nativeCtx, SSL_OP_CIPHER_SERVER_PREFERENCE);

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


int
HttpServer::getActiveConnections () const
{
    return activeConnections_.load();
}


size_t
HttpServer::ipShardIndex (const string & ip) const
{
    return std::hash<string>{}(ip) % IP_SHARD_COUNT;
}


bool
HttpServer::acquireConnectionSlot (const string & ip)
{
    auto & shard = ipShards_[ipShardIndex(ip)];
    std::lock_guard lock(shard.mutex);
    auto & count = shard.counts[ip];
    if (count >= maxConnectionsPerIp_) {
        return false;
    }
    ++count;
    return true;
}


void
HttpServer::releaseConnectionSlot (const string & ip)
{
    auto & shard = ipShards_[ipShardIndex(ip)];
    std::lock_guard lock(shard.mutex);
    auto it = shard.counts.find(ip);
    if (it != shard.counts.end()) {
        --it->second;
        if (it->second <= 0) {
            shard.counts.erase(it);
        }
    }
}


// --- ConnectionGuard RAII ---

HttpServer::ConnectionGuard::ConnectionGuard (HttpServer & server, string clientIp)
    : server_(server),
      clientIp_(std::move(clientIp))
{
}


HttpServer::ConnectionGuard::~ConnectionGuard ()
{
    if (busy_) {
        --server_.busyWorkers_;
    }
    if (slotAcquired_ && !clientIp_.empty()) {
        server_.releaseConnectionSlot(clientIp_);
    }
    --server_.activeConnections_;
}


void
HttpServer::ConnectionGuard::setBusy ()
{
    if (!busy_) {
        ++server_.busyWorkers_;
        busy_ = true;
    }
}


void
HttpServer::ConnectionGuard::clearBusy ()
{
    if (busy_) {
        --server_.busyWorkers_;
        busy_ = false;
    }
}


void
HttpServer::ConnectionGuard::setSlotAcquired ()
{
    slotAcquired_ = true;
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
        clientIp = RateLimiter::normalizeIp(remoteEp.address().to_string());
    } catch (const std::exception & e) {
        Log::Error("HttpServer: failed to get remote endpoint: "s + e.what());
        --activeConnections_;
        return;
    }

    ConnectionGuard guard(*this, clientIp);

    try {
        // Enable TCP_NODELAY to avoid Nagle delays on small responses
        asio::error_code noDelayEc;
        socket.set_option(asio::ip::tcp::no_delay(true), noDelayEc);

        // Per-IP connection limit
        if (!acquireConnectionSlot(clientIp)) {
            auto resp = HttpResponse(429, "Too Many Requests");
            resp.setConnectionClose();
            resp.setJsonBody("{\"error\":\"Per-IP connection limit exceeded\"}"s);
            auto respStr = resp.build();
            asio::error_code ec;
            asio::write(socket, asio::buffer(respStr), ec);
            return;
        }
        guard.setSlotAcquired();

        // Rate limiting check (use pre-normalized IP)
        if (!RateLimiter::allowRequestNormalized(clientIp)) {
            auto resp = HttpResponse(429, "Too Many Requests");
            resp.setConnectionClose();
            resp.setJsonBody("{\"error\":\"Rate limit exceeded\"}"s);
            auto respStr = resp.build();
            asio::error_code ec;
            asio::write(socket, asio::buffer(respStr), ec);
            return;
        }

        // Set idle timeout (receive) on the socket
        if (idleTimeoutSeconds_ > 0) {
            asio::socket_base::linger lingerOpt(true, idleTimeoutSeconds_);
            socket.set_option(lingerOpt);
            struct timeval tv;
            tv.tv_sec  = idleTimeoutSeconds_;
            tv.tv_usec = 0;
            if (setsockopt(socket.native_handle(), SOL_SOCKET, SO_RCVTIMEO,
                           reinterpret_cast<const char *>(&tv), sizeof(tv)) < 0) {
                Log::Error("HttpServer: setsockopt SO_RCVTIMEO failed"s);
            }
        }

        // Set write timeout on the socket
        if (writeTimeoutSeconds_ > 0) {
            struct timeval sendTv;
            sendTv.tv_sec  = writeTimeoutSeconds_;
            sendTv.tv_usec = 0;
            if (setsockopt(socket.native_handle(), SOL_SOCKET, SO_SNDTIMEO,
                           reinterpret_cast<const char *>(&sendTv), sizeof(sendTv)) < 0) {
                Log::Error("HttpServer: setsockopt SO_SNDTIMEO failed"s);
            }
        }

        // Keep-alive loop: process multiple requests on the same connection
        thread_local std::vector<byte> readBuffer(READ_BUFFER_SIZE);
        int requestCount = 0;

        while (running_) {
            // Read request using thread-local buffer to avoid per-connection allocation
            asio::error_code ec;
            auto bytesRead = socket.read_some(asio::buffer(readBuffer.data(), readBuffer.size()), ec);
            if (ec || bytesRead == 0)
                break;

            HttpRequest request;
            if (!HttpRequest::parse(readBuffer.data(), bytesRead, request)) {
                auto resp = HttpResponse::badRequest("Malformed HTTP request");
                resp.setConnectionClose();
                auto respStr = resp.build();
                asio::write(socket, asio::buffer(respStr), ec);
                break;
            }

            // WebSocket upgrade ends the keep-alive loop
            if (WebSocketSession::isWebSocketUpgrade(request.headers)) {
                upgradeToWebSocket(std::move(socket), request);
                return;
            }

            guard.setBusy();

            // Generate correlation ID
            auto requestId = generateRequestId();
            Log::setCorrelationId(requestId);

            auto startTime = std::chrono::steady_clock::now();

#ifdef ALPINE_TRACING_ENABLED
            ScopedSpan rootSpan("http.request"s);
            rootSpan.addAttribute("http.method"s, request.method);
            rootSpan.addAttribute("http.path"s, request.path);
            rootSpan.addAttribute("net.peer.ip"s, clientIp);
            rootSpan.addAttribute("http.request_id"s, requestId);
#endif

            // Dispatch the request
            auto response = router_.dispatch(request);
            response.setRequestId(requestId);

            // Compress response if client accepts it and body is large enough
            auto acceptEncoding = request.headers.find("accept-encoding"s);
            if (acceptEncoding != request.headers.end() && response.bodySize() > Compression::MIN_COMPRESS_SIZE) {
                auto encoding = Compression::selectEncoding(acceptEncoding->second);
                if (encoding != "identity"s) {
                    response.compressBody(encoding);
                }
            }

            // Determine whether to close after this response
            ++requestCount;
            auto connIt = request.headers.find("connection"s);
            bool clientRequestedClose = (connIt != request.headers.end() &&
                                         connIt->second.contains("close"s));
            bool lastRequest = clientRequestedClose ||
                               requestCount >= keepAliveMaxRequests_;

            if (lastRequest) {
                response.setConnectionClose();
            } else {
                response.setKeepAliveParams(idleTimeoutSeconds_,
                                            keepAliveMaxRequests_ - requestCount);
            }

            auto responseStr = response.build();
            asio::write(socket, asio::buffer(responseStr), ec);

            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - startTime).count();

            MetricsRegistry::recordHistogram("http_request_duration_seconds"s,
                                             elapsed / 1000.0);

#ifdef ALPINE_TRACING_ENABLED
            rootSpan.addAttribute("http.status_code"s, std::to_string(response.statusCode()));
            rootSpan.addAttribute("http.latency_ms"s, std::to_string(elapsed));
            rootSpan.setStatus(response.statusCode() < 400);
#endif

            Log::Info("request"s, {
                {"method"s,     StringUtils::sanitizeForLog(request.method)},
                {"path"s,       StringUtils::sanitizeForLog(request.path)},
                {"client_ip"s,  clientIp},
                {"status"s,     std::to_string(response.statusCode())},
                {"latency_ms"s, std::to_string(elapsed)},
                {"size"s,       std::to_string(responseStr.size())},
                {"request_id"s, requestId}
            });

            Log::clearCorrelationId();

            guard.clearBusy();

            if (ec || lastRequest)
                break;
        }

    } catch (const std::exception & e) {
        Log::Error("HttpServer::handleConnection exception: "s + e.what());
        Log::clearCorrelationId();
    }
}


#ifdef ALPINE_TLS_ENABLED
void
HttpServer::handleTlsConnection (asio::ssl::stream<asio::ip::tcp::socket> stream)
{
    string clientIp;

    // Perform TLS handshake before creating guard (can't get IP until handshake)
    try {
        asio::error_code hsEc;
        stream.handshake(asio::ssl::stream_base::server, hsEc);
        if (hsEc) {
            Log::Error("HttpServer: TLS handshake failed: "s + hsEc.message());
            --activeConnections_;
            return;
        }

        auto remoteEp = stream.lowest_layer().remote_endpoint();
        clientIp = RateLimiter::normalizeIp(remoteEp.address().to_string());
    } catch (const std::exception & e) {
        Log::Error("HttpServer: TLS connection setup failed: "s + e.what());
        --activeConnections_;
        return;
    }

    ConnectionGuard guard(*this, clientIp);

    try {
        // Enable TCP_NODELAY on the underlying socket
        asio::error_code noDelayEc;
        stream.lowest_layer().set_option(asio::ip::tcp::no_delay(true), noDelayEc);

        // Per-IP connection limit
        if (!acquireConnectionSlot(clientIp)) {
            auto resp = HttpResponse(429, "Too Many Requests");
            resp.setConnectionClose();
            resp.setJsonBody("{\"error\":\"Per-IP connection limit exceeded\"}"s);
            auto respStr = resp.build();
            asio::error_code ec;
            asio::write(stream, asio::buffer(respStr), ec);
            return;
        }
        guard.setSlotAcquired();

        // Rate limiting check (use pre-normalized IP)
        if (!RateLimiter::allowRequestNormalized(clientIp)) {
            auto resp = HttpResponse(429, "Too Many Requests");
            resp.setConnectionClose();
            resp.setJsonBody("{\"error\":\"Rate limit exceeded\"}"s);
            auto respStr = resp.build();
            asio::error_code ec;
            asio::write(stream, asio::buffer(respStr), ec);
            return;
        }

        // Set write timeout on the underlying socket
        if (writeTimeoutSeconds_ > 0) {
            struct timeval sendTv;
            sendTv.tv_sec  = writeTimeoutSeconds_;
            sendTv.tv_usec = 0;
            if (setsockopt(stream.lowest_layer().native_handle(), SOL_SOCKET, SO_SNDTIMEO,
                           reinterpret_cast<const char *>(&sendTv), sizeof(sendTv)) < 0) {
                Log::Error("HttpServer: setsockopt SO_SNDTIMEO failed"s);
            }
        }

        processRequest(stream, clientIp, guard);

        // Graceful TLS shutdown
        asio::error_code shutdownEc;
        stream.shutdown(shutdownEc);

    } catch (const std::exception & e) {
        Log::Error("HttpServer::handleTlsConnection exception: "s + e.what());
    }
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
        return;  // ConnectionGuard in caller handles cleanup
    }

    // Send handshake response
    auto handshake = WebSocketSession::buildHandshakeResponse(keyIt->second);
    asio::error_code ec;
    asio::write(socket, asio::buffer(handshake), ec);
    if (ec) {
        Log::Error("WebSocket handshake write failed: "s + ec.message());
        return;  // ConnectionGuard in caller handles cleanup
    }

    Log::Info("WebSocket connection established"s);

    // Create session and run it (blocking on this thread until close)
    auto session = std::make_shared<WebSocketSession>(std::move(socket));
    session->start();
    // ConnectionGuard in caller handles activeConnections_ cleanup
}


template <typename Stream>
void
HttpServer::processRequest (Stream &          stream,
                            const string &    clientIp,
                            ConnectionGuard & guard)
{
    // Reuse thread-local buffer to avoid per-connection allocation
    thread_local std::vector<byte> buffer(READ_BUFFER_SIZE);
    int requestCount = 0;

    while (running_) {
        asio::error_code ec;

        // Read headers first
        auto bytesRead = stream.read_some(asio::buffer(buffer.data(), buffer.size()), ec);
        if (ec || bytesRead == 0)
            break;

        ulong totalRead = bytesRead;

        // Parse the request
        HttpRequest request;
        if (!HttpRequest::parse(buffer.data(), totalRead, request)) {
            auto resp = HttpResponse::badRequest("Malformed HTTP request");
            resp.setConnectionClose();
            auto respStr = resp.build();
            asio::write(stream, asio::buffer(respStr), ec);
            break;
        }

        // Check for Content-Length and read body if needed
        auto clIt = request.headers.find("content-length"s);
        if (clIt != request.headers.end()) {
            auto contentLengthOpt = parseUlong(clIt->second);
            if (!contentLengthOpt) {
                auto resp = HttpResponse::badRequest("Invalid Content-Length");
                resp.setConnectionClose();
                auto respStr = resp.build();
                asio::write(stream, asio::buffer(respStr), ec);
                break;
            }
            ulong contentLength = *contentLengthOpt;
            if (contentLength > MAX_BODY_SIZE) {
                auto resp = HttpResponse(413, "Payload Too Large");
                resp.setConnectionClose();
                resp.setBody("Request body too large"s);
                auto respStr = resp.build();
                asio::write(stream, asio::buffer(respStr), ec);
                break;
            }

            while (request.body.length() < contentLength) {
                bytesRead = stream.read_some(asio::buffer(buffer.data(), buffer.size()), ec);
                if (ec || bytesRead == 0)
                    break;
                request.body.append(reinterpret_cast<const char *>(buffer.data()), bytesRead);
            }

            if (request.body.length() < contentLength) {
                auto resp = HttpResponse::badRequest("Incomplete request body");
                resp.setConnectionClose();
                auto respStr = resp.build();
                asio::write(stream, asio::buffer(respStr), ec);
                break;
            }
        }

        guard.setBusy();

        // Generate correlation ID
        auto requestId = generateRequestId();
        Log::setCorrelationId(requestId);

        auto startTime = std::chrono::steady_clock::now();

        // Dispatch
        auto response = router_.dispatch(request);
        response.setRequestId(requestId);

        // Compress response if client accepts it and body is large enough
        auto acceptEncoding = request.headers.find("accept-encoding"s);
        if (acceptEncoding != request.headers.end() && response.bodySize() > Compression::MIN_COMPRESS_SIZE) {
            auto encoding = Compression::selectEncoding(acceptEncoding->second);
            if (encoding != "identity"s) {
                response.compressBody(encoding);
            }
        }

        // Determine whether to close after this response
        ++requestCount;
        auto connIt = request.headers.find("connection"s);
        bool clientRequestedClose = (connIt != request.headers.end() &&
                                     connIt->second.contains("close"s));
        bool lastRequest = clientRequestedClose ||
                           requestCount >= keepAliveMaxRequests_;

        if (lastRequest) {
            response.setConnectionClose();
        } else {
            response.setKeepAliveParams(idleTimeoutSeconds_,
                                        keepAliveMaxRequests_ - requestCount);
        }

        auto responseStr = response.build();
        asio::write(stream, asio::buffer(responseStr), ec);

        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - startTime).count();

        MetricsRegistry::recordHistogram("http_request_duration_seconds"s,
                                         elapsed / 1000.0);

        Log::Info("request"s, {
            {"method"s,     StringUtils::sanitizeForLog(request.method)},
            {"path"s,       StringUtils::sanitizeForLog(request.path)},
            {"client_ip"s,  clientIp},
            {"status"s,     std::to_string(response.statusCode())},
            {"latency_ms"s, std::to_string(elapsed)},
            {"size"s,       std::to_string(responseStr.size())},
            {"request_id"s, requestId}
        });

        Log::clearCorrelationId();

        guard.clearBusy();

        if (ec || lastRequest)
            break;
    }
}
