/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <vector>

class TcpTransport;


class TorService
{
  public:
    TorService() = default;
    ~TorService();

    bool initialize(ushort torControlPort, const string & controlAuth);

    bool addPortMapping(ushort virtualPort, ushort localTargetPort);

    bool createService();

    void shutdown();

    const string & onionAddress() const;
    bool isActive() const;


  private:
    struct PortMapping
    {
        ushort virtualPort;
        ushort targetPort;
    };

    bool authenticate(const string & auth);
    bool sendCommand(const string & command, string & response);
    bool readResponse(string & response);

    TcpTransport * controlConn_{nullptr};
    string serviceId_;
    string onionAddress_;
    bool active_{false};
    std::vector<PortMapping> pendingMappings_;
};
