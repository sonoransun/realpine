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



