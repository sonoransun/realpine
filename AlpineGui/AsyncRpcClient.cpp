/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AsyncRpcClient.h>
#include <NetUtils.h>
#include <TcpConnector.h>
#include <TcpTransport.h>
#include <cstdlib>
#include <cstring>


bool
AsyncRpcClient::connect(const string & host, ushort port)
{
    host_ = host;
    port_ = htons(port);

    if (!NetUtils::stringIpToLong(host, ipAddress_))
        return false;

    // Verify connectivity with a test connection
    TcpConnector connector;
    if (!connector.setDestination(ipAddress_, port_))
        return false;

    TcpTransport * transport = nullptr;
    if (!connector.connect(transport))
        return false;

    delete transport;
    connected_ = true;
    return true;
}


void
AsyncRpcClient::disconnect()
{
    connected_ = false;
    ipAddress_ = 0;
    port_ = 0;
}


bool
AsyncRpcClient::isConnected() const
{
    return connected_;
}


bool
AsyncRpcClient::call(const string & method, const string & paramsJson, string & resultJson)
{
    if (!connected_)
        return false;

    // Build JSON-RPC 2.0 request body
    string idStr = std::to_string(requestId_++);
    string body =
        "{\"jsonrpc\":\"2.0\",\"method\":\""s + method + "\",\"params\":"s + paramsJson + ",\"id\":"s + idStr + "}";

    // Build HTTP request
    string contentLength = std::to_string(body.length());
    string httpRequest = "POST /rpc HTTP/1.1\r\n"s + "Host: "s + host_ + "\r\n" + "Content-Type: application/json\r\n" +
                         "Content-Length: "s + contentLength + "\r\n" + "Connection: close\r\n" + "\r\n" + body;

    // Connect
    TcpConnector connector;
    if (!connector.setDestination(ipAddress_, port_))
        return false;

    TcpTransport * transport = nullptr;
    if (!connector.connect(transport))
        return false;

    // Send request
    if (!transport->send((const byte *)httpRequest.c_str(), httpRequest.length())) {
        delete transport;
        return false;
    }

    // Read response
    string responseData;
    byte buffer[8192];

    while (true) {
        ulong bytesRead = 0;
        if (!transport->receive(buffer, sizeof(buffer), bytesRead))
            break;
        if (bytesRead == 0)
            break;

        responseData.append((const char *)buffer, bytesRead);
    }

    delete transport;

    if (responseData.empty())
        return false;

    // Find body after HTTP headers (blank line = \r\n\r\n)
    ulong bodyStart = responseData.find("\r\n\r\n");
    if (bodyStart == string::npos)
        return false;

    bodyStart += 4;

    string jsonBody = responseData.substr(bodyStart);

    if (jsonBody.empty())
        return false;

    // Check for JSON-RPC error
    if (jsonBody.find("\"error\"") != string::npos && jsonBody.find("\"result\"") == string::npos) {
        resultJson = jsonBody;
        return false;
    }

    // Extract the result value
    ulong resultPos = jsonBody.find("\"result\":");
    if (resultPos == string::npos) {
        resultJson = jsonBody;
        return false;
    }

    ulong valueStart = resultPos + 9;

    // Find ",\"id\":" which marks the end of the result
    ulong idMarker = jsonBody.rfind(",\"id\":");
    if (idMarker != string::npos && idMarker > valueStart) {
        resultJson = jsonBody.substr(valueStart, idMarker - valueStart);
    } else {
        ulong lastBrace = jsonBody.rfind('}');
        if (lastBrace != string::npos && lastBrace > valueStart)
            resultJson = jsonBody.substr(valueStart, lastBrace - valueStart);
        else
            resultJson = jsonBody.substr(valueStart);
    }

    return true;
}
