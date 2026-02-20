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


