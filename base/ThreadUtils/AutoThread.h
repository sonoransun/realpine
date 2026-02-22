/// Copyright (C) 2026 sonoransun — see LICENCE.txt


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

