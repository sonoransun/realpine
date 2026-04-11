/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <Log.h>
#include <TcpConnector.h>
#include <TcpTransport.h>
#include <TorSocksProxy.h>

#include <Platform.h>
#include <vector>


bool
TorSocksProxy::connect(ushort socksPort,
                       const string & targetHost,
                       ushort targetPort,
                       std::unique_ptr<TcpTransport> & transport)
{
    // Connect to Tor SOCKS5 proxy
    TcpConnector connector;
    connector.setDestination(htonl(INADDR_LOOPBACK), htons(socksPort));

    TcpTransport * rawTransport = nullptr;
    if (!connector.connect(rawTransport)) {
        Log::Debug("TorSocksProxy: Failed to connect to SOCKS5 proxy.");
        return false;
    }

    transport.reset(rawTransport);

    // SOCKS5 greeting: version 5, 1 auth method, no-auth
    byte greeting[] = {0x05, 0x01, 0x00};
    if (!transport->send(greeting, sizeof(greeting))) {
        transport.reset();
        return false;
    }

    // Read greeting response
    byte greetResp[2];
    ulong bytesRead = 0;
    if (!transport->receive(greetResp, sizeof(greetResp), bytesRead) || bytesRead != 2 || greetResp[0] != 0x05 ||
        greetResp[1] != 0x00) {
        Log::Debug("TorSocksProxy: SOCKS5 greeting rejected.");
        transport.reset();
        return false;
    }

    // SOCKS5 connect request: CONNECT, DOMAINNAME type
    // [ver=5, cmd=1(connect), rsv=0, atyp=3(domain), len, domain..., port_hi, port_lo]
    byte hostLen = static_cast<byte>(targetHost.size());

    std::vector<byte> connectReq;
    connectReq.push_back(0x05);     // version
    connectReq.push_back(0x01);     // CONNECT
    connectReq.push_back(0x00);     // reserved
    connectReq.push_back(0x03);     // DOMAINNAME
    connectReq.push_back(hostLen);  // domain length

    for (char c : targetHost)
        connectReq.push_back(static_cast<byte>(c));

    connectReq.push_back(static_cast<byte>((targetPort >> 8) & 0xFF));  // port high
    connectReq.push_back(static_cast<byte>(targetPort & 0xFF));         // port low

    if (!transport->send(connectReq.data(), connectReq.size())) {
        transport.reset();
        return false;
    }

    // Read connect response (minimum 10 bytes for IPv4 bind addr)
    byte connectResp[10];
    bytesRead = 0;
    if (!transport->receive(connectResp, sizeof(connectResp), bytesRead) || bytesRead < 4) {
        Log::Debug("TorSocksProxy: SOCKS5 connect response too short.");
        transport.reset();
        return false;
    }

    if (connectResp[1] != 0x00) {
        Log::Debug("TorSocksProxy: SOCKS5 connect failed, status="s + std::to_string(static_cast<int>(connectResp[1])));
        transport.reset();
        return false;
    }

    // Connection established — transport is now tunneled through Tor
    return true;
}
