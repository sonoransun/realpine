/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <ThreadSigMask.h>
#include <Log.h>
#include <StringUtils.h>


// Ctor defaulted in header


// Dtor defaulted in header



bool 
ThreadSigMask::setSigMask (SignalSet & sigSet)
{
#ifdef _VERBOSE
    Log::Debug ("ThreadSigMask::setSigMask invoked.");
#endif

    const int how = SIG_SETMASK;
    sigset_t signals;

    sigSet.getSignalSet (signals);

#if defined(_VERY_VERBOSE) && defined(ALPINE_PLATFORM_POSIX)
    int i;
    string sigString;

    for (i=0; i < _SIGSET_NWORDS; i++) {
        sigString += std::to_string (signals.__val[i]);
        sigString += "-";
    }

    Log::Debug ("SignalMask: "s + sigString);
#endif

    int retVal;
    retVal = pthread_sigmask (how, &signals, 0);

    if (retVal != 0) {
        Log::Error ("Setting signal mask failed with error code: "s +
                    std::to_string (retVal) + " in ThreadSigMask::setSigMask.");
        return false;
    }


    return true;
}



