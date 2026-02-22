/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <TorTunnel.h>
#include <TcpConnector.h>
#include <TcpTransport.h>
#include <JsonReader.h>
#include <JsonWriter.h>
#include <Log.h>

#include <Platform.h>
#include <functional>
#include <cstring>
#include <ctime>


TorTunnel::~TorTunnel ()
{
    stop();
    connections_.clear();
    acceptor_.close();
}



bool
TorTunnel::initialize (ushort listenPort, ushort socksPort,
                       const string & peerList,
                       const string & onionAddress, ushort restPort)
{
    listenPort_   = listenPort;
    socksPort_    = socksPort;
    onionAddress_ = onionAddress;
    restPort_     = restPort;

    // Parse comma-separated peer list
    if (!peerList.empty()) {
        string remaining = peerList;

        while (!remaining.empty()) {
            auto commaPos = remaining.find(',');
            string peer;

            if (commaPos != string::npos) {
                peer = remaining.substr(0, commaPos);
                remaining = remaining.substr(commaPos + 1);
            } else {
                peer = remaining;
                remaining.clear();
            }

            // Trim whitespace
            auto start = peer.find_first_not_of(" \t");
            auto end   = peer.find_last_not_of(" \t");

            if (start != string::npos && end != string::npos) {
                peer = peer.substr(start, end - start + 1);
                configuredPeers_.push_back(peer);
            }
        }
    }

    // Bind acceptor to localhost only
    if (!acceptor_.create(htonl(INADDR_LOOPBACK), htons(listenPort_))) {
        Log::Error("TorTunnel: Failed to create TCP acceptor on port "s +
                   std::to_string(listenPort_));
        return false;
    }

    acceptor_.nonBlocking();

    // Generate responder ID
    char hostname[256] = {};
    gethostname(hostname, sizeof(hostname) - 1);
    auto hashVal = std::hash<string>{}(hostname);
    responderId_ = "tor-bridge-"s + std::to_string(hashVal & 0xFFFFFF);

    Log::Info("TorTunnel: Listening on port "s + std::to_string(listenPort_) +
              " with " + std::to_string(configuredPeers_.size()) + " configured peers");
    return true;
}



void
TorTunnel::threadMain ()
{
    Log::Info("TorTunnel: Thread started.");

    connectToConfiguredPeers();

    time_t lastBeaconTime    = 0;
    time_t lastReconnectTime = time(nullptr);

    while (isActive()) {

        // Build pollfd array: acceptor + all connections
        std::vector<pollfd> fds;

        pollfd acceptorPfd;
        acceptorPfd.fd = acceptor_.getFd();
        acceptorPfd.events = POLLIN;
        acceptorPfd.revents = 0;
        fds.push_back(acceptorPfd);

        for (auto & pc : connections_) {
            pollfd pfd;
            pfd.fd = pc.transport->getFd();
            pfd.events = POLLIN;
            pfd.revents = 0;
            fds.push_back(pfd);
        }

        int result = alpine_poll(fds.data(), fds.size(), POLL_TIMEOUT_MS);

        if (result < 0) {
            if (alpine_socket_errno() == EINTR)
                continue;
            Log::Error("TorTunnel: poll() error.");
            break;
        }

        // Check acceptor
        if (fds[0].revents & POLLIN)
            acceptInboundConnection();

        // Check connections for incoming data
        for (size_t i = 0; i < connections_.size(); ++i) {
            if (fds[i + 1].revents & (POLLIN | POLLERR | POLLHUP)) {
                string json;
                if (recvFramedMessage(*connections_[i].transport, json))
                    handleMessage(connections_[i], json);
                else {
                    Log::Info("TorTunnel: Connection to "s +
                              connections_[i].onionAddress + " closed.");
                    connections_[i].transport.reset();
                }
            }
        }

        // Remove dead connections
        removeDeadConnections();

        // Periodic beacon
        time_t now = time(nullptr);

        if (now - lastBeaconTime >= BEACON_INTERVAL_SEC) {
            for (auto & pc : connections_) {
                if (pc.transport)
                    sendDiscoveryBeacon(pc);
            }
            lastBeaconTime = now;
        }

        // Periodic reconnect
        if (now - lastReconnectTime >= RECONNECT_INTERVAL_SEC) {
            connectToConfiguredPeers();
            lastReconnectTime = now;
        }
    }

    connections_.clear();
    Log::Info("TorTunnel: Thread exiting.");
}



