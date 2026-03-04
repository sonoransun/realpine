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

} // anonymous namespace



WebSocketSession::WebSocketSession (asio::ip::tcp::socket socket)
    : socket_(std::move(socket))
{
}


WebSocketSession::~WebSocketSession ()
{
    unsubscribeFromEvents();
}



void
WebSocketSession::start ()
{
    open_ = true;
    subscribeToEvents();
    readLoop();
}



void
WebSocketSession::sendText (const string & message)
{
    if (!open_)
        return;
    sendFrame(t_Opcode::Text, message);
}



void
WebSocketSession::close ()
{
    if (!open_)
        return;
    open_ = false;
    unsubscribeFromEvents();

    sendFrame(t_Opcode::Close, ""s);

    asio::error_code ec;
    socket_.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
    socket_.close(ec);
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



void
WebSocketSession::readLoop ()
{
    while (open_) {
        t_Frame frame;
        if (!readFrame(frame)) {
            open_ = false;
            break;
        }

        switch (frame.opcode) {
            case t_Opcode::Text:
                onMessage(frame.payload);
                break;
            case t_Opcode::Ping:
                sendPong(frame.payload);
                break;
            case t_Opcode::Close:
                open_ = false;
                sendFrame(t_Opcode::Close, ""s);
                break;
            case t_Opcode::Pong:
                break;
            default:
                break;
        }
    }

    unsubscribeFromEvents();

    asio::error_code ec;
    socket_.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
    socket_.close(ec);
}



bool
WebSocketSession::readFrame (t_Frame & frame)
{
    asio::error_code ec;
    byte header[2];

    auto n = asio::read(socket_, asio::buffer(header, 2), ec);
    if (ec || n < 2)
        return false;

    frame.fin    = (header[0] & 0x80) != 0;
    frame.opcode = static_cast<t_Opcode>(header[0] & 0x0F);
    frame.masked = (header[1] & 0x80) != 0;

    uint64_t payloadLen = header[1] & 0x7F;

    if (payloadLen == 126) {
        byte ext[2];
        n = asio::read(socket_, asio::buffer(ext, 2), ec);
        if (ec || n < 2) return false;
        payloadLen = (uint64_t(ext[0]) << 8) | uint64_t(ext[1]);
    } else if (payloadLen == 127) {
        byte ext[8];
        n = asio::read(socket_, asio::buffer(ext, 8), ec);
        if (ec || n < 8) return false;
        payloadLen = 0;
        for (int i = 0; i < 8; ++i)
            payloadLen = (payloadLen << 8) | uint64_t(ext[i]);
    }

    frame.payloadLength = payloadLen;

    if (frame.masked) {
        n = asio::read(socket_, asio::buffer(frame.maskKey, 4), ec);
        if (ec || n < 4) return false;
    }

    if (payloadLen > 0) {
        vector<byte> data(payloadLen);
        n = asio::read(socket_, asio::buffer(data.data(), payloadLen), ec);
        if (ec || n < payloadLen) return false;

        if (frame.masked) {
            for (uint64_t i = 0; i < payloadLen; ++i)
                data[i] ^= frame.maskKey[i % 4];
        }

        frame.payload.assign(reinterpret_cast<const char *>(data.data()), payloadLen);
    }

    return true;
}



void
WebSocketSession::sendFrame (t_Opcode       opcode,
                             const string & payload)
{
    std::lock_guard<std::mutex> lock(writeMutex_);

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

    asio::error_code ec;
    asio::write(socket_, asio::buffer(frame.data(), frame.size()), ec);

    if (ec) {
        Log::Error("WebSocketSession: write error: "s + ec.message());
        open_ = false;
    }
}



void
WebSocketSession::sendPong (const string & payload)
{
    sendFrame(t_Opcode::Pong, payload);
}



void
WebSocketSession::onMessage (const string & message)
{
    Log::Debug("WebSocketSession received: "s + message);
}



void
WebSocketSession::subscribeToEvents ()
{
    auto self = shared_from_this();

    auto sub1 = EventBus::subscribe(t_Event::PeerDiscovered,
        [self](t_Event, const string & data) {
            self->sendText("{\"event\":\"PeerDiscovered\",\"data\":\""s + data + "\"}"s);
        });

    auto sub2 = EventBus::subscribe(t_Event::PeerDisconnected,
        [self](t_Event, const string & data) {
            self->sendText("{\"event\":\"PeerDisconnected\",\"data\":\""s + data + "\"}"s);
        });

    auto sub3 = EventBus::subscribe(t_Event::QueryCompleted,
        [self](t_Event, const string & data) {
            self->sendText("{\"event\":\"QueryCompleted\",\"data\":\""s + data + "\"}"s);
        });

    auto sub4 = EventBus::subscribe(t_Event::QueryProgress,
        [self](t_Event, const string & data) {
            self->sendText("{\"event\":\"QueryProgress\",\"data\":\""s + data + "\"}"s);
        });

    auto sub5 = EventBus::subscribe(t_Event::GroupChanged,
        [self](t_Event, const string & data) {
            self->sendText("{\"event\":\"GroupChanged\",\"data\":\""s + data + "\"}"s);
        });

    subscriptions_ = {sub1, sub2, sub3, sub4, sub5};
}



void
WebSocketSession::unsubscribeFromEvents ()
{
    for (auto id : subscriptions_)
        EventBus::unsubscribe(id);
    subscriptions_.clear();
}
