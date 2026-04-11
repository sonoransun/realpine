/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <Mutex.h>
#include <ThreadTypes.h>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <stop_token>
#include <thread>


class AutoThread
{
  public:
    AutoThread();

    virtual ~AutoThread();


    // Derived threadMain — legacy overload (no stop_token).
    //
    virtual void threadMain() = 0;

    // Derived threadMain — cooperative-cancellation overload.
    // Default implementation delegates to the legacy threadMain()
    // so existing subclasses keep working without changes.
    //
    virtual void threadMain(std::stop_token stopToken);


    bool create();

    bool destroy();

    bool resume();

    bool isActive();

    bool getThreadId(t_ThreadId & threadId);


  private:
    std::jthread thread_;
    t_ThreadId threadId_;

    std::atomic<bool> running_{false};
    std::atomic<bool> created_{false};
    std::atomic<bool> destroyed_{false};

    // Condition variable used for pause/resume signaling.
    // The original used Mutex acquire/release pairs as a
    // semaphore; we now use a proper CV for clarity.
    //
    std::mutex cvMutex_;
    std::condition_variable cv_;
    bool resumed_{false};


    static void threadEntry(std::stop_token stopToken, AutoThread * self);
};