void
TorTunnel::connectToConfiguredPeers ()
{
    for (const auto & peerAddr : configuredPeers_) {

        // Parse "address:port" format
        string addr = peerAddr;
        ushort port = 8090;  // default virtual port

        auto colonPos = peerAddr.rfind(':');
        if (colonPos != string::npos) {
            addr = peerAddr.substr(0, colonPos);
            port = static_cast<ushort>(atoi(peerAddr.substr(colonPos + 1).c_str()));
        }

        // Skip if already connected to this peer
        bool alreadyConnected = false;
        for (const auto & pc : connections_) {
            if (pc.transport && pc.onionAddress == addr) {
                alreadyConnected = true;
                break;
            }
        }

        if (alreadyConnected)
            continue;

        std::unique_ptr<TcpTransport> transport;

        if (connectViaSocks5(addr, port, transport)) {
            PeerConnection pc;
            pc.transport = std::move(transport);
            pc.onionAddress = addr;
            pc.outbound = true;
            connections_.push_back(std::move(pc));

            Log::Info("TorTunnel: Connected to "s + addr + ":" +
                      std::to_string(port) + " via SOCKS5");
        } else {
            Log::Debug("TorTunnel: Failed to connect to "s + addr + ":" +
                       std::to_string(port) + " via SOCKS5");
        }
    }
}



