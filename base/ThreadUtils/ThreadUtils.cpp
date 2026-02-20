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



