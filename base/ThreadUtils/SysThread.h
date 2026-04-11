/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <Mutex.h>
#include <ThreadTypes.h>
#include <atomic>
#include <stop_token>
#include <thread>


class SysThread
{
  public:
    SysThread();

    virtual ~SysThread();


    // Derived threadMain — legacy overload (no stop_token).
    //
    virtual void threadMain() = 0;

    // Derived threadMain — cooperative-cancellation overload.
    // Default implementation delegates to the legacy threadMain()
    // so existing subclasses keep working without changes.
    //
    virtual void threadMain(std::stop_token stopToken);


    // Thread operations
    //
    void setDeleteOnExit(bool deleteOnExit);

    bool run();

    bool stop();

    bool isActive();

    t_ThreadId getThreadId();


  protected:
    /// Returns true while the thread should keep running.
    /// Checks both the running_ flag and the jthread stop token.
    ///
    [[nodiscard]] bool shouldContinue() const;


  private:
    std::jthread thread_;
    t_ThreadId threadId_;

    bool deleteOnExit_;
    std::atomic<bool> running_{false};
    std::atomic<bool> created_{false};
    Mutex waitLock_;

    // Cached stop_token from the jthread, used by shouldContinue().
    std::stop_token stopToken_;


    static void threadEntry(std::stop_token stopToken, SysThread * self);
};
