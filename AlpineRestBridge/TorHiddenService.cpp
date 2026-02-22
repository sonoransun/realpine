/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <TorHiddenService.h>
#include <TcpConnector.h>
#include <TcpTransport.h>
#include <Log.h>

#include <Platform.h>
#include <cstring>


TorHiddenService::~TorHiddenService ()
{
    shutdown();
}



bool
TorHiddenService::initialize (ushort localTargetPort,
                              ushort torControlPort,
                              ushort onionVirtualPort,
                              const string & controlAuth)
{
    TcpConnector connector;
    connector.setDestination(htonl(INADDR_LOOPBACK), htons(torControlPort));

    if (!connector.connect(controlConn_)) {
        Log::Error("TorHiddenService: Failed to connect to Tor control port "s +
                   std::to_string(torControlPort));
        return false;
    }

    if (!authenticate(controlAuth)) {
        Log::Error("TorHiddenService: Authentication with Tor control port failed.");
        delete controlConn_;
        controlConn_ = nullptr;
        return false;
    }

    if (!createHiddenService(onionVirtualPort, localTargetPort)) {
        Log::Error("TorHiddenService: Failed to create hidden service.");
        delete controlConn_;
        controlConn_ = nullptr;
        return false;
    }

    active_ = true;

    Log::Info("TorHiddenService: Created hidden service at "s + onionAddress_);
    return true;
}



void
TorHiddenService::shutdown ()
{
    if (!active_)
        return;

    if (controlConn_ && !serviceId_.empty()) {
        string response;
        sendCommand("DEL_ONION " + serviceId_ + "\r\n", response);
        Log::Info("TorHiddenService: Removed hidden service "s + serviceId_);
    }

    active_ = false;
    serviceId_.clear();
    onionAddress_.clear();

    delete controlConn_;
    controlConn_ = nullptr;
}



const string &
TorHiddenService::onionAddress () const
{
    return onionAddress_;
}



bool
TorHiddenService::isActive () const
{
    return active_;
}



bool
TorHiddenService::authenticate (const string & auth)
{
    string command;

    if (auth.empty())
        command = "AUTHENTICATE\r\n";
    else
        command = "AUTHENTICATE \"" + auth + "\"\r\n";

    string response;
    if (!sendCommand(command, response))
        return false;

    return response.find("250 OK") != string::npos;
}



bool
TorHiddenService::createHiddenService (ushort virtualPort, ushort targetPort)
{
    string command = "ADD_ONION NEW:BEST Port="s +
                     std::to_string(virtualPort) + ",127.0.0.1:" +
                     std::to_string(targetPort) + "\r\n";

    string response;
    if (!sendCommand(command, response))
        return false;

    // Parse ServiceID from response lines like "250-ServiceID=xxxxx"
    auto pos = response.find("ServiceID=");
    if (pos == string::npos) {
        Log::Error("TorHiddenService: No ServiceID in ADD_ONION response.");
        return false;
    }

    pos += 10;  // skip "ServiceID="
    auto endPos = response.find_first_of("\r\n", pos);
    if (endPos == string::npos)
        endPos = response.size();

    serviceId_ = response.substr(pos, endPos - pos);
    onionAddress_ = serviceId_ + ".onion";

    return true;
}



bool
TorHiddenService::sendCommand (const string & command, string & response)
{
    if (!controlConn_)
        return false;

    if (!controlConn_->send(reinterpret_cast<const byte *>(command.data()),
                            command.size())) {
        Log::Error("TorHiddenService: Failed to send command to control port.");
        return false;
    }

    return readResponse(response);
}



bool
TorHiddenService::readResponse (string & response)
{
    response.clear();

    byte buffer[4096];
    ulong bytesRead = 0;

    // Read until we get a line starting with "250 " (final response line)
    // or an error code (5xx, 4xx)
    while (true) {
        if (!controlConn_->receive(buffer, sizeof(buffer) - 1, bytesRead))
            return false;

        if (bytesRead == 0)
            return false;

        response.append(reinterpret_cast<char *>(buffer), bytesRead);

        // Check if we've received the final response line.
        // Final lines start with "250 " (space, not hyphen) or error codes.
        auto lastNewline = response.rfind('\n');
        string lastLine;

        if (lastNewline != string::npos && lastNewline > 0) {
            // Find start of last complete line
            auto prevNewline = response.rfind('\n', lastNewline - 1);
            if (prevNewline == string::npos)
                lastLine = response.substr(0, lastNewline);
            else
                lastLine = response.substr(prevNewline + 1,
                                           lastNewline - prevNewline - 1);
        } else {
            lastLine = response;
        }

        // Remove trailing \r
        if (!lastLine.empty() && lastLine.back() == '\r')
            lastLine.pop_back();

        // "250 " means final success line; 5xx/4xx means error
        if (lastLine.starts_with("250 ") ||
            lastLine.starts_with("5") ||
            lastLine.starts_with("4"))
            break;
    }

    return true;
}
