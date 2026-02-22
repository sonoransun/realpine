/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <AlpineCorbaClient.h>
#include <CorbaUtils.h>
#include <string>


class AlpineStackStatusIntf
{
  public:


    // Public types
    //
    struct t_AlpineStackTransferStats {
        ulong  totalTransfers;
        ulong  peakTransfers;
    };


    // Supported interface operations
    //
    static bool  getTransferStats (t_AlpineStackTransferStats & stats);


  private:

};

