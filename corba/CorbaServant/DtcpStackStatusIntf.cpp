///////
///
///  Copyright (C) 2026  sonoransun
///
///  Permission is hereby granted, free of charge, to any person obtaining a copy
///  of this software and associated documentation files (the "Software"), to deal
///  in the Software without restriction, including without limitation the rights
///  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
///  copies of the Software, and to permit persons to whom the Software is
///  furnished to do so, subject to the following conditions:
///
///  The above copyright notice and this permission notice shall be included in all
///  copies or substantial portions of the Software.
///
///  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
///  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
///  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
///  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
///  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
///  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
///  SOFTWARE.
///
///////


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



