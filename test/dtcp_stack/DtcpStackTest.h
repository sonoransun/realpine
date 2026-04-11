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


class DtcpStackTest
{
  public:
    DtcpStackTest() = default;
    ~DtcpStackTest() = default;


    static void initialize();

    static void sendWelcomeMessage();

    static void sendHelloMessage();

    static void sendStringMessages(const string & message);


  private:
};
