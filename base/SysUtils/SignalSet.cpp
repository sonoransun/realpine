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


#include <SignalSet.h>


SignalSet::SignalSet ()
{
    sigemptyset (&signalSet_);
}



// Dtor defaulted in header



bool 
SignalSet::clear ()
{
    sigemptyset (&signalSet_);

    return true;
}



bool 
SignalSet::fill ()
{
    sigfillset (&signalSet_);

    return true;
}



bool 
SignalSet::add (int sigNumber)
{
    sigaddset (&signalSet_, sigNumber);

    return true;
}



bool 
SignalSet::remove (int sigNumber)
{
    sigdelset (&signalSet_, sigNumber);

    return true;
}



bool 
SignalSet::getSignalSet (sigset_t & signalSet)
{
    signalSet = signalSet_;

    return true;
}



static const char * SignalNameArray [] = {
    "invalid zero signal",
    "hangup",
    "interrupt",
    "quit",
    "illegal instruction",
    "trace trap",
    "abort",
    "bus error",
    "floating point exception",
    "kill",
    "user 1",
    "segmentation violation",
    "user 2",
    "broken pipe",
    "alarm",
    "terminate",
    "stack fault",
    "child exit",
    "continue",
    "stop",
    "keyboard stop",
    "TTY input",
    "TTY output",
    "urgent socket condition",
    "CPU limit exceeded",
    "file size limit exceeded",
    "virtual alarm",
    "profiling alarm",
    "window size change",
    "I/O event",
    "power event",
    "bad system call"
};



bool 
SignalSet::signalAsString (int       sigNumber,
                           string &  sigString)
{
    const int maxSigNumber = SignalMax;

    if ( (sigNumber < 1) ||
         (sigNumber > maxSigNumber) ) {

        // Invalid signal number...
        return false;
    }

    sigString = SignalNameArray[sigNumber];


    return true;
}



