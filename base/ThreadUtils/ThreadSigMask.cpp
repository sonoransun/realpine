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



