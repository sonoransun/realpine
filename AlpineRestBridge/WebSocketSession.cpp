/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <WebSocketSession.h>
#include <Log.h>
#include <array>
#include <cstring>

// SHA-1 for WebSocket handshake
#include <sstream>
#include <iomanip>
#include <cstdint>


// Minimal SHA-1 implementation for WebSocket handshake accept key
namespace {

struct Sha1 {
    uint32_t state[5]{0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0};
    uint64_t count{0};
    byte buffer[64]{};

    static uint32_t rol(uint32_t v, uint32_t bits) { return (v << bits) | (v >> (32 - bits)); }

    void transform(const byte * data) {
        uint32_t w[80];
        for (int i = 0; i < 16; ++i)
            w[i] = (uint32_t(data[i*4]) << 24) | (uint32_t(data[i*4+1]) << 16)
                  | (uint32_t(data[i*4+2]) << 8) | uint32_t(data[i*4+3]);
        for (int i = 16; i < 80; ++i)
            w[i] = rol(w[i-3] ^ w[i-8] ^ w[i-14] ^ w[i-16], 1);

        uint32_t a = state[0], b = state[1], c = state[2], d = state[3], e = state[4];
        for (int i = 0; i < 80; ++i) {
            uint32_t f, k;
            if (i < 20)      { f = (b & c) | (~b & d);       k = 0x5A827999; }
            else if (i < 40) { f = b ^ c ^ d;                 k = 0x6ED9EBA1; }
            else if (i < 60) { f = (b & c) | (b & d) | (c & d); k = 0x8F1BBCDC; }
            else              { f = b ^ c ^ d;                 k = 0xCA62C1D6; }
            uint32_t tmp = rol(a, 5) + f + e + k + w[i];
            e = d; d = c; c = rol(b, 30); b = a; a = tmp;
        }
        state[0] += a; state[1] += b; state[2] += c; state[3] += d; state[4] += e;
    }

    void update(const byte * data, ulong len) {
        ulong idx = count % 64;
        count += len;
        for (ulong i = 0; i < len; ++i) {
            buffer[idx++] = data[i];
            if (idx == 64) { transform(buffer); idx = 0; }
        }
    }

    void update(const string & s) { update(reinterpret_cast<const byte *>(s.data()), s.size()); }

