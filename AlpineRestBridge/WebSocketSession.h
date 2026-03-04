/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <EventBus.h>
#include <asio.hpp>
#include <functional>
#include <mutex>
#include <vector>
#include <memory>
#include <deque>
#include <chrono>


class WebSocketSession : public std::enable_shared_from_this<WebSocketSession>
{
  public:

    explicit WebSocketSession (asio::ip::tcp::socket socket);
    ~WebSocketSession ();

    void  start ();

    void  sendText (const string & message);

    void  close ();

    bool  isOpen () const;


    static bool  isWebSocketUpgrade (const std::unordered_map<string, string> & headers);

    static string  buildHandshakeResponse (const string & secWebSocketKey);


    // --- Shared buffer pool ---

    static constexpr size_t  BUFFER_POOL_SIZE   = 32;
    static constexpr size_t  BUFFER_SIZE         = 65536;  // 64KB

    using t_Buffer = std::shared_ptr<std::array<byte, BUFFER_SIZE>>;

    static t_Buffer  borrowBuffer ();
    static void      returnBuffer (t_Buffer buf);


  private:

    enum class t_Opcode : byte {
        Continuation = 0x0,
        Text         = 0x1,
        Binary       = 0x2,
        Close        = 0x8,
        Ping         = 0x9,
        Pong         = 0xA
    };

    struct t_Frame {
        bool       fin;
        t_Opcode   opcode;
        bool       masked;
        uint64_t   payloadLength;
        byte       maskKey[4];
        string     payload;
    };


    // --- Async read chain ---

    void  asyncReadHeader ();
    void  onHeaderRead (const asio::error_code & ec, size_t bytesRead);
    void  asyncReadExtendedLength (uint64_t initialLen);
    void  asyncReadMaskAndPayload ();
    void  asyncReadPayload ();
    void  onFrameComplete ();

    // --- Async write ---

    void  doSendFrame (t_Opcode       opcode,
                       const string & payload);

    void  flushWriteQueue ();

    void  sendPong (const string & payload);

    void  onMessage (const string & message);

    // --- Event subscription with batching ---

    void  subscribeToEvents ();
    void  unsubscribeFromEvents ();
    void  queueEvent (const string & eventJson);
    void  flushEventBatch ();

    // --- Ping/pong keepalive ---

    void  startPingTimer ();
    void  onPingTimer (const asio::error_code & ec);
    void  startPongDeadline ();
    void  onPongTimeout (const asio::error_code & ec);

    // --- Shutdown ---

    void  doClose ();


    asio::ip::tcp::socket                  socket_;
    asio::strand<asio::ip::tcp::socket::executor_type>  strand_;
    bool                                   open_{false};
    vector<EventBus::t_SubscriberId>       subscriptions_;

    // Current frame being read
    byte       headerBuf_[2]{};
    byte       extLenBuf_[8]{};
    byte       maskBuf_[4]{};
    t_Frame    currentFrame_{};
    t_Buffer   readBuffer_;

    // Write queue (serialized via strand)
    std::deque<vector<byte>>               writeQueue_;
    bool                                   writing_{false};

    // Event batching
    std::deque<string>                     pendingEvents_;
    asio::steady_timer                     batchTimer_;
    bool                                   batchTimerActive_{false};

    // Ping/pong keepalive
    asio::steady_timer                     pingTimer_;
    asio::steady_timer                     pongDeadlineTimer_;
    bool                                   awaitingPong_{false};


    // --- Static buffer pool ---

    static std::mutex                            bufferPoolMutex_s;
    static std::deque<t_Buffer>                  bufferPool_s;
    static bool                                  bufferPoolInitialized_s;

    static void  initBufferPool ();

};
