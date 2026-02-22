/// Copyright (C) 2026 sonoransun — see LICENCE.txt


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



