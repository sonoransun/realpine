/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <AlpineCorbaClient.h>
#include <CorbaUtils.h>
#include <string>


class DtcpStackMgmtIntf
{
  public:


    // Supported interface operations
    //
    static bool  natDiscovery (bool isRequired);

    static bool  natDiscoveryRequired ();

    static bool  setDataSendingLimit (ulong limit);

    static bool  getDataSendingLimit (ulong & limit);

    static bool  setStackThreadLimit (ulong limit);

    static bool  getStackThreadLimit (ulong & limit);

    static bool  setReceiveBufferLimit (ulong limit);

    static bool  getReceiveBufferLimit (ulong & limit);

    static bool  setSendBufferLimit (ulong limit);

    static bool  getSendBufferLimit (ulong & limit);


  private:

};

