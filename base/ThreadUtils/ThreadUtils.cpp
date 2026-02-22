/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <ThreadUtils.h>
#include <MutexLock.h>
#include <Log.h>
#include <StringUtils.h>


ushort                   ThreadUtils::threadCount_s = 0;
ulong                    ThreadUtils::currThreadIndex_s = 1; 
ThreadUtils::t_IdIndex   ThreadUtils::threadIdIndex_s;
Mutex                    ThreadUtils::dataLock_s;



// Ctor defaulted in header


// Dtor defaulted in header


   
ulong 
ThreadUtils::getThreadIndex ()
{
    // NOTE: NO logging is used in this method, as this would create a 
    // cyclic call pattern between Log::timestamp and getThreadId. (kaBOOM!)
    //
    ulong       index;
    t_ThreadId  id;

    id = std::this_thread::get_id ();

    auto iter = threadIdIndex_s.find (id);

    if (iter == threadIdIndex_s.end ()) {
        index = 0;
    }
    else {
        index = (*iter).second;
    }

    return index;
}



t_ThreadId
ThreadUtils::getThreadId ()
{
    return (std::this_thread::get_id ());
}



ushort 
ThreadUtils::numThreads ()
{
#ifdef _VERBOSE
    Log::Debug ("ThreadUtils::getThreadId invoked.");
#endif

    ushort count;
    MutexLock  lock(dataLock_s);

    count = threadCount_s;

    return count;
}



void  
ThreadUtils::addThread (t_ThreadId  id)
{
#ifdef _VERBOSE
    Log::Debug ("ThreadUtils::addThread invoked.");
#endif

    MutexLock  lock(dataLock_s);

    threadCount_s++;
    threadIdIndex_s.emplace (id, currThreadIndex_s++);
}



void  
ThreadUtils::deleteThread (t_ThreadId  id)
{
#ifdef _VERBOSE
    Log::Debug ("ThreadUtils::getThreadId invoked.");
#endif

    MutexLock  lock(dataLock_s);

    threadCount_s--;
    threadIdIndex_s.erase (id);
}



