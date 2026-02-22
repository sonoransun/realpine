/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <DtcpStackThread.h>
#include <DtcpBaseUdpTransport.h>
#include <DtcpThreadTable.h>
#include <Log.h>
#include <StringUtils.h>



DtcpStackThread::DtcpStackThread (DtcpBaseUdpTransport *  udpTransport)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpStackThread constructor invoked.");
#endif

    udpTransport_ = udpTransport;
}



DtcpStackThread::~DtcpStackThread ()
{
#ifdef _VERBOSE
    Log::Debug ("DtcpStackThread destructor invoked.");
#endif
}



void 
DtcpStackThread::threadMain ()
{
#ifdef _VERBOSE
    Log::Debug ("DtcpStackThread::threadMain invoked.");
#endif

    t_ThreadId  id;
    getThreadId (id);

    bool finished = false;
    bool status;

    // Process events in queue until exhausted, then return and wait.
    //
    while (!finished) {

        status = udpTransport_->processEvents (id);

        if (!status) {
            // nothing left in queue, finished...
            finished = true;
        }
    }

    udpTransport_->threadIdle (id);
}



