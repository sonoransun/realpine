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


#include <AlpineBroadcast.h>
#include <AlpineQuery.h>
#include <DtcpBroadcastSet.h>
#include <Log.h>
#include <StringUtils.h>



AlpineBroadcast::AlpineBroadcast (DtcpBroadcastSet *  destinations)
    :   DtcpBroadcast(destinations)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineBroadcast constructor invoked.");
#endif

    query_ = nullptr;
}



AlpineBroadcast::~AlpineBroadcast ()
{
#ifdef _VERBOSE
    Log::Debug ("AlpineBroadcast destructor invoked.");
#endif
}



bool  
AlpineBroadcast::handlePacketSend (ulong  transportId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineBroadcast::handlePacketSend invoked.  Transport ID: "s +
                std::to_string (transportId));
#endif

    if (query_) {
        query_->packetSent (transportId);
    }

    return true;
}



bool  
AlpineBroadcast::handleSendComplete (ulong             numSent,
                                     struct timeval &  duration)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineBroadcast destructor invoked.  Total sent: "s +
                std::to_string (numSent));
#endif

    if (query_) {
        query_->broadcastComplete (numSent, duration);
    }

    return true;
}



void  
AlpineBroadcast::setQueryParent (AlpineQuery *  query)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineBroadcast::setQueryParent invoked.");
#endif

    query_ = query;
}



