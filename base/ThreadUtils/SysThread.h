/// Copyright (C) 2026 sonoransun — see LICENCE.txt


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



  protected:

    /// Returns true while the thread should keep running.
    /// Subclasses should check this in their thread loop to
    /// support clean join-on-destroy shutdown.
    ///
    [[nodiscard]] bool shouldContinue () const;


  private:

    std::thread     thread_;
    t_ThreadId      threadId_;

    bool            deleteOnExit_;
    std::atomic<bool> running_{false};
    std::atomic<bool> created_{false};
    Mutex           waitLock_;


    static void threadEntry (SysThread * self);

};

