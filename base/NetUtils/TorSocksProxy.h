/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <memory>

class TcpTransport;


class TorSocksProxy
{
  public:
    static bool
    connect(ushort socksPort, const string & targetHost, ushort targetPort, std::unique_ptr<TcpTransport> & transport);
};
