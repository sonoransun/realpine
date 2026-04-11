/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <DtcpBaseUdpTransport.h>
#include <DtcpSocketThread.h>
#include <DtcpThreadTable.h>
#include <Log.h>
#include <Platform.h>
#include <StringUtils.h>


DtcpSocketThread::DtcpSocketThread(DtcpBaseUdpTransport * udpTransport)
{
#ifdef _VERBOSE
    Log::Debug("DtcpSocketThread constructor invoked.");
#endif

    udpTransport_ = udpTransport;
}


DtcpSocketThread::~DtcpSocketThread()
{
#ifdef _VERBOSE
    Log::Debug("DtcpSocketThread destructor invoked.");
#endif
}


void
DtcpSocketThread::threadMain()
{
#ifdef _VERBOSE
    Log::Debug("DtcpSocketThread::threadMain invoked.");
#endif

    bool finished = false;
    bool status;

    // A higher nice value provides prompt response to incoming data.  Renice this
    // thread if possible.  We use the maximum value as the majority of the thread
    // time is in sleep state waiting on poll() activity. (no system impact)
    //
    if ((getuid() == 0) || (geteuid() == 0)) {
        nice(-40);
    }

    // wait for data on UDP socket and process accordingly.
    // if an error is returned, return and wait for resume.
    //
    while (!finished) {

        status = udpTransport_->processSocketEvents();

        if (!status)
            finished = true;
    }


    // wait for resume.
    //
}