bool
TorTunnel::connectViaSocks5 (const string & onionAddr, ushort port,
                             std::unique_ptr<TcpTransport> & transport)
{
    // Connect to Tor SOCKS5 proxy
    TcpConnector connector;
    connector.setDestination(htonl(INADDR_LOOPBACK), htons(socksPort_));

    TcpTransport * rawTransport = nullptr;
    if (!connector.connect(rawTransport)) {
        Log::Debug("TorTunnel: Failed to connect to SOCKS5 proxy.");
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
    if (!transport->receive(greetResp, sizeof(greetResp), bytesRead) ||
        bytesRead != 2 || greetResp[0] != 0x05 || greetResp[1] != 0x00) {
        Log::Debug("TorTunnel: SOCKS5 greeting rejected.");
        transport.reset();
        return false;
    }

    // SOCKS5 connect request: CONNECT, DOMAINNAME type
    // [ver=5, cmd=1(connect), rsv=0, atyp=3(domain), len, domain..., port_hi, port_lo]
    byte hostLen = static_cast<byte>(onionAddr.size());

    std::vector<byte> connectReq;
    connectReq.push_back(0x05);      // version
    connectReq.push_back(0x01);      // CONNECT
    connectReq.push_back(0x00);      // reserved
    connectReq.push_back(0x03);      // DOMAINNAME
    connectReq.push_back(hostLen);   // domain length

    for (char c : onionAddr)
        connectReq.push_back(static_cast<byte>(c));

    connectReq.push_back(static_cast<byte>((port >> 8) & 0xFF));  // port high
    connectReq.push_back(static_cast<byte>(port & 0xFF));          // port low

    if (!transport->send(connectReq.data(), connectReq.size())) {
        transport.reset();
        return false;
    }

    // Read connect response (minimum 10 bytes for IPv4 bind addr)
    byte connectResp[10];
    bytesRead = 0;
    if (!transport->receive(connectResp, sizeof(connectResp), bytesRead) ||
        bytesRead < 4) {
        Log::Debug("TorTunnel: SOCKS5 connect response too short.");
        transport.reset();
        return false;
    }

    if (connectResp[1] != 0x00) {
        Log::Debug("TorTunnel: SOCKS5 connect failed, status="s +
                   std::to_string(static_cast<int>(connectResp[1])));
        transport.reset();
        return false;
    }

    // Connection established — transport is now tunneled through Tor
    return true;
}



void
TorTunnel::acceptInboundConnection ()
{
    std::unique_ptr<TcpTransport> transport;

    if (!acceptor_.accept(transport))
        return;

    PeerConnection pc;
    pc.transport = std::move(transport);
    pc.onionAddress = "inbound";
    pc.outbound = false;
    connections_.push_back(std::move(pc));

    Log::Info("TorTunnel: Accepted inbound connection.");
}



void
TorTunnel::removeDeadConnections ()
{
    auto it = connections_.begin();
    while (it != connections_.end()) {
        if (!it->transport)
            it = connections_.erase(it);
        else
            ++it;
    }
}



bool
TorTunnel::sendFramedMessage (TcpTransport & conn, const string & json)
{
    uint32_t payloadLen = static_cast<uint32_t>(json.size());

    if (payloadLen > MAX_MESSAGE_SIZE) {
        Log::Error("TorTunnel: Message too large to send.");
        return false;
    }

    // Send 4-byte big-endian length header
    uint32_t netLen = htonl(payloadLen);
    if (!conn.send(reinterpret_cast<const byte *>(&netLen), sizeof(netLen)))
        return false;

    // Send payload
    return conn.send(reinterpret_cast<const byte *>(json.data()), payloadLen);
}



bool
TorTunnel::recvFramedMessage (TcpTransport & conn, string & json)
{
    // Read 4-byte length header
    byte header[4];
    ulong bytesRead = 0;

    if (!conn.receive(header, sizeof(header), bytesRead) || bytesRead != 4)
        return false;

    uint32_t payloadLen = (static_cast<uint32_t>(header[0]) << 24) |
                          (static_cast<uint32_t>(header[1]) << 16) |
                          (static_cast<uint32_t>(header[2]) << 8)  |
                          (static_cast<uint32_t>(header[3]));

    if (payloadLen == 0 || payloadLen > MAX_MESSAGE_SIZE) {
        Log::Error("TorTunnel: Invalid message length: "s + std::to_string(payloadLen));
        return false;
    }

    // Read payload
    std::vector<byte> buffer(payloadLen);
    ulong totalRead = 0;

    while (totalRead < payloadLen) {
        ulong chunkRead = 0;
        if (!conn.receive(buffer.data() + totalRead,
                          payloadLen - totalRead, chunkRead))
            return false;

        if (chunkRead == 0)
            return false;

        totalRead += chunkRead;
    }

    json.assign(reinterpret_cast<char *>(buffer.data()), payloadLen);
    return true;
}



void
TorTunnel::handleMessage (PeerConnection & peer, const string & json)
{
    JsonReader reader(json);

    string type;
    if (!reader.getString("type", type)) {
        Log::Debug("TorTunnel: Received message with no type field.");
        return;
    }

    if (type == "alpine_query")
        handleQuery(peer, json);
    else if (type == "alpine_cancel")
        handleCancel(json);
    else if (type == "alpine_response")
        Log::Debug("TorTunnel: Received response from "s + peer.onionAddress);
    else if (type == "alpine_discovery") {
        // Update peer's onion address from beacon if it was inbound
        string peerOnion;
        if (reader.getString("onionAddress", peerOnion) && !peerOnion.empty())
            peer.onionAddress = peerOnion;
        Log::Debug("TorTunnel: Discovery beacon from "s + peer.onionAddress);
    } else {
        Log::Debug("TorTunnel: Unknown message type: " + type);
    }
}



void
TorTunnel::handleQuery (PeerConnection & peer, const string & json)
{
    JsonReader reader(json);

    ulong  queryId = 0;
    string senderId;
    string queryString;

    if (!reader.getUlong("queryId", queryId)) {
        Log::Debug("TorTunnel: Query missing queryId.");
        return;
    }

    if (!reader.getString("queryString", queryString)) {
        Log::Debug("TorTunnel: Query missing queryString.");
        return;
    }

    reader.getString("senderId", senderId);

    Log::Info("TorTunnel: Received query "s + std::to_string(queryId) +
              " from " + senderId + ": " + queryString);

    AlpineStackInterface::t_QueryOptions options;
    reader.getString("groupName", options.groupName);

    ulong autoHaltLimit = 0;
    if (reader.getUlong("autoHaltLimit", autoHaltLimit))
        options.autoHaltLimit = autoHaltLimit;
    else
        options.autoHaltLimit = 0;

    ulong peerDescMax = 0;
    if (reader.getUlong("peerDescMax", peerDescMax))
        options.peerDescMax = peerDescMax;
    else
        options.peerDescMax = 0;

    options.autoDownload = false;
    options.optionId = 0;

    ulong alpineQueryId = 0;

    if (!AlpineStackInterface::startQuery(options, queryString, alpineQueryId)) {
        Log::Error("TorTunnel: Failed to start query for request "s +
                   std::to_string(queryId));
        return;
    }

    // Poll for query completion
    int elapsed = 0;

    while (isActive() && elapsed < QUERY_TIMEOUT_MS) {
        if (!AlpineStackInterface::queryInProgress(alpineQueryId))
            break;
        usleep(QUERY_POLL_MS * 1000);
        elapsed += QUERY_POLL_MS;
    }

    AlpineStackInterface::t_PeerResourcesIndex results;
    AlpineStackInterface::getQueryResults(alpineQueryId, results);

    sendResponse(peer, queryId, results);

    AlpineStackInterface::cancelQuery(alpineQueryId);

    Log::Info("TorTunnel: Completed query "s + std::to_string(queryId) +
              " with " + std::to_string(results.size()) + " peers");
}



void
TorTunnel::handleCancel (const string & json)
{
    JsonReader reader(json);

    ulong queryId = 0;
    string senderId;

    reader.getUlong("queryId", queryId);
    reader.getString("senderId", senderId);

    Log::Info("TorTunnel: Cancel request for query "s +
              std::to_string(queryId) + " from " + senderId);
}



void
TorTunnel::sendResponse (PeerConnection & peer, ulong queryId,
                         const AlpineStackInterface::t_PeerResourcesIndex & results)
{
    JsonWriter writer;
    writer.beginObject();
    writer.key("type");        writer.value("alpine_response");
    writer.key("queryId");     writer.value(queryId);
    writer.key("responderId"); writer.value(responderId_);
    writer.key("resources");
    writer.beginArray();

    for (const auto & [peerId, peerResources] : results) {
        for (const auto & res : peerResources.resourceDescList) {
            writer.beginObject();
            writer.key("resourceId");   writer.value(res.resourceId);
            writer.key("size");         writer.value(res.size);
            writer.key("description");  writer.value(res.description);
            writer.key("locators");
            writer.beginArray();

            for (const auto & loc : res.locators)
                writer.value(loc);

            writer.endArray();
            writer.endObject();
        }
    }

    writer.endArray();
    writer.endObject();

    auto payload = writer.result();

    if (!sendFramedMessage(*peer.transport, payload))
        Log::Error("TorTunnel: Failed to send response to "s + peer.onionAddress);
    else
        Log::Debug("TorTunnel: Sent response to "s + peer.onionAddress +
                   " (" + std::to_string(payload.size()) + " bytes)");
}



void
TorTunnel::sendDiscoveryBeacon (PeerConnection & peer)
{
    JsonWriter writer;
    writer.beginObject();
    writer.key("type");           writer.value("alpine_discovery");
    writer.key("service");        writer.value("alpine-bridge");
    writer.key("version");        writer.value("1");
    writer.key("restPort");       writer.value(static_cast<ulong>(restPort_));
    writer.key("bridgeVersion");  writer.value("devel-00019");
    writer.key("onionAddress");   writer.value(onionAddress_);
    writer.endObject();

    auto payload = writer.result();

    if (!sendFramedMessage(*peer.transport, payload)) {
        Log::Debug("TorTunnel: Failed to send beacon to "s + peer.onionAddress);
        peer.transport.reset();
    }
}
