/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AlpinePeerProfile.h>
#include <Log.h>
#include <StringUtils.h>


AlpinePeerProfile::AlpinePeerProfile()
{
#ifdef _VERBOSE
    Log::Debug("AlpinePeerProfile constructor invoked.");
#endif

    peerId_ = 0;
    quality_ = 0;
    queries_ = 0;
    responses_ = 0;
}


AlpinePeerProfile::AlpinePeerProfile(const AlpinePeerProfile & copy)
{
#ifdef _VERBOSE
    Log::Debug("AlpinePeerProfile copy constructor invoked.");
#endif

    peerId_ = copy.peerId_;
    quality_ = copy.quality_;
    queries_ = copy.queries_;
    responses_ = copy.responses_;
}


AlpinePeerProfile::AlpinePeerProfile(ulong peerId)
{
#ifdef _VERBOSE
    Log::Debug("AlpinePeerProfile peer ID constructor invoked.");
#endif

    peerId_ = peerId;
    quality_ = 0;
    queries_ = 0;
    responses_ = 0;
}


AlpinePeerProfile::~AlpinePeerProfile()
{
#ifdef _VERBOSE
    Log::Debug("AlpinePeerProfile destructor invoked.");
#endif
}


AlpinePeerProfile &
AlpinePeerProfile::operator=(const AlpinePeerProfile & copy)
{
#ifdef _VERBOSE
    Log::Debug("AlpinePeerProfile assignment invoked.");
#endif

    if (&copy == this) {
        return *this;
    }

    peerId_ = copy.peerId_;
    quality_ = copy.quality_;
    queries_ = copy.queries_;
    responses_ = copy.responses_;


    return *this;
}


void
AlpinePeerProfile::getPeerId(ulong & peerId)
{
    peerId = peerId_;
}


void
AlpinePeerProfile::getRelativeQuality(short & quality)
{
    quality = quality_;
}


void
AlpinePeerProfile::getTotalQueries(ulong & total)
{
    total = queries_;
}


void
AlpinePeerProfile::getTotalResponses(ulong & total)
{
    total = responses_;
}


void
AlpinePeerProfile::setPeerId(ulong peerId)
{
#ifdef _VERBOSE
    Log::Debug("AlpinePeerProfile::setPeerId invoked.  Peer ID: "s + std::to_string(peerId));
#endif

    peerId_ = peerId;
}


void
AlpinePeerProfile::setRelativeQuality(short quality)
{
#ifdef _VERBOSE
    Log::Debug("AlpinePeerProfile::setRelativeQuality invoked.  Quality: "s + std::to_string(quality));
#endif

    quality_ = quality;
}


void
AlpinePeerProfile::modifyRelativeQuality(short delta)
{
#ifdef _VERBOSE
    Log::Debug("AlpinePeerProfile::modifyRelativeQuality invoked.  Quality delta: "s + std::to_string(delta));
#endif

    quality_ += delta;
}


void
AlpinePeerProfile::incrTotalQueries()
{
#ifdef _VERBOSE
    Log::Debug("AlpinePeerProfile::incrTotalQueries invoked.  New total: "s + std::to_string(queries_ + 1));
#endif

    queries_++;
}


void
AlpinePeerProfile::incrTotalResponses()
{
#ifdef _VERBOSE
    Log::Debug("AlpinePeerProfile::incrTotalResponses invoked.  New total: "s + std::to_string(responses_ + 1));
#endif

    responses_++;
}
