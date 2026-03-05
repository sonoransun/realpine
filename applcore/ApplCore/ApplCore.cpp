/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <ApplCore.h>
#include <SignalSet.h>
#include <ThreadSigMask.h>
#include <SignalMonitorThread.h>
#include <WriteLock.h>
#include <ReadLock.h>
#include <Log.h>
#include <StringUtils.h>
#include <Platform.h>
#ifndef ALPINE_PLATFORM_WINDOWS
#include <sys/wait.h>
#include <unistd.h>
#endif



ApplCore::t_SigHandlerIndex *     ApplCore::sigHandlerIndex_s = nullptr;
ApplCore::t_MethodIndex *         ApplCore::methodIndex_s = nullptr;
SignalMonitorThread *             ApplCore::signalMonitor_s = nullptr;
ReadWriteSem                      ApplCore::dataLock_s;
std::atomic<int>                  ApplCore::shutdownSignal_s{0};
std::atomic<bool>                 ApplCore::shutdownRequested_s{false};



// Ctor defaulted in header


// Dtor defaulted in header



bool 
ApplCore::initialize (int      argc,
                      char **  argv)
{
    Log::Info ("Application core initialize invoked; preparing core data.");

    updateLog (argc, argv);


    // Mask all signals in this thread (must be main process thread).
    // This prevents problems with signals received in various other threads
    // of execution, which cause major problems.
    //
    SignalSet  sigSet;
    sigSet.fill ();

    ThreadSigMask::setSigMask (sigSet);


    // Scope lock
    {
        WriteLock  lock(dataLock_s);

        // initialize signal handler indexes
        //
        signalMonitor_s   = new SignalMonitorThread;
        sigHandlerIndex_s = new t_SigHandlerIndex;
        methodIndex_s     = new t_MethodIndex;

        sigHandlerIndex_s->resize (SignalMax +1);

        // initialize all list pointers to null to indicate no handlers present.
        //
        for (auto& sigItem : *sigHandlerIndex_s) {
            sigItem = nullptr;
        }
    }


    // Allocate a handler list for each default signal handler
    //
    bool status = true;

    if (status)
        status = addSignalHandler (&defaultInterruptHandler, SIGINT);
   
    if (status) 
        status = addSignalHandler (&defaultQuitHandler, SIGQUIT);

    if (status)
        status = addSignalHandler (&defaultAbortHandler, SIGABRT);

    if (status)
        status = addSignalHandler (&defaultSegvHandler, SIGSEGV);

    if (status)
        status = addSignalHandler (&defaultTerminateHandler, SIGTERM);

    if (status)
        status = addSignalHandler (&defaultChildHandler, SIGCHLD);


    if (!status) {
        Log::Error ("Error loading default signal handlers in ApplCore::initialize.");
        return false;
    }

    // Scope lock
    {
        WriteLock  lock(dataLock_s);

        // start signal monitor thread
        //
        signalMonitor_s->run ();
    }


    return true;
}



bool 
ApplCore::addSignalHandler (t_SigHandler  method,
                            int           signal)
{
#ifdef _VERBOSE
    Log::Debug ("ApplCore::addSignalHandler invoked.");
#endif

    if ( (signal < 1) ||
         (signal > SignalMax) ) {

        // Invalid signal number
#ifdef _VERBOSE
       Log::Error ("Invalid signal number passed to ApplCore::addSignalHandler.");
#endif

       return false;
    }


    WriteLock  lock(dataLock_s);

    if (!sigHandlerIndex_s) {
        // not initialized
        return false;
    }

    t_SigHandlerList *  sigList;
    sigList = (*sigHandlerIndex_s)[signal];

    if (!sigList) {
        // no signal handlers defined for this signal yet, allocate list.
        //
        sigList = new t_SigHandlerList;
        (*sigHandlerIndex_s)[signal] = sigList;
    }

    // Add handler to end of signal handler list.
    // Handlers will be invoked in the order in which they were added.
    //
    sigList->push_back (method);


    // See if this method was previously added to method index
    //
    ulong methodAddress = reinterpret_cast<ulong>(method); // method is reference bu ulong address
    t_MethodMemberList * memberList = nullptr;

    auto methodIter = methodIndex_s->find (methodAddress);

    if (methodIter != methodIndex_s->end ()) {
        // this method is already indexed, add this list to handlers.
        memberList = (*methodIter).second;
    }
    else {
        // not indexed, create new member list and index.
        memberList = new t_MethodMemberList;
        methodIndex_s->emplace (methodAddress, memberList);
    }
         
    // Add signal list to member list (used for cleanup if handler is removed)
    //
    memberList->push_back (sigList);


    return true;
}



bool 
ApplCore::removeSignalHandler (t_SigHandler  method)
{
#ifdef _VERBOSE
    Log::Debug ("ApplCore::removeSignalHandler invoked.");
#endif


    return true;
}



