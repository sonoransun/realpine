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


#include <SignalMonitorThread.h>
#include <SignalSet.h>
#include <ThreadSigMask.h>
#include <Log.h>
#include <StringUtils.h>
#include <Platform.h>



// Ctor defaulted in header



SignalMonitorThread::~SignalMonitorThread () = default;


  
void 
SignalMonitorThread::threadMain ()
{
    // All we do here is endlessly wait for signals, then dispatch
    // the handleSignal method in ApplCore...
    //

    pid_t  threadPid;
    threadPid = getpid ();
    Log::Info ("Signal monitor thread is process ID: "s + std::to_string (threadPid));


    int result;
    int sigNumber;
    sigset_t  sysSigSet;
    SignalSet sigSet;
    sigSet.fill ();

// TEMP
    ThreadSigMask::setSigMask (sigSet);


    while (true) {

        // Clear our signal set and wait for a signal
        //
        //ThreadSigMask::setSigMask (sigSet);
        sigSet.getSignalSet (sysSigSet);

        result = sigwait (&sysSigSet, &sigNumber);

        if (result == 0) {

            // received a signal, mask all signals and dispatch...
            //
            //ThreadSigMask::setSigMask (sigSet);

            string signalName;
            SignalSet::signalAsString (sigNumber, signalName);

            Log::Debug ("Received signal '"s + signalName + "' (" +
                        std::to_string (sigNumber) + ")");

            ApplCore::handleSignal (sigNumber);
        }
        else {
            Log::Error ("Call to sigwait failed with value: "s +
                                 std::to_string (result) + " .");
        }
    }

    // should never get here...
}



