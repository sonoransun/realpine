/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <ReadWriteSem.h>
#include <OptHash.h>
#include <vector>
#include <list>
#include <atomic>


class SignalMonitorThread;


class ApplCore
{
  public:

    ApplCore () = default;
    ~ApplCore () = default;


    using t_SigHandler = void(*)();


    static bool initialize (int      argc,
                            char **  argv);

    static bool addSignalHandler (t_SigHandler  method,
                                  int           signal);

    static bool removeSignalHandler (t_SigHandler  method);

    static bool removeDefaultHandler (int signal);

    static bool isShutdownRequested ();

    static int  getShutdownSignal ();


    using t_SigHandlerList = list<t_SigHandler>;

    using t_SigHandlerIndex = vector<t_SigHandlerList *>;


    using t_MethodMemberList = list<t_SigHandlerList *>;

    using t_MethodIndex = std::unordered_map <ulong,
                      t_MethodMemberList *,
                      OptHash<ulong>,
                      equal_to<ulong> >;

    using t_MethodIndexPair = std::pair <ulong, t_MethodMemberList *>;
                      

  private:

    static t_SigHandlerIndex *    sigHandlerIndex_s;
    static t_MethodIndex *        methodIndex_s;
    static SignalMonitorThread *  signalMonitor_s;
    static ReadWriteSem           dataLock_s;

    static std::atomic<int>       shutdownSignal_s;
    static std::atomic<bool>      shutdownRequested_s;


    static void  updateLog (int      argc,
                            char **  argv);

    // default handlers for specific signals
    //
    static void  defaultInterruptHandler ();

    static void  defaultQuitHandler ();

    static void  defaultAbortHandler ();

    static void  defaultSegvHandler ();

    static void  defaultTerminateHandler ();

    static void  defaultChildHandler ();


    // Invoked by signal monitor thread.
    //
    static void  handleSignal (int signal);


    friend class SignalMonitorThread;

};

