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
#include <atomic>


class SysThread
{
  public:

    SysThread ();

    virtual ~SysThread ();



    // Derived threadMain ()
    //
    virtual void threadMain () = 0;


    // Thread operations
    //
    void setDeleteOnExit (bool deleteOnExit);

    bool run ();

    bool stop ();

    bool isActive ();

    t_ThreadId  getThreadId ();



  private:

    std::thread     thread_;
    t_ThreadId      threadId_;

    bool            deleteOnExit_;
    std::atomic<bool> running_{false};
    std::atomic<bool> created_{false};
    Mutex           waitLock_;


    static void threadEntry (SysThread * self);

};

