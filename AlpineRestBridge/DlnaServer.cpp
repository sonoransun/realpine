/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <DlnaHandler.h>
#include <DlnaServer.h>
#include <Log.h>
#include <MediaStreamer.h>

#include <cstring>
#include <thread>


static string
extractHeader(const string & request, const string & name)
{
    auto pos = request.find(name);

    if (pos == string::npos) {
        // Try lowercase
        string lower = name;
        for (auto & c : lower)
            c = toupper(c);
        pos = request.find(lower);
    }

    if (pos == string::npos)
        return "";

    auto valueStart = pos + name.length();
    auto lineEnd = request.find("\r\n", valueStart);
    string value = request.substr(valueStart, lineEnd - valueStart);

    auto firstNonSpace = value.find_first_not_of(' ');
    return (firstNonSpace != string::npos) ? value.substr(firstNonSpace) : value;
}


DlnaServer::DlnaServer(ContentStore & store)
    : store_(store),
      port_(0),
      transcodeEnabled_(true)
{}


DlnaServer::~DlnaServer()
{
    stop();
    acceptor_.close();
}


bool
DlnaServer::initialize(ushort port, const string & hostAddress, bool transcodeEnabled)
{
    port_ = port;
    hostAddress_ = hostAddress;
    transcodeEnabled_ = transcodeEnabled;

    deviceUuid_ = generateUuid(hostAddress_, port_);
    baseUrl_ = "http://"s + hostAddress_ + ":" + std::to_string(port_);

    if (!acceptor_.create(0, port_)) {
        Log::Error("DlnaServer: Failed to create acceptor on port "s + std::to_string(port_));
        return false;
    }

    Log::Info("DlnaServer: Initialized on port "s + std::to_string(port_));
    return true;
}


void
DlnaServer::threadMain()
{
    Log::Info("DlnaServer: Thread started.");

    while (isActive()) {
        std::unique_ptr<TcpTransport> transport;

        if (!acceptor_.accept(transport)) {
            if (isActive())
                Log::Error("DlnaServer: Accept failed.");
            continue;
        }

        std::thread connThread([this, t = std::move(transport)]() mutable { handleConnection(std::move(t)); });
        connThread.detach();
    }

    Log::Info("DlnaServer: Thread exiting.");
}


ushort
DlnaServer::getPort()
{
    return port_;
}


string
DlnaServer::getBaseUrl()
{
    return baseUrl_;
}


string
DlnaServer::getDeviceUuid()
{
    return deviceUuid_;
}


void
DlnaServer::handleConnection(std::unique_ptr<TcpTransport> transport)
{
    byte buffer[8192];
    ulong bytesRead = 0;

    if (!transport->receive(buffer, sizeof(buffer) - 1, bytesRead))
        return;

    buffer[bytesRead] = 0;
    string request((const char *)buffer, bytesRead);

    // Parse method and path from first line
    auto spacePos = request.find(' ');
    if (spacePos == string::npos)
        return;

    string method = request.substr(0, spacePos);
    auto pathEnd = request.find(' ', spacePos + 1);
    if (pathEnd == string::npos)
        return;

    string path = request.substr(spacePos + 1, pathEnd - spacePos - 1);

    Log::Debug("DlnaServer: "s + method + " " + path);

    if (method == "GET") {
        if (path == "/device.xml") {
            sendResponse(transport.get(),
                         200,
                         "text/xml; charset=\"utf-8\"",
                         DlnaHandler::deviceDescription(deviceUuid_, baseUrl_));
            return;
        }

        if (path == "/cds.xml") {
            sendResponse(transport.get(), 200, "text/xml; charset=\"utf-8\"", DlnaHandler::cdsServiceDescription());
            return;
        }

        if (path == "/cms.xml") {
            sendResponse(transport.get(), 200, "text/xml; charset=\"utf-8\"", DlnaHandler::cmsServiceDescription());
            return;
        }

        if (path.starts_with("/media/")) {
            string mediaId = path.substr(7);

            if (mediaId.contains("/transcode")) {
                mediaId = mediaId.substr(0, mediaId.find('/'));
                if (transcodeEnabled_)
                    MediaStreamer::serveTranscode(transport->getFd(), store_, mediaId);
                else
                    sendResponse(transport.get(), 404, "text/plain", "Transcoding disabled");
                return;
            }

            string rangeHeader = extractHeader(request, "Range:");
            MediaStreamer::serveFile(transport->getFd(), store_, mediaId, rangeHeader);
            return;
        }
    }

    if (method == "POST") {
        auto bodyStart = request.find("\r\n\r\n");
        string soapBody = (bodyStart != string::npos) ? request.substr(bodyStart + 4) : "";
        string soapAction = extractHeader(request, "SOAPAction:");

        if (path == "/control/cds") {
            sendResponse(transport.get(),
                         200,
                         "text/xml; charset=\"utf-8\"",
                         DlnaHandler::handleCdsAction(soapBody, soapAction, store_, baseUrl_, transcodeEnabled_));
            return;
        }

        if (path == "/control/cms") {
            sendResponse(transport.get(),
                         200,
                         "text/xml; charset=\"utf-8\"",
                         DlnaHandler::handleCmsAction(soapBody, soapAction));
            return;
        }
    }

    sendResponse(transport.get(), 404, "text/plain", "Not Found");
}


void
DlnaServer::sendResponse(TcpTransport * t, int status, const string & contentType, const string & body)
{
    const char * statusText = (status == 200) ? "OK" : (status == 404) ? "Not Found" : "Error";

    string response = "HTTP/1.1 "s + std::to_string(status) + " " + statusText +
                      "\r\n"
                      "Content-Type: " +
                      contentType +
                      "\r\n"
                      "Content-Length: " +
                      std::to_string(body.length()) +
                      "\r\n"
                      "Connection: close\r\n"
                      "\r\n" +
                      body;

    t->send((const byte *)response.c_str(), response.length());
}


string
DlnaServer::generateUuid(const string & hostname, ushort port)
{
    ulong hash = 5381;
    for (auto c : hostname)
        hash = hash * 33 + c;
    hash = hash * 33 + port;

    char buf[64];
    snprintf(buf,
             sizeof(buf),
             "uuid:alpine-%08x-%04x-%04x-%04x-%012llx",
             (uint)(hash & 0xFFFFFFFF),
             (uint)((hash >> 8) & 0xFFFF),
             (uint)((hash >> 16) & 0xFFFF),
             (uint)((hash >> 24) & 0xFFFF),
             (ulonglong)(hash & 0xFFFFFFFFFFFF));
    return buf;
}
