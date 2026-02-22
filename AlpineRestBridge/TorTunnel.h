/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <SysThread.h>
#include <TcpAcceptor.h>
#include <AlpineStackInterface.h>
#include <memory>
#include <vector>

class TcpTransport;


class TorTunnel : public SysThread
{
  public:

    TorTunnel () = default;
    ~TorTunnel ();

    bool initialize (ushort listenPort, ushort socksPort,
                     const string & peerList,
                     const string & onionAddress, ushort restPort);

    void threadMain ();


  private:

    struct PeerConnection {
        std::unique_ptr<TcpTransport> transport;
        string  onionAddress;
        bool    outbound;
    };

    void connectToConfiguredPeers ();
    bool connectViaSocks5 (const string & onionAddr, ushort port,
                           std::unique_ptr<TcpTransport> & transport);
    void acceptInboundConnection ();
    void removeDeadConnections ();

    bool sendFramedMessage (TcpTransport & conn, const string & json);
    bool recvFramedMessage (TcpTransport & conn, string & json);

    void handleMessage (PeerConnection & peer, const string & json);
    void handleQuery (PeerConnection & peer, const string & json);
    void handleCancel (const string & json);
    void sendDiscoveryBeacon (PeerConnection & peer);
    void sendResponse (PeerConnection & peer, ulong queryId,
                       const AlpineStackInterface::t_PeerResourcesIndex & results);

    static constexpr int POLL_TIMEOUT_MS        = 1000;
    static constexpr int QUERY_POLL_MS          = 100;
    static constexpr int QUERY_TIMEOUT_MS       = 15000;
    static constexpr int BEACON_INTERVAL_SEC    = 30;
    static constexpr int RECONNECT_INTERVAL_SEC = 60;
    static constexpr int MAX_MESSAGE_SIZE       = 65536;

    TcpAcceptor                    acceptor_;
    ushort                         listenPort_{0};
    ushort                         socksPort_{0};
    ushort                         restPort_{0};
    string                         onionAddress_;
    std::vector<string>            configuredPeers_;
    std::vector<PeerConnection>    connections_;
    string                         responderId_;
};
