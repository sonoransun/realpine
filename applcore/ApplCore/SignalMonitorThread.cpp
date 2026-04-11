/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <Log.h>
#include <Platform.h>
#include <SignalMonitorThread.h>
#include <SignalSet.h>
#include <StringUtils.h>
#include <ThreadSigMask.h>
#include <ThreadUtils.h>

#ifndef ALPINE_PLATFORM_WINDOWS
#include <cerrno>
#include <pthread.h>
#include <signal.h>
#endif


// Ctor defaulted in header


SignalMonitorThread::~SignalMonitorThread()
{
    // Wake the thread from sigwait() so it can observe the
    // running flag and exit, allowing join in the base destructor.
    //
    wakeForShutdown();
}


void
SignalMonitorThread::wakeForShutdown()
{
#ifndef ALPINE_PLATFORM_WINDOWS
    // Send SIGUSR1 to the monitor thread to unblock sigwait().
    //
    if (nativeThreadValid_.load()) {
        pthread_t tid = nativeThread_.load();
        pthread_kill(tid, SIGUSR1);
    }
#endif
}


void
SignalMonitorThread::threadMain()
{
    // All we do here is endlessly wait for signals, then dispatch
    // the handleSignal method in ApplCore...
    //
    ThreadUtils::setCurrentThreadName("alpine-signal"s);

    pid_t threadPid;
    threadPid = getpid();
    Log::Info("Signal monitor thread is process ID: "s + std::to_string(threadPid));

#ifdef ALPINE_PLATFORM_WINDOWS
    // Windows: signals are handled via SetConsoleCtrlHandler in ApplCore.
    // This thread just sleeps to keep the SysThread lifecycle alive.
    while (shouldContinue()) {
        alpine_usleep(500000);
    }
#else
    // Store the native thread handle so wakeForShutdown() can
    // send SIGUSR1 to unblock sigwait().
    //
    nativeThread_.store(pthread_self());
    nativeThreadValid_.store(true);

    int result;
    int sigNumber;
    sigset_t sysSigSet;
    SignalSet sigSet;
    sigSet.fill();

    // Add SIGUSR1 to the set so sigwait() will return when we
    // send it during shutdown.
    //
    sigSet.add(SIGUSR1);

    ThreadSigMask::setSigMask(sigSet);


    while (shouldContinue()) {

        // Clear our signal set and wait for a signal
        //
        sigSet.getSignalSet(sysSigSet);

        result = sigwait(&sysSigSet, &sigNumber);

        if (result != 0) {
            Log::Error("Call to sigwait failed with value: "s + std::to_string(result) + " .");
            continue;
        }

        // SIGUSR1 is our shutdown wakeup signal — exit the loop.
        //
        if (sigNumber == SIGUSR1) {
            break;
        }

        // Received a real signal, dispatch...
        //
        string signalName;
        SignalSet::signalAsString(sigNumber, signalName);

        Log::Debug("Received signal '"s + signalName + "' (" + std::to_string(sigNumber) + ")");

        ApplCore::handleSignal(sigNumber);
    }
#endif
}
