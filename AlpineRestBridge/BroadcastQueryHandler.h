/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <AlpineStackInterface.h>
#include <Common.h>
#include <SysThread.h>
#include <UdpConnection.h>


class BroadcastQueryHandler : public SysThread
{
  public:
    BroadcastQueryHandler() = default;
    ~BroadcastQueryHandler();

    bool initialize(ushort listenPort);

    void threadMain();


  private:
    void handleMessage(const string & message, ulong senderIp, ushort senderPort);
    void handleQuery(const string & message, ulong senderIp, ushort senderPort);
    void handleCancel(const string & message);
    void sendResponse(ulong queryId,
                      ulong destIp,
                      ushort destPort,
                      const AlpineStackInterface::t_PeerResourcesIndex & results);

    static constexpr int RECV_TIMEOUT_MS = 1000;
    static constexpr int QUERY_POLL_MS = 100;
    static constexpr int QUERY_TIMEOUT_MS = 15000;
    static constexpr int RECV_BUFFER_SIZE = 4096;

    UdpConnection udpSocket_;
    ushort listenPort_{0};
    string responderId_;
};
