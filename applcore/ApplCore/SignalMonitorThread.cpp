/// Copyright (C) 2026 sonoransun — see LICENCE.txt


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

#ifdef ALPINE_PLATFORM_WINDOWS
    // Windows: signals are handled via SetConsoleCtrlHandler in ApplCore.
    // This thread just sleeps to keep the AutoThread lifecycle alive.
    while (true) {
        alpine_usleep(1000000);
    }
#else
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
#endif

    // should never get here...
}



