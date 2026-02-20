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


#pragma once
#include <Common.h>
#include <ReadWriteSem.h>
#include <OptHash.h>
#include <vector>
#include <list>


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