    std::array<byte, 20> digest() {
        uint64_t bits = count * 8;
        byte pad = 0x80;
        update(&pad, 1);
        pad = 0;
        while (count % 64 != 56) update(&pad, 1);
        for (int i = 7; i >= 0; --i) { byte b = byte(bits >> (i * 8)); update(&b, 1); }
        std::array<byte, 20> result{};
        for (int i = 0; i < 5; ++i) {
            result[i*4]   = byte(state[i] >> 24);
            result[i*4+1] = byte(state[i] >> 16);
            result[i*4+2] = byte(state[i] >> 8);
            result[i*4+3] = byte(state[i]);
        }
        return result;
    }
};

static const char base64Chars[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

string base64Encode(const byte * data, ulong len) {
    string result;
    result.reserve(((len + 2) / 3) * 4);
    for (ulong i = 0; i < len; i += 3) {
        uint32_t n = uint32_t(data[i]) << 16;
        if (i + 1 < len) n |= uint32_t(data[i+1]) << 8;
        if (i + 2 < len) n |= uint32_t(data[i+2]);
        result += base64Chars[(n >> 18) & 0x3F];
        result += base64Chars[(n >> 12) & 0x3F];
        result += (i + 1 < len) ? base64Chars[(n >> 6) & 0x3F] : '=';
        result += (i + 2 < len) ? base64Chars[n & 0x3F] : '=';
    }
    return result;
}

string eventName(t_Event event) {
    switch (event) {
        case t_Event::PeerDiscovered:   return "PeerDiscovered"s;
        case t_Event::PeerDisconnected: return "PeerDisconnected"s;
        case t_Event::QueryCompleted:   return "QueryCompleted"s;
        case t_Event::QueryProgress:    return "QueryProgress"s;
        case t_Event::GroupChanged:     return "GroupChanged"s;
    }
    return "Unknown"s;
}

} // anonymous namespace


// --- Static buffer pool ---

std::mutex                                    WebSocketSession::bufferPoolMutex_s;
std::deque<WebSocketSession::t_Buffer>        WebSocketSession::bufferPool_s;
bool                                          WebSocketSession::bufferPoolInitialized_s{false};


void
WebSocketSession::initBufferPool ()
{
    if (bufferPoolInitialized_s)
        return;
    for (size_t i = 0; i < BUFFER_POOL_SIZE; ++i)
        bufferPool_s.push_back(std::make_shared<std::array<byte, BUFFER_SIZE>>());
    bufferPoolInitialized_s = true;
}


WebSocketSession::t_Buffer
WebSocketSession::borrowBuffer ()
{
    std::lock_guard lock(bufferPoolMutex_s);
    initBufferPool();
    if (bufferPool_s.empty())
        return std::make_shared<std::array<byte, BUFFER_SIZE>>();
    auto buf = std::move(bufferPool_s.front());
    bufferPool_s.pop_front();
    return buf;
}


void
WebSocketSession::returnBuffer (t_Buffer buf)
{
    std::lock_guard lock(bufferPoolMutex_s);
    if (bufferPool_s.size() < BUFFER_POOL_SIZE)
        bufferPool_s.push_back(std::move(buf));
}



WebSocketSession::WebSocketSession (asio::ip::tcp::socket socket)
    : socket_(std::move(socket))
    , strand_(asio::make_strand(socket_.get_executor()))
    , batchTimer_(strand_)
    , pingTimer_(strand_)
    , pongDeadlineTimer_(strand_)
{
}


WebSocketSession::~WebSocketSession ()
{
    unsubscribeFromEvents();
    if (readBuffer_) {
        returnBuffer(std::move(readBuffer_));
        readBuffer_ = nullptr;
    }
}



void
WebSocketSession::start ()
{
    open_ = true;
    subscribeToEvents();
    startPingTimer();
    asyncReadHeader();
}



void
WebSocketSession::sendText (const string & message)
{
    if (!open_)
        return;

    auto self = shared_from_this();
    asio::post(strand_, [self, message]() {
        self->doSendFrame(t_Opcode::Text, message);
    });
}



void
WebSocketSession::close ()
{
    if (!open_)
        return;

    auto self = shared_from_this();
    asio::post(strand_, [self]() {
        self->doClose();
    });
}



bool
WebSocketSession::isOpen () const
{
    return open_;
}



bool
WebSocketSession::isWebSocketUpgrade (const std::unordered_map<string, string> & headers)
{
    auto upgradeIt = headers.find("upgrade");
    if (upgradeIt == headers.end())
        return false;

    // Check for "websocket" case-insensitively
    string val = upgradeIt->second;
    for (auto & c : val)
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

    return val.contains("websocket");
}



string
WebSocketSession::buildHandshakeResponse (const string & secWebSocketKey)
{
    static const string magic = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"s;

    Sha1 sha;
    sha.update(secWebSocketKey);
    sha.update(magic);
    auto hash = sha.digest();

    string accept = base64Encode(hash.data(), hash.size());

    return "HTTP/1.1 101 Switching Protocols\r\n"
           "Upgrade: websocket\r\n"
           "Connection: Upgrade\r\n"
           "Sec-WebSocket-Accept: "s + accept + "\r\n\r\n"s;
}



// --- Async read chain ---

void
WebSocketSession::asyncReadHeader ()
{
    if (!open_)
        return;

    auto self = shared_from_this();
    asio::async_read(socket_,
        asio::buffer(headerBuf_, 2),
        asio::bind_executor(strand_,
            [self](const asio::error_code & ec, size_t bytesRead) {
                self->onHeaderRead(ec, bytesRead);
            }));
}



void
WebSocketSession::onHeaderRead (const asio::error_code & ec, size_t bytesRead)
{
    if (ec || bytesRead < 2) {
        if (open_) {
            open_ = false;
            unsubscribeFromEvents();
            pingTimer_.cancel();
            pongDeadlineTimer_.cancel();
            asio::error_code closeEc;
            socket_.shutdown(asio::ip::tcp::socket::shutdown_both, closeEc);
            socket_.close(closeEc);
        }
        return;
    }

    currentFrame_ = {};
    currentFrame_.fin    = (headerBuf_[0] & 0x80) != 0;
    currentFrame_.opcode = static_cast<t_Opcode>(headerBuf_[0] & 0x0F);
    currentFrame_.masked = (headerBuf_[1] & 0x80) != 0;

    uint64_t initialLen = headerBuf_[1] & 0x7F;

    if (initialLen == 126 || initialLen == 127) {
        asyncReadExtendedLength(initialLen);
    } else {
        currentFrame_.payloadLength = initialLen;
        asyncReadMaskAndPayload();
    }
}



void
WebSocketSession::asyncReadExtendedLength (uint64_t initialLen)
{
    size_t bytesToRead = (initialLen == 126) ? 2 : 8;

    auto self = shared_from_this();
    asio::async_read(socket_,
        asio::buffer(extLenBuf_, bytesToRead),
        asio::bind_executor(strand_,
            [self, initialLen, bytesToRead](const asio::error_code & ec, size_t n) {
                if (ec || n < bytesToRead) {
                    self->open_ = false;
                    self->unsubscribeFromEvents();
                    self->pingTimer_.cancel();
                    self->pongDeadlineTimer_.cancel();
                    asio::error_code closeEc;
                    self->socket_.shutdown(asio::ip::tcp::socket::shutdown_both, closeEc);
                    self->socket_.close(closeEc);
                    return;
                }

                if (initialLen == 126) {
                    self->currentFrame_.payloadLength =
                        (uint64_t(self->extLenBuf_[0]) << 8) |
                         uint64_t(self->extLenBuf_[1]);
                } else {
                    self->currentFrame_.payloadLength = 0;
                    for (int i = 0; i < 8; ++i)
                        self->currentFrame_.payloadLength =
                            (self->currentFrame_.payloadLength << 8) |
                             uint64_t(self->extLenBuf_[i]);
                }

                self->asyncReadMaskAndPayload();
            }));
}



void
WebSocketSession::asyncReadMaskAndPayload ()
{
    if (currentFrame_.masked) {
        auto self = shared_from_this();
        asio::async_read(socket_,
            asio::buffer(maskBuf_, 4),
            asio::bind_executor(strand_,
                [self](const asio::error_code & ec, size_t n) {
                    if (ec || n < 4) {
                        self->open_ = false;
                        self->unsubscribeFromEvents();
                        self->pingTimer_.cancel();
                        self->pongDeadlineTimer_.cancel();
                        asio::error_code closeEc;
                        self->socket_.shutdown(asio::ip::tcp::socket::shutdown_both, closeEc);
                        self->socket_.close(closeEc);
                        return;
                    }
                    std::memcpy(self->currentFrame_.maskKey, self->maskBuf_, 4);
                    self->asyncReadPayload();
                }));
    } else {
        asyncReadPayload();
    }
}



void
WebSocketSession::asyncReadPayload ()
{
    auto payloadLen = currentFrame_.payloadLength;

    if (payloadLen == 0) {
        currentFrame_.payload.clear();
        onFrameComplete();
        return;
    }

    // Borrow a buffer from the pool for payloads that fit; allocate otherwise
    if (payloadLen <= BUFFER_SIZE) {
        readBuffer_ = borrowBuffer();
        auto self = shared_from_this();
        asio::async_read(socket_,
            asio::buffer(readBuffer_->data(), payloadLen),
            asio::bind_executor(strand_,
                [self, payloadLen](const asio::error_code & ec, size_t n) {
                    if (ec || n < payloadLen) {
                        if (self->readBuffer_) {
                            returnBuffer(std::move(self->readBuffer_));
                            self->readBuffer_ = nullptr;
                        }
                        self->open_ = false;
                        self->unsubscribeFromEvents();
                        self->pingTimer_.cancel();
                        self->pongDeadlineTimer_.cancel();
                        asio::error_code closeEc;
                        self->socket_.shutdown(asio::ip::tcp::socket::shutdown_both, closeEc);
                        self->socket_.close(closeEc);
                        return;
                    }

                    auto * data = self->readBuffer_->data();
                    if (self->currentFrame_.masked) {
                        for (uint64_t i = 0; i < payloadLen; ++i)
                            data[i] ^= self->currentFrame_.maskKey[i % 4];
                    }
                    self->currentFrame_.payload.assign(
                        reinterpret_cast<const char *>(data), payloadLen);

                    returnBuffer(std::move(self->readBuffer_));
                    self->readBuffer_ = nullptr;

                    self->onFrameComplete();
                }));
    } else {
        // Large payload — allocate directly
        auto largeBuf = std::make_shared<vector<byte>>(payloadLen);
        auto self = shared_from_this();
        asio::async_read(socket_,
            asio::buffer(largeBuf->data(), payloadLen),
            asio::bind_executor(strand_,
                [self, largeBuf, payloadLen](const asio::error_code & ec, size_t n) {
                    if (ec || n < payloadLen) {
                        self->open_ = false;
                        self->unsubscribeFromEvents();
                        self->pingTimer_.cancel();
                        self->pongDeadlineTimer_.cancel();
                        asio::error_code closeEc;
                        self->socket_.shutdown(asio::ip::tcp::socket::shutdown_both, closeEc);
                        self->socket_.close(closeEc);
                        return;
                    }

                    if (self->currentFrame_.masked) {
                        for (uint64_t i = 0; i < payloadLen; ++i)
                            (*largeBuf)[i] ^= self->currentFrame_.maskKey[i % 4];
                    }
                    self->currentFrame_.payload.assign(
                        reinterpret_cast<const char *>(largeBuf->data()), payloadLen);

                    self->onFrameComplete();
                }));
    }
}



void
WebSocketSession::onFrameComplete ()
{
    switch (currentFrame_.opcode) {
        case t_Opcode::Text:
            onMessage(currentFrame_.payload);
            break;
        case t_Opcode::Ping:
            sendPong(currentFrame_.payload);
            break;
        case t_Opcode::Close:
            open_ = false;
            doSendFrame(t_Opcode::Close, ""s);
            unsubscribeFromEvents();
            pingTimer_.cancel();
            pongDeadlineTimer_.cancel();
            {
                asio::error_code ec;
                socket_.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
                socket_.close(ec);
            }
            return;
        case t_Opcode::Pong:
            awaitingPong_ = false;
            pongDeadlineTimer_.cancel();
            break;
        default:
            break;
    }

    // Post next read
    asyncReadHeader();
}



// --- Async write ---

void
WebSocketSession::doSendFrame (t_Opcode       opcode,
                                const string & payload)
{
    // Must be called on the strand
    vector<byte> frame;
    frame.push_back(0x80 | static_cast<byte>(opcode));  // FIN + opcode

    auto len = payload.size();
    if (len < 126) {
        frame.push_back(static_cast<byte>(len));
    } else if (len <= 0xFFFF) {
        frame.push_back(126);
        frame.push_back(static_cast<byte>((len >> 8) & 0xFF));
        frame.push_back(static_cast<byte>(len & 0xFF));
    } else {
        frame.push_back(127);
        for (int i = 7; i >= 0; --i)
            frame.push_back(static_cast<byte>((len >> (i * 8)) & 0xFF));
    }

    frame.insert(frame.end(),
                 reinterpret_cast<const byte *>(payload.data()),
                 reinterpret_cast<const byte *>(payload.data() + payload.size()));

    writeQueue_.push_back(std::move(frame));

    if (!writing_)
        flushWriteQueue();
}



void
WebSocketSession::flushWriteQueue ()
{
    if (writeQueue_.empty()) {
        writing_ = false;
        return;
    }

    writing_ = true;
    auto self = shared_from_this();
    asio::async_write(socket_,
        asio::buffer(writeQueue_.front().data(), writeQueue_.front().size()),
        asio::bind_executor(strand_,
            [self](const asio::error_code & ec, size_t) {
                if (ec) {
                    Log::Error("WebSocketSession: write error: "s + ec.message());
                    self->open_ = false;
                    self->writeQueue_.clear();
                    self->writing_ = false;
                    return;
                }

                self->writeQueue_.pop_front();
                self->flushWriteQueue();
            }));
}



void
WebSocketSession::sendPong (const string & payload)
{
    doSendFrame(t_Opcode::Pong, payload);
}



void
WebSocketSession::onMessage (const string & message)
{
    Log::Debug("WebSocketSession received: "s + message);
}



// --- Event subscription with batching ---

void
WebSocketSession::subscribeToEvents ()
{
    auto self = shared_from_this();

    auto makeHandler = [self](t_Event event) {
        return [self, event](t_Event, const string & data) {
            auto json = "{\"event\":\""s + eventName(event) +
                        "\",\"data\":\""s + data + "\"}"s;
            self->queueEvent(std::move(json));
        };
    };

    auto sub1 = EventBus::subscribe(t_Event::PeerDiscovered,   makeHandler(t_Event::PeerDiscovered));
    auto sub2 = EventBus::subscribe(t_Event::PeerDisconnected, makeHandler(t_Event::PeerDisconnected));
    auto sub3 = EventBus::subscribe(t_Event::QueryCompleted,   makeHandler(t_Event::QueryCompleted));
    auto sub4 = EventBus::subscribe(t_Event::QueryProgress,    makeHandler(t_Event::QueryProgress));
    auto sub5 = EventBus::subscribe(t_Event::GroupChanged,     makeHandler(t_Event::GroupChanged));

    subscriptions_ = {sub1, sub2, sub3, sub4, sub5};
}



void
WebSocketSession::unsubscribeFromEvents ()
{
    for (auto id : subscriptions_)
        EventBus::unsubscribe(id);
    subscriptions_.clear();
}



void
WebSocketSession::queueEvent (const string & eventJson)
{
    auto self = shared_from_this();
    asio::post(strand_, [self, eventJson]() {
        self->pendingEvents_.push_back(eventJson);

        if (!self->batchTimerActive_) {
            self->batchTimerActive_ = true;
            self->batchTimer_.expires_after(std::chrono::milliseconds(50));
            self->batchTimer_.async_wait(
                [self](const asio::error_code & ec) {
                    self->batchTimerActive_ = false;
                    if (!ec)
                        self->flushEventBatch();
                });
        }
    });
}



void
WebSocketSession::flushEventBatch ()
{
    if (pendingEvents_.empty() || !open_)
        return;

    if (pendingEvents_.size() == 1) {
        doSendFrame(t_Opcode::Text, pendingEvents_.front());
        pendingEvents_.clear();
        return;
    }

    // Batch into JSON array
    string batch = "["s;
    bool first = true;
    for (const auto & ev : pendingEvents_) {
        if (!first)
            batch += ","s;
        batch += ev;
        first = false;
    }
    batch += "]"s;
    pendingEvents_.clear();

    doSendFrame(t_Opcode::Text, batch);
}



// --- Ping/pong keepalive ---

void
WebSocketSession::startPingTimer ()
{
    pingTimer_.expires_after(std::chrono::seconds(30));
    auto self = shared_from_this();
    pingTimer_.async_wait(
        asio::bind_executor(strand_,
            [self](const asio::error_code & ec) {
                self->onPingTimer(ec);
            }));
}



void
WebSocketSession::onPingTimer (const asio::error_code & ec)
{
    if (ec || !open_)
        return;

    doSendFrame(t_Opcode::Ping, ""s);
    awaitingPong_ = true;
    startPongDeadline();
    startPingTimer();
}



void
WebSocketSession::startPongDeadline ()
{
    pongDeadlineTimer_.expires_after(std::chrono::seconds(10));
    auto self = shared_from_this();
    pongDeadlineTimer_.async_wait(
        asio::bind_executor(strand_,
            [self](const asio::error_code & ec) {
                self->onPongTimeout(ec);
            }));
}



void
WebSocketSession::onPongTimeout (const asio::error_code & ec)
{
    if (ec || !open_)
        return;

    if (awaitingPong_) {
        Log::Info("WebSocketSession: pong timeout, closing session"s);
        doClose();
    }
}



void
WebSocketSession::doClose ()
{
    if (!open_)
        return;

    open_ = false;
    unsubscribeFromEvents();
    pingTimer_.cancel();
    pongDeadlineTimer_.cancel();
    batchTimer_.cancel();

    doSendFrame(t_Opcode::Close, ""s);

    asio::error_code ec;
    socket_.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
    socket_.close(ec);
}
