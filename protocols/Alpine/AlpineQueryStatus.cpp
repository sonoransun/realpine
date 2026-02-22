/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AlpineQueryStatus.h>
#include <Log.h>
#include <StringUtils.h>



// Ctor defaulted in header



AlpineQueryStatus::AlpineQueryStatus (const AlpineQueryStatus & copy)
{
}



// Dtor defaulted in header



const AlpineQueryStatus & 
AlpineQueryStatus::operator = (const AlpineQueryStatus & copy)
{
    if (&copy == this) {
        return *this;
    }


    return *this;
}



ulong
AlpineQueryStatus::totalPackets ()
{
    return totalPackets_;
}



ulong  
AlpineQueryStatus::numPacketsSent ()
{
    return packetsSent_;
}



ulong  
AlpineQueryStatus::numRepliesReceived ()
{
    return repliesReceived_;
}



double 
AlpineQueryStatus::percentComplete ()
{
    return percentComplete_;
}



bool   
AlpineQueryStatus::isActive ()
{
    return isActive_;
}



void
AlpineQueryStatus::setTotalPackets (ulong  totalPackets)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryStatus::setTotalPackets invoked.");
#endif

    totalPackets_ = totalPackets;
}



void  
AlpineQueryStatus::setPacketsSent (ulong  numSent)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryStatus::setPacketsSent invoked.");
#endif

    packetsSent_ = numSent;
}



void  
AlpineQueryStatus::setRepliesReceived (ulong  numReceived)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryStatus::setRepliesReceived invoked.");
#endif

    repliesReceived_ = numReceived;
}



void  
AlpineQueryStatus::setPercentComplete (const double &  percentage)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryStatus::setPercentComplete invoked.");
#endif

    percentComplete_ = percentage;
}



void  
AlpineQueryStatus::setIsActive (bool  value)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryStatus::setIsActive invoked.");
#endif

    isActive_ = value;
}