bool 
ApplCore::removeDefaultHandler (int signal)
{
#ifdef _VERBOSE
    Log::Debug ("ApplCore::removeDefaultHandler invoked.");
#endif


    return true;
}



void  
ApplCore::updateLog (int      argc,
                     char **  argv)
{
    const string  logFileOption ("--logFile");
    const string  logLevelOption ("--logLevel");

    char ** currArgPtr;
    char *  currArg = nullptr;
    char *  lastArg;

    currArgPtr = argv;
    currArg = *(currArgPtr++);
    lastArg = argv[argc -1];

    char * logFile  = nullptr;
    char * logLevel = nullptr;

    bool done = false;

    while (!done) {
        std::string_view arg(currArg);

        if (arg == logFileOption) {
            if (currArg != lastArg) {
                logFile = *currArgPtr;
            }
        }
        else if (arg == logLevelOption) {
            if (currArg != lastArg) {
                logLevel = *currArgPtr;
            }
        }

        if (currArg == lastArg) {
            done = true;
        }
        else {
            currArg = *(currArgPtr++);
        }
    }


    // Setup log level
    //
    bool  isValid;
    Log::t_LogLevel  newLogLevel = Log::t_LogLevel::Debug; // default log level

    if (!logLevel) {
        Log::Error ("No log level specified.  --logLevel option required!");
    }
    else {
        isValid = Log::stringToLogLevel (std::string(logLevel), newLogLevel);

        if (!isValid) {
            Log::Error ("Invalid log level passed on command line!");
        }
    }

    Log::setLogLevel (newLogLevel);

     
    // Setup log file
    //   
    if (!logFile) {
        Log::Error ("No logFile specified.  --logFile option required!");
        return;
    }

    Log::initialize (std::string(logFile), newLogLevel);
}



bool
ApplCore::isShutdownRequested ()
{
    return shutdownRequested_s.load (std::memory_order_acquire);
}



int
ApplCore::getShutdownSignal ()
{
    return shutdownSignal_s.load (std::memory_order_acquire);
}



void
ApplCore::defaultInterruptHandler ()
{
#ifdef _VERBOSE
    Log::Debug ("ApplCore::defaultInterruptHandler invoked.");
#endif

    Log::Info ("Received SIGINT, requesting shutdown."s);
    shutdownSignal_s.store (SIGINT, std::memory_order_release);
    shutdownRequested_s.store (true, std::memory_order_release);
}



void
ApplCore::defaultQuitHandler ()
{
#ifdef _VERBOSE
    Log::Debug ("ApplCore::defaultQuitHandler invoked.");
#endif

    Log::Info ("Received SIGQUIT, requesting shutdown."s);
    shutdownSignal_s.store (SIGQUIT, std::memory_order_release);
    shutdownRequested_s.store (true, std::memory_order_release);
}



void
ApplCore::defaultAbortHandler ()
{
#ifdef _VERBOSE
    Log::Debug ("ApplCore::defaultAbortHandler invoked.");
#endif

    Log::Info ("Received SIGABORT, requesting shutdown."s);
    shutdownSignal_s.store (SIGABRT, std::memory_order_release);
    shutdownRequested_s.store (true, std::memory_order_release);
}



void
ApplCore::defaultSegvHandler ()
{
    // SIGSEGV — corrupt state, unsafe to log or allocate.
    // Use _exit() to terminate immediately.
    _exit (1);
}



void
ApplCore::defaultTerminateHandler ()
{
#ifdef _VERBOSE
    Log::Debug ("ApplCore::defaultTerminateHandler invoked.");
#endif

    Log::Info ("Received SIGTERM, requesting shutdown."s);
    shutdownSignal_s.store (SIGTERM, std::memory_order_release);
    shutdownRequested_s.store (true, std::memory_order_release);
}



void  
ApplCore::defaultChildHandler ()
{
#ifdef _VERBOSE
    Log::Debug ("ApplCore::defaultChildHandler invoked.");
#endif

    Log::Info ("Received SIGCHLD, checking child PIDs...");

    int childStatus;
    int result;

    result = waitpid (-1, &childStatus, WNOHANG);

    if (result <= 0) {
        // No child after all?
        return;
    }

    Log::Info ("Child process: "s + std::to_string (result) +
               " exited.");
}



void  
ApplCore::handleSignal (int signal)
{
#ifdef _VERBOSE
    Log::Debug ("ApplCore::handleSignal invoked.");
#endif

    if ( (signal < 1) ||
         (signal > SignalMax) ) {

        // Invalid signal number
       Log::Error ("Invalid signal number passed to ApplCore::handleSignal.");
       return;
    }


    ReadLock  lock(dataLock_s);

    t_SigHandlerList *  handlerList;
    handlerList = (*sigHandlerIndex_s)[signal];

    if (!handlerList) {
        // no handlers registered.
        return;
    }


    // iterate through each handler in list and invoke.
    //
    t_SigHandler  sigHandler;

    for (const auto& handler : *handlerList) {

        sigHandler = handler;
        sigHandler();
    }
}



