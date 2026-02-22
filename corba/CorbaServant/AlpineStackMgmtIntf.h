/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <AlpineCorbaClient.h>
#include <CorbaUtils.h>
#include <string>


class AlpineStackMgmtIntf
{
  public:


    // Supported interface operations
    //
    static bool  setTotalTransferLimit (ulong limit);

    static bool  getTotalTransferLimit (ulong & limit);

    static bool  setPeerTransferLimit (ulong limit);

    static bool  getPeerTransferLimit (ulong & limit);


  private:

};

