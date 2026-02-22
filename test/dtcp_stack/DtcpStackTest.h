/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <Log.h>
#include <StringUtils.h>
#include <ApplCore.h>
#include <AlpineDtcpUdpTransport.h>
#include <DtcpPacket.h>
#include <DtcpBaseConnMux.h>
#include <AlpineDtcpConnAcceptor.h>
#include <AlpineDtcpConnTransport.h>


class DtcpStackTest 
{
  public:

    DtcpStackTest () = default;
    ~DtcpStackTest () = default;


    static void  initialize ();

    static void  sendWelcomeMessage ();

    static void  sendHelloMessage ();

    static void  sendStringMessages (const string &  message);


  private:

};

