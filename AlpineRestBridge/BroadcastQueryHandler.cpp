/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <BroadcastQueryHandler.h>
#include <JsonReader.h>
#include <JsonWriter.h>
#include <Log.h>
#include <NetUtils.h>

#include <Platform.h>
#include <functional>


BroadcastQueryHandler::~BroadcastQueryHandler()
{
    stop();
    udpSocket_.close();
}


bool
BroadcastQueryHandler::initialize(ushort listenPort)
{
    listenPort_ = listenPort;

    if (!udpSocket_.create(0, htons(listenPort_))) {
        Log::Error("BroadcastQueryHandler: Failed to create UDP socket.");
        return false;
    }

    // SO_REUSEADDR + SO_REUSEPORT are applied inside UdpConnection::create()
    // before bind(), so multiple workers can listen on this port. We re-apply
    // SO_REUSEPORT here as a defensive check; on Linux it remains set on the
    // already-bound socket (no-op) but documents the intent.
    //
    int reuseAddr = 1;
    if (setsockopt(udpSocket_.getFd(), SOL_SOCKET, SO_REUSEADDR, &reuseAddr, sizeof(reuseAddr)) < 0) {
        Log::Error("BroadcastQueryHandler: Failed to set SO_REUSEADDR.");
        udpSocket_.close();
        return false;
    }
#ifdef SO_REUSEPORT
    if (setsockopt(udpSocket_.getFd(), SOL_SOCKET, SO_REUSEPORT, &reuseAddr, sizeof(reuseAddr)) < 0) {
        Log::Error("BroadcastQueryHandler: Failed to set SO_REUSEPORT.");
        udpSocket_.close();
        return false;
    }
#endif

    char hostname[256] = {};
    gethostname(hostname, sizeof(hostname) - 1);
    auto hashVal = std::hash<string>{}(hostname);
    responderId_ = "bridge-"s + std::to_string(hashVal & 0xFFFFFF);

    Log::Info("BroadcastQueryHandler: Initialized on port "s + std::to_string(listenPort_) +
              " responderId=" + responderId_);
    return true;
}


void
BroadcastQueryHandler::threadMain()
{
    Log::Info("BroadcastQueryHandler: Thread started.");

    byte recvBuffer[RECV_BUFFER_SIZE];

    pollfd pfd;
    pfd.fd = udpSocket_.getFd();
    pfd.events = POLLIN;

    while (isActive()) {

        if (alpine_poll(&pfd, 1, RECV_TIMEOUT_MS) <= 0)
            continue;

        ulong sourceIp = 0;
        ushort sourcePort = 0;
        uint dataLength = 0;

        if (!udpSocket_.receiveData(recvBuffer, RECV_BUFFER_SIZE - 1, sourceIp, sourcePort, dataLength))
            continue;

        if (dataLength == 0)
            continue;

        string message(reinterpret_cast<char *>(recvBuffer), dataLength);
        handleMessage(message, sourceIp, sourcePort);
    }

    Log::Info("BroadcastQueryHandler: Thread exiting.");
}


void
BroadcastQueryHandler::handleMessage(const string & message, ulong senderIp, ushort senderPort)
{
    JsonReader reader(message);

    string type;
    if (!reader.getString("type", type)) {
        Log::Debug("BroadcastQueryHandler: Received message with no type field.");
        return;
    }

    if (type == "alpine_query")
        handleQuery(message, senderIp, senderPort);
    else if (type == "alpine_cancel")
        handleCancel(message);
    else
        Log::Debug("BroadcastQueryHandler: Unknown message type: " + type);
}


void
BroadcastQueryHandler::handleQuery(const string & message, ulong senderIp, ushort senderPort)
{
    JsonReader reader(message);

    ulong queryId = 0;
    string senderId;
    string senderAddress;
    ulong senderPortJson = 0;
    string queryString;

    if (!reader.getUlong("queryId", queryId)) {
        Log::Debug("BroadcastQueryHandler: Query missing queryId.");
        return;
    }

    if (!reader.getString("queryString", queryString)) {
        Log::Debug("BroadcastQueryHandler: Query missing queryString.");
        return;
    }

    reader.getString("senderId", senderId);
    reader.getString("senderAddress", senderAddress);
    reader.getUlong("senderPort", senderPortJson);

    Log::Info("BroadcastQueryHandler: Received query "s + std::to_string(queryId) + " from " + senderId + ": " +
              queryString);

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
        Log::Error("BroadcastQueryHandler: Failed to start query for broadcast request "s + std::to_string(queryId));
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

    // Use sender address/port from JSON payload (Android protocol convention)
    ulong destIp = senderIp;
    ushort destPort = senderPort;

    if (!senderAddress.empty()) {
        ulong parsedIp = 0;
        if (NetUtils::stringIpToLong(senderAddress, parsedIp))
            destIp = parsedIp;
    }

    if (senderPortJson > 0)
        destPort = htons(static_cast<ushort>(senderPortJson));

    sendResponse(queryId, destIp, destPort, results);

    AlpineStackInterface::cancelQuery(alpineQueryId);

    Log::Info("BroadcastQueryHandler: Completed query "s + std::to_string(queryId) + " with " +
              std::to_string(results.size()) + " peers");
}


void
BroadcastQueryHandler::handleCancel(const string & message)
{
    JsonReader reader(message);

    ulong queryId = 0;
    string senderId;

    reader.getUlong("queryId", queryId);
    reader.getString("senderId", senderId);

    Log::Info("BroadcastQueryHandler: Cancel request for query "s + std::to_string(queryId) + " from " + senderId);
}


void
BroadcastQueryHandler::sendResponse(ulong queryId,
                                    ulong destIp,
                                    ushort destPort,
                                    const AlpineStackInterface::t_PeerResourcesIndex & results)
{
    JsonWriter writer;
    writer.beginObject();
    writer.key("type");
    writer.value("alpine_response");
    writer.key("queryId");
    writer.value(queryId);
    writer.key("responderId");
    writer.value(responderId_);
    writer.key("resources");
    writer.beginArray();

    for (const auto & [peerId, peerResources] : results) {
        for (const auto & res : peerResources.resourceDescList) {
            writer.beginObject();
            writer.key("resourceId");
            writer.value(res.resourceId);
            writer.key("size");
            writer.value(res.size);
            writer.key("description");
            writer.value(res.description);
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

    string destIpStr;
    NetUtils::longIpToString(destIp, destIpStr);

    if (!udpSocket_.sendData(destIp, destPort, reinterpret_cast<const byte *>(payload.data()), payload.size())) {
        Log::Error("BroadcastQueryHandler: Failed to send response to "s + destIpStr + ":" +
                   std::to_string(ntohs(destPort)));
    } else {
        Log::Debug("BroadcastQueryHandler: Sent response to "s + destIpStr + ":" + std::to_string(ntohs(destPort)) +
                   " (" + std::to_string(payload.size()) + " bytes)");
    }
}
