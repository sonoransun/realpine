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
#include <mutex>
#include <condition_variable>


class AutoThread
{
  public:

    AutoThread ();

    virtual ~AutoThread ();



    // Derived threadMain ()
    //
    virtual void threadMain () = 0;


    bool create ();

    bool destroy ();

    bool resume ();

    bool isActive ();

    bool getThreadId (t_ThreadId & threadId);



  private:

    std::thread         thread_;
    t_ThreadId          threadId_;

    std::atomic<bool>   running_{false};
    std::atomic<bool>   created_{false};
    std::atomic<bool>   destroyed_{false};

    // Condition variable used for pause/resume signaling.
    // The original used Mutex acquire/release pairs as a
    // semaphore; we now use a proper CV for clarity.
    //
    std::mutex          cvMutex_;
    std::condition_variable cv_;
    bool                resumed_{false};


    static void threadEntry (AutoThread * self);

};

