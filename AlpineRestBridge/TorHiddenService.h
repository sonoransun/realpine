/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>

class TcpTransport;


class TorHiddenService
{
  public:

    TorHiddenService () = default;
    ~TorHiddenService ();

    bool initialize (ushort localTargetPort,
                     ushort torControlPort,
                     ushort onionVirtualPort,
                     const string & controlAuth);
    void shutdown ();

    const string & onionAddress () const;
    bool isActive () const;


  private:

    bool authenticate (const string & auth);
    bool createHiddenService (ushort virtualPort, ushort targetPort);
    bool sendCommand (const string & command, string & response);
    bool readResponse (string & response);

    TcpTransport * controlConn_{nullptr};
    string         serviceId_;
    string         onionAddress_;
    bool           active_{false};
};
