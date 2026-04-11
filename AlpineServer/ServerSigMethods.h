/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <AlpineDtcpConnAcceptor.h>
#include <AlpineDtcpConnTransport.h>
#include <AlpineDtcpUdpTransport.h>
#include <ApplCore.h>
#include <Common.h>
#include <DtcpBaseConnMux.h>
#include <DtcpPacket.h>
#include <Log.h>
#include <StringUtils.h>


class ServerSigMethods
{
  public:
    ServerSigMethods() = default;
    ~ServerSigMethods() = default;


    static void initialize();

    static void sendWelcomeMessage();

    static void sendHelloMessage();

  private:
    static void sendStringMessages(const string & message, bool reliable);
};
