/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <HttpServer.h>
#include <HttpRequest.h>
#include <HttpResponse.h>
#include <Log.h>

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

        Log::Info("HttpServer started on port " + std::to_string(port));
        return true;
    } catch (const std::exception & e) {
        Log::Error("HttpServer::start failed: "s + e.what());
        return false;
    }
}


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


void
HttpServer::handleConnection (asio::ip::tcp::socket socket)
{
    try {
        socket.set_option(asio::socket_base::receive_buffer_size(65536));

        std::vector<byte> buffer(65536);
        asio::error_code ec;
        ulong totalRead = 0;

        // Read headers first
        auto bytesRead = socket.read_some(asio::buffer(buffer.data(), buffer.size()), ec);
        if (ec || bytesRead == 0) {
            --activeConnections_;
            return;
        }
        totalRead = bytesRead;

        // Parse the request
        HttpRequest request;
        if (!HttpRequest::parse(buffer.data(), totalRead, request)) {
            auto resp = HttpResponse::badRequest("Malformed HTTP request");
            auto respStr = resp.build();
            asio::write(socket, asio::buffer(respStr), ec);
            --activeConnections_;
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
                asio::write(socket, asio::buffer(respStr), ec);
                --activeConnections_;
                return;
            }

            while (request.body.length() < contentLength) {
                bytesRead = socket.read_some(asio::buffer(buffer.data(), buffer.size()), ec);
                if (ec || bytesRead == 0)
                    break;
                request.body.append(reinterpret_cast<const char *>(buffer.data()), bytesRead);
            }
        }

        // Dispatch
        auto response = router_.dispatch(request);
        auto responseStr = response.build();
        asio::write(socket, asio::buffer(responseStr), ec);

    } catch (const std::exception & e) {
        Log::Error("HttpServer::handleConnection exception: "s + e.what());
    }

    --activeConnections_;
}
