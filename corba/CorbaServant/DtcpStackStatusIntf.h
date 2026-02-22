/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <AlpineCorbaClient.h>
#include <CorbaUtils.h>
#include <string>


class DtcpStackStatusIntf
{
  public:


    // Public types
    //
    struct t_DtcpStackBufferStats {
        ulong  numSendBuffers;
        ulong  peakSendBuffersUsed;
        ulong  numReceiveBuffers;
        ulong  peakReceiveBuffersUsed;
        ulong  numThreadBuffers;
    };


    // Supported interface operations
    //
    static bool  getBufferStats (t_DtcpStackBufferStats & stats);


  private:

};

