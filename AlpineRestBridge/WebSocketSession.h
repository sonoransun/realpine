/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <EventBus.h>
#include <asio.hpp>
#include <functional>
#include <mutex>
#include <vector>
#include <memory>


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

    void  readLoop ();

    bool  readFrame (t_Frame & frame);

    void  sendFrame (t_Opcode       opcode,
                     const string & payload);

    void  sendPong (const string & payload);

    void  onMessage (const string & message);

    void  subscribeToEvents ();

    void  unsubscribeFromEvents ();


    asio::ip::tcp::socket              socket_;
    std::mutex                         writeMutex_;
    bool                               open_{false};
    vector<EventBus::t_SubscriberId>   subscriptions_;

};
