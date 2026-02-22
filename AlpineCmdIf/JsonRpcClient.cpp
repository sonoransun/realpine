/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <JsonRpcClient.h>
#include <TcpConnector.h>
#include <TcpTransport.h>
#include <NetUtils.h>
#include <Log.h>
#include <cstdlib>
#include <cstring>


string   JsonRpcClient::host_s;
ulong    JsonRpcClient::ipAddress_s  = 0;
ushort   JsonRpcClient::port_s       = 0;
ulong    JsonRpcClient::requestId_s  = 1;



bool
JsonRpcClient::initialize (const string &  host,
                           ushort          port)
{
    host_s = host;
    port_s = htons(port);

    if (!NetUtils::stringIpToLong(host, ipAddress_s))
    {
        Log::Error("JsonRpcClient: Invalid server address: "s + host);
        return false;
    }

    Log::Info("JsonRpcClient: Initialized for "s + host + ":" +
              std::to_string(port));
    return true;
}



bool
JsonRpcClient::call (const string &  method,
                     const string &  paramsJson,
                     string &        resultJson)
{
    // Build JSON-RPC 2.0 request body
    string idStr = std::to_string(requestId_s++);
    string body = "{\"jsonrpc\":\"2.0\",\"method\":\""s + method +
                  "\",\"params\":"s + paramsJson +
                  ",\"id\":"s + idStr + "}";

    // Build HTTP request
    string contentLength = std::to_string(body.length());
    string httpRequest = "POST /rpc HTTP/1.1\r\n"s +
                         "Host: "s + host_s + "\r\n" +
                         "Content-Type: application/json\r\n" +
                         "Content-Length: "s + contentLength + "\r\n" +
                         "Connection: close\r\n" +
                         "\r\n" +
                         body;

    // Connect
    TcpConnector connector;
    if (!connector.setDestination(ipAddress_s, port_s))
    {
        Log::Error("JsonRpcClient: Failed to set destination");
        return false;
    }

    TcpTransport * transport = nullptr;
    if (!connector.connect(transport))
    {
        Log::Error("JsonRpcClient: Failed to connect to server");
        return false;
    }

    // Send request
    if (!transport->send((const byte *)httpRequest.c_str(), httpRequest.length()))
    {
        Log::Error("JsonRpcClient: Failed to send request");
        delete transport;
        return false;
    }

    // Read response
    string responseData;
    byte buffer[8192];

    while (true)
    {
        ulong bytesRead = 0;
        if (!transport->receive(buffer, sizeof(buffer), bytesRead))
            break;
        if (bytesRead == 0)
            break;

        responseData.append((const char *)buffer, bytesRead);
    }

    delete transport;

    if (responseData.empty())
    {
        Log::Error("JsonRpcClient: Empty response from server");
        return false;
    }

    // Find body after HTTP headers (blank line = \r\n\r\n)
    ulong bodyStart = responseData.find("\r\n\r\n");
    if (bodyStart == string::npos)
    {
        Log::Error("JsonRpcClient: Malformed HTTP response");
        return false;
    }
    bodyStart += 4;

    string jsonBody = responseData.substr(bodyStart);

    if (jsonBody.empty())
    {
        Log::Error("JsonRpcClient: Empty JSON body in response");
        return false;
    }

    // Check for JSON-RPC error
    if (jsonBody.find("\"error\"") != string::npos &&
        jsonBody.find("\"result\"") == string::npos)
    {
        Log::Error("JsonRpcClient: Server returned error: "s + jsonBody);
        resultJson = jsonBody;
        return false;
    }

    // Extract the result value — find "result": and take everything between
    // the colon and the last ,\"id\" or }
    ulong resultPos = jsonBody.find("\"result\":");
    if (resultPos == string::npos)
    {
        Log::Error("JsonRpcClient: No result in response");
        resultJson = jsonBody;
        return false;
    }

    // Result starts after "result":
    ulong valueStart = resultPos + 9;

    // Find the ,\"id\" that follows the result value
    // We need to find the matching end — the result could be an object or value
    // Find ",\"id\":" which marks the end of the result
    ulong idMarker = jsonBody.rfind(",\"id\":");
    if (idMarker != string::npos && idMarker > valueStart)
    {
        resultJson = jsonBody.substr(valueStart, idMarker - valueStart);
    }
    else
    {
        // Fallback — take everything between "result": and the last }
        ulong lastBrace = jsonBody.rfind('}');
        if (lastBrace != string::npos && lastBrace > valueStart)
            resultJson = jsonBody.substr(valueStart, lastBrace - valueStart);
        else
            resultJson = jsonBody.substr(valueStart);
    }

    return true;
}
