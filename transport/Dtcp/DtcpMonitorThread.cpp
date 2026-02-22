/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <DtcpMonitorThread.h>
#include <DtcpBaseUdpTransport.h>
#include <DtcpThreadTable.h>
#include <Log.h>
#include <StringUtils.h>


DtcpMonitorThread::DtcpMonitorThread (DtcpBaseUdpTransport * udpTransport)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpMonitorThread constructor invoked.");
#endif

    udpTransport_ = udpTransport;
}



DtcpMonitorThread::~DtcpMonitorThread ()
{
#ifdef _VERBOSE
    Log::Debug ("DtcpMonitorThread destructor invoked.");
#endif
}



void 
DtcpMonitorThread::threadMain ()
{
#ifdef _VERBOSE
    Log::Debug ("DtcpMonitorThread::threadMain invoked.");
#endif

    bool finished = false;
    bool status;

    // process timed events for UDP transport until directed otherwise.
    // 
    while (!finished) {    

        status = udpTransport_->processTimedEvents ();

        if (!status)
            finished = true;
    }


    // return and wait for resume.
    //
}


