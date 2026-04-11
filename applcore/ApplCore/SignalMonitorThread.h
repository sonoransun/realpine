/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <ApplCore.h>
#include <Common.h>
#include <Platform.h>
#include <SysThread.h>

#ifndef ALPINE_PLATFORM_WINDOWS
#include <pthread.h>
#include <signal.h>
#endif


class SignalMonitorThread : public SysThread
{
  public:
    SignalMonitorThread() = default;
    virtual ~SignalMonitorThread();


    virtual void threadMain();

    /// Wake the signal monitor thread from sigwait() so it can
    /// observe the running flag and exit cleanly during shutdown.
    ///
    void wakeForShutdown();


  private:
#ifndef ALPINE_PLATFORM_WINDOWS
    std::atomic<pthread_t> nativeThread_{};
    std::atomic<bool> nativeThreadValid_{false};
#endif
};
