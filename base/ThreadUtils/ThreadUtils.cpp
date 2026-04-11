/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <Log.h>
#include <MutexLock.h>
#include <StringUtils.h>
#include <ThreadUtils.h>

#if defined(__linux__) || defined(__FreeBSD__) || defined(__DragonFly__) || defined(__APPLE__)
#include <pthread.h>
#endif


ushort ThreadUtils::threadCount_s = 0;
ulong ThreadUtils::currThreadIndex_s = 1;
ThreadUtils::t_IdIndex ThreadUtils::threadIdIndex_s;
Mutex ThreadUtils::dataLock_s;


// Ctor defaulted in header


// Dtor defaulted in header


ulong
ThreadUtils::getThreadIndex()
{
    // NOTE: NO logging is used in this method, as this would create a
    // cyclic call pattern between Log::timestamp and getThreadId. (kaBOOM!)
    //
    ulong index;
    t_ThreadId id;

    id = std::this_thread::get_id();

    auto iter = threadIdIndex_s.find(id);

    if (iter == threadIdIndex_s.end()) {
        index = 0;
    } else {
        index = (*iter).second;
    }

    return index;
}


t_ThreadId
ThreadUtils::getThreadId()
{
    return (std::this_thread::get_id());
}


ushort
ThreadUtils::numThreads()
{
#ifdef _VERBOSE
    Log::Debug("ThreadUtils::getThreadId invoked.");
#endif

    ushort count;
    MutexLock lock(dataLock_s);

    count = threadCount_s;

    return count;
}


void
ThreadUtils::addThread(t_ThreadId id)
{
#ifdef _VERBOSE
    Log::Debug("ThreadUtils::addThread invoked.");
#endif

    MutexLock lock(dataLock_s);

    threadCount_s++;
    threadIdIndex_s.emplace(id, currThreadIndex_s++);
}


void
ThreadUtils::deleteThread(t_ThreadId id)
{
#ifdef _VERBOSE
    Log::Debug("ThreadUtils::getThreadId invoked.");
#endif

    MutexLock lock(dataLock_s);

    threadCount_s--;
    threadIdIndex_s.erase(id);
}


void
ThreadUtils::setCurrentThreadName(const string & name)
{
#if defined(__linux__) || defined(__FreeBSD__) || defined(__DragonFly__)
#ifdef ALPINE_HAVE_PTHREAD_SETNAME_NP
    // Linux limits thread names to 16 bytes including the NUL terminator,
    // so truncate to 15 characters before passing to pthread_setname_np.
    //
    string truncated = name.substr(0, 15);
    pthread_setname_np(pthread_self(), truncated.c_str());
#else
    // TODO: wire ALPINE_HAVE_PTHREAD_SETNAME_NP probe in CMakeLists.txt;
    //       fall back to direct Linux call in the meantime so thread names
    //       still show up in /proc/<pid>/task/<tid>/comm.
#if defined(__linux__)
    string truncated = name.substr(0, 15);
    pthread_setname_np(pthread_self(), truncated.c_str());
#else
    (void)name;
#endif
#endif
#elif defined(__APPLE__)
    // Darwin variant takes only the name string (operates on current thread).
    // The macOS limit is 64 bytes including NUL.
    //
    string truncated = name.substr(0, 63);
    pthread_setname_np(truncated.c_str());
#else
    // Windows (and anything else): no-op for this first pass.
    //
    (void)name;
#endif
}
