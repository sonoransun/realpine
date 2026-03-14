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



std::unique_ptr<ApplCore::t_SigHandlerIndex>  ApplCore::sigHandlerIndex_s;
std::unique_ptr<ApplCore::t_MethodIndex>      ApplCore::methodIndex_s;
std::unique_ptr<SignalMonitorThread>           ApplCore::signalMonitor_s;
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
        signalMonitor_s   = std::make_unique<SignalMonitorThread>();
        sigHandlerIndex_s = std::make_unique<t_SigHandlerIndex>();
        methodIndex_s     = std::make_unique<t_MethodIndex>();

        sigHandlerIndex_s->resize(SignalMax + 1);
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

#ifndef ALPINE_PLATFORM_WINDOWS
    if (status)
        status = addSignalHandler (&defaultChildHandler, SIGCHLD);
#endif


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

#ifdef ALPINE_PLATFORM_WINDOWS
    // Windows console control handler for graceful shutdown
    SetConsoleCtrlHandler([](DWORD ctrlType) -> BOOL {
        switch (ctrlType) {
            case CTRL_C_EVENT:
                handleSignal(SIGINT);
                return TRUE;
            case CTRL_BREAK_EVENT:
                handleSignal(SIGTERM);
                return TRUE;
            case CTRL_CLOSE_EVENT:
                handleSignal(SIGTERM);
                return TRUE;
            default:
                return FALSE;
        }
    }, TRUE);
#endif

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

    auto & sigListPtr = (*sigHandlerIndex_s)[signal];

    if (!sigListPtr) {
        // no signal handlers defined for this signal yet, allocate list.
        sigListPtr = std::make_unique<t_SigHandlerList>();
    }

    // Add handler to end of signal handler list.
    // Handlers will be invoked in the order in which they were added.
    //
    sigListPtr->push_back(method);


    // See if this method was previously added to method index
    //
    auto methodAddress = reinterpret_cast<ulong>(method);

    auto methodIter = methodIndex_s->find(methodAddress);
    t_MethodMemberList * memberList = nullptr;

    if (methodIter != methodIndex_s->end()) {
        // this method is already indexed, add this list to handlers.
        memberList = methodIter->second.get();
    } else {
        // not indexed, create new member list and index.
        auto newList = std::make_unique<t_MethodMemberList>();
        memberList = newList.get();
        methodIndex_s->emplace(methodAddress, std::move(newList));
    }

    // Add signal list to member list (used for cleanup if handler is removed)
    //
    memberList->push_back(sigListPtr.get());


    return true;
}



bool
ApplCore::removeSignalHandler (t_SigHandler  method)
{
#ifdef _VERBOSE
    Log::Debug("ApplCore::removeSignalHandler invoked.");
#endif

    WriteLock lock(dataLock_s);

    if (!methodIndex_s || !sigHandlerIndex_s) {
        return false;
    }

    auto methodAddress = reinterpret_cast<ulong>(method);
    auto methodIter = methodIndex_s->find(methodAddress);

    if (methodIter == methodIndex_s->end()) {
        return false;
    }

    // Remove this method from every signal handler list it belongs to
    auto * memberList = methodIter->second.get();
    for (auto * sigList : *memberList) {
        sigList->remove(method);
    }

    // Remove from method index
    methodIndex_s->erase(methodIter);

    return true;
}



bool
ApplCore::removeDefaultHandler (int signal)
{
#ifdef _VERBOSE
    Log::Debug("ApplCore::removeDefaultHandler invoked.");
#endif

    if (signal < 1 || signal > SignalMax) {
        return false;
    }

    WriteLock lock(dataLock_s);

    if (!sigHandlerIndex_s) {
        return false;
    }

    auto & sigListPtr = (*sigHandlerIndex_s)[signal];
    if (!sigListPtr) {
        return false;
    }

    // Remove each handler in this signal's list from the method index
    for (const auto & handler : *sigListPtr) {
        auto methodAddress = reinterpret_cast<ulong>(handler);
        auto methodIter = methodIndex_s->find(methodAddress);
        if (methodIter != methodIndex_s->end()) {
            // Remove this signal list from the method's member list
            auto * memberList = methodIter->second.get();
            memberList->remove(sigListPtr.get());
            // If method has no more signal lists, remove from index
            if (memberList->empty()) {
                methodIndex_s->erase(methodIter);
            }
        }
    }

    // Clear the signal handler list
    sigListPtr.reset();

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



// NOTE: These handlers are invoked from SignalMonitorThread via sigwait(),
// not from async signal context. Log calls are safe here.

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
    ::write(STDERR_FILENO, "FATAL: SIGSEGV received, terminating.\n", 38);
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

#ifndef ALPINE_PLATFORM_WINDOWS
    int childStatus;
    int result;

    result = waitpid (-1, &childStatus, WNOHANG);

    if (result <= 0) {
        // No child after all?
        return;
    }

    Log::Info ("Child process: "s + std::to_string (result) +
               " exited.");
#endif
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

    const auto & handlerListPtr = (*sigHandlerIndex_s)[signal];

    if (!handlerListPtr) {
        // no handlers registered.
        return;
    }

    // iterate through each handler in list and invoke.
    //
    for (const auto & handler : *handlerListPtr) {
        handler();
    }
}



