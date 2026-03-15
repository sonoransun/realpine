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

    // Signal the thread to stop via the jthread stop token, then
    // allow the jthread destructor to request_stop + join.
    //
    running_ = false;

    if (deleteOnExit_) {
        // When deleteOnExit_ is true the thread deletes `this`,
        // so we must detach to avoid the jthread destructor
        // joining a thread that references a destroyed object.
        //
        if (thread_.joinable ()) {
            thread_.request_stop ();
            thread_.detach ();
        }
    } else {
        // jthread destructor will call request_stop() + join()
        // automatically.  Explicit request_stop() here ensures
        // the stop is signalled even before the destructor body
        // finishes, so shouldContinue() returns false immediately.
        //
        if (thread_.joinable ()) {
            thread_.request_stop ();
        }
    }
}



void
SysThread::threadMain (std::stop_token /* stopToken */)
{
    // Default implementation: delegate to the legacy threadMain().
    // Subclasses that want cooperative cancellation should override
    // this overload and check stopToken.stop_requested() directly.
    //
    threadMain ();
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


    // Launch the thread via std::jthread.  The jthread constructor
    // passes the stop_token as the first argument to the callable.
    //
    thread_ = std::jthread ([this] (std::stop_token st) {
        threadEntry (std::move (st), this);
    });

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

    // Request cooperative cancellation via the stop token,
    // then set the legacy running_ flag for backward compat.
    //
    thread_.request_stop ();
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
    return running_.load () && !stopToken_.stop_requested ();
}



void
SysThread::threadEntry (std::stop_token stopToken, SysThread * self)
{
#ifdef _VERBOSE
    Log::Debug ("SysThread::threadEntry invoked.");
#endif

    // Cache the stop token so shouldContinue() can inspect it.
    self->stopToken_ = stopToken;

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


    // Invoke derived threadMain (stop_token) overload, which by
    // default calls the legacy threadMain().
    //
    self->threadMain (stopToken);

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


