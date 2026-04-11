/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <Mutex.h>
#include <ThreadTypes.h>
#include <thread>


class ThreadUtils
{
  public:
    ThreadUtils() = default;

    ~ThreadUtils() = default;


    static ulong getThreadIndex();

    static t_ThreadId getThreadId();

    static ushort numThreads();


    /// Sets the name of the *current* thread as it appears in /proc/self/task/<tid>/comm
    /// (Linux), top, and debuggers.  Linux truncates to 15 bytes + NUL.  Windows is a no-op
    /// for now (thread naming on Windows is high-cost and low-value).
    ///
    static void setCurrentThreadName(const string & name);


    // std::thread::id has a built-in std::hash specialization,
    // so no custom hasher is needed.
    //
    using t_IdIndex = std::unordered_map<t_ThreadId, ulong>;

    using t_IdIndexPair = std::pair<t_ThreadId, ulong>;


  private:
    static ushort threadCount_s;
    static ulong currThreadIndex_s;
    static t_IdIndex threadIdIndex_s;
    static Mutex dataLock_s;


    static void addThread(t_ThreadId id);

    static void deleteThread(t_ThreadId id);


    friend class SysThread;
    friend class AutoThread;
};
