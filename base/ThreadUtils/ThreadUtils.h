/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <ThreadTypes.h>
#include <Mutex.h>
#include <thread>


class ThreadUtils
{
  public:

    ThreadUtils () = default;

    ~ThreadUtils () = default;



    static ulong  getThreadIndex ();

    static t_ThreadId  getThreadId ();

    static ushort  numThreads ();



    // std::thread::id has a built-in std::hash specialization,
    // so no custom hasher is needed.
    //
    using t_IdIndex = std::unordered_map <t_ThreadId, ulong>;

    using t_IdIndexPair = std::pair <t_ThreadId, ulong>;


  private:

    static ushort     threadCount_s;
    static ulong      currThreadIndex_s;
    static t_IdIndex  threadIdIndex_s;
    static Mutex      dataLock_s;


    static void  addThread (t_ThreadId  id);

    static void  deleteThread (t_ThreadId  id);


    friend class SysThread;
    friend class AutoThread;
};

