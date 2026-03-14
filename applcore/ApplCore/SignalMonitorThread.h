/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <SysThread.h>
#include <ApplCore.h>
#include <Platform.h>

#ifndef ALPINE_PLATFORM_WINDOWS
#include <signal.h>
#include <pthread.h>
#endif


class SignalMonitorThread : public SysThread
{
  public:

    SignalMonitorThread () = default;
    virtual ~SignalMonitorThread ();


    virtual void threadMain ();

    /// Wake the signal monitor thread from sigwait() so it can
    /// observe the running flag and exit cleanly during shutdown.
    ///
    void wakeForShutdown ();


  private:

#ifndef ALPINE_PLATFORM_WINDOWS
    std::atomic<pthread_t>  nativeThread_{};
    std::atomic<bool>       nativeThreadValid_{false};
#endif

};

