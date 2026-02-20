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

