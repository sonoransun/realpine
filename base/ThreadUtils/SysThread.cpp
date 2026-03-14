/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <SysThread.h>
#include <ThreadUtils.h>
#include <Log.h>
#include <StringUtils.h>
#include <Platform.h>


SysThread::SysThread ()
{
#ifdef _VERBOSE
    Log::Debug ("SysThread constructor invoked.");
#endif

    deleteOnExit_ = false;

    waitLock_.acquire ();
}



SysThread::~SysThread ()
{
#ifdef _VERBOSE
    Log::Debug ("SysThread destructor invoked.");
#endif

    // If deleteOnExit_ is true, the thread will delete itself,
    // so we must detach.  Otherwise join for a clean shutdown.
    //
    running_ = false;

    if (deleteOnExit_ && thread_.joinable ()) {
        thread_.detach ();
    } else if (thread_.joinable ()) {
        thread_.join ();
    }
}



void
SysThread::setDeleteOnExit (bool deleteOnExit)
{
#ifdef _VERBOSE
    Log::Debug ("SysThread::setDeleteOnExit invoked.");
#endif

    deleteOnExit_ = deleteOnExit;
}



bool
SysThread::run ()
{
#ifdef _VERBOSE
    Log::Debug ("SysThread::run invoked.");
#endif

    if (created_) {
        Log::Error ("Duplicate call to SysThread::run.  Ignoring.");
        return false;
    }


    // Launch the thread via std::thread.
    //
    thread_ = std::thread ([this] { threadEntry (this); });

    threadId_ = thread_.get_id ();

    // Only detach when deleteOnExit_ is set, because the thread
    // will delete the object itself and cannot be joined.
    // Otherwise keep the thread joinable for clean shutdown.
    //
    if (deleteOnExit_) {
        thread_.detach ();
    }

    created_ = true;

    ThreadUtils::addThread (threadId_);

    running_ = true;
    waitLock_.release ();


    return true;
}



bool
SysThread::stop ()
{
#ifdef _VERBOSE
    Log::Debug ("SysThread::stop invoked.");
#endif

    // Flag-based stop: the thread should check running_ and exit.
    // This replaces the previous pthread_cancel approach.
    //
    running_ = false;

    // Join the thread so it completes cleanly before we return.
    //
    if (thread_.joinable ()) {
        thread_.join ();
    }

    created_ = false;

    ThreadUtils::deleteThread (threadId_);

    return true;
}



bool
SysThread::isActive ()
{
#ifdef _VERBOSE
    Log::Debug ("SysThread::isActive invoked.");
#endif

    return running_;
}



t_ThreadId
SysThread::getThreadId ()
{
#ifdef _VERBOSE
    Log::Debug ("SysThread::getThreadId invoked.");
#endif

    return threadId_;
}



bool
SysThread::shouldContinue () const
{
    return running_.load ();
}



void
SysThread::threadEntry (SysThread * self)
{
#ifdef _VERBOSE
    Log::Debug ("SysThread::threadEntry invoked.");
#endif

    // Pause for initial resume
    //
    self->waitLock_.acquire ();
    self->waitLock_.release ();


#ifdef _VERBOSE
    pid_t  pid;
    pid = getpid ();

    Log::Debug ("++> Thread started with ID: "s +
                threadIdToString (self->threadId_) +
                " - Process ID: "s + std::to_string (pid));
#endif


    // Invoke derived threadMain ()...
    //
    self->threadMain ();

    self->running_ = false;

    ThreadUtils::deleteThread (self->threadId_);

    // at this point, the thread will exit.  If deleteOnExit is true,
    // delete the thread object associated with this thread.
    //
    if (self->deleteOnExit_) {
#ifdef _VERBOSE
        Log::Debug ("deleteOnExit true in thread object.  Deleting object.");
#endif
        delete (self);
    }
}


