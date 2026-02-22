/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <DtcpStackStatusIntf.h>
#include <Log.h>
#include <StringUtils.h>



bool  
DtcpStackStatusIntf::getBufferStats (t_DtcpStackBufferStats & stats)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpStackStatusIntf::getBufferStats invoked.");
#endif

    ExNewEnv;

    Alpine::t_DtcpStackBufferStats  corbaStats;

    ExTry {

        AlpineCorbaClient::DtcpStackStatus::getBufferStats (corbaStats, ExTryEnv);

        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {
        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " from AlpineCorbaClient::DtcpStackStatus::getBufferStats in call to "
                             "DtcpStackStatusIntf::getBufferStats.");
        return false;
    }
    ExCatchAny {
        Log::Error ("Caught unknown exception from "
                             "AlpineCorbaClient::DtcpStackStatus::getBufferStats in call to "
                             "DtcpStackStatusIntf::getBufferStats.");
        return false;
    }
    ExEndTry;
    ExCheck;

    stats.numSendBuffers          = corbaStats.numSendBuffers;
    stats.peakSendBuffersUsed     = corbaStats.peakSendBuffersUsed;
    stats.numReceiveBuffers       = corbaStats.numReceiveBuffers;
    stats.peakReceiveBuffersUsed  = corbaStats.peakReceiveBuffersUsed;
    stats.numThreadBuffers        = corbaStats.numThreadBuffers;


    return true;
}



