/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AlpineBroadcast.h>
#include <AlpineQuery.h>
#include <DtcpBroadcastSet.h>
#include <Log.h>
#include <StringUtils.h>


AlpineBroadcast::AlpineBroadcast(DtcpBroadcastSet * destinations)
    : DtcpBroadcast(destinations)
{
#ifdef _VERBOSE
    Log::Debug("AlpineBroadcast constructor invoked.");
#endif

    query_ = nullptr;
}


AlpineBroadcast::~AlpineBroadcast()
{
#ifdef _VERBOSE
    Log::Debug("AlpineBroadcast destructor invoked.");
#endif
}


bool
AlpineBroadcast::handlePacketSend(ulong transportId)
{
#ifdef _VERBOSE
    Log::Debug("AlpineBroadcast::handlePacketSend invoked.  Transport ID: "s + std::to_string(transportId));
#endif

    if (query_) {
        query_->packetSent(transportId);
    }

    return true;
}


bool
AlpineBroadcast::handleSendComplete(ulong numSent, struct timeval & duration)
{
#ifdef _VERBOSE
    Log::Debug("AlpineBroadcast destructor invoked.  Total sent: "s + std::to_string(numSent));
#endif

    if (query_) {
        query_->broadcastComplete(numSent, duration);
    }

    return true;
}


void
AlpineBroadcast::setQueryParent(AlpineQuery * query)
{
#ifdef _VERBOSE
    Log::Debug("AlpineBroadcast::setQueryParent invoked.");
#endif

    query_ = query;
}
