/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AutoThread.h>
#include <ThreadUtils.h>
#include <Log.h>
#include <StringUtils.h>
#include <Platform.h>


AutoThread::AutoThread ()
{
#ifdef _VERBOSE
    Log::Debug ("AutoThread constructor invoked.");
#endif
}



AutoThread::~AutoThread ()
{
#ifdef _VERBOSE
    Log::Debug ("AutoThread destructor invoked.");
#endif

    // Signal the thread to exit if it is still running, then
    // detach so the jthread destructor does not block.
    //
    destroyed_ = true;

    {
        std::lock_guard<std::mutex> lk(cvMutex_);
        resumed_ = true;
    }
    cv_.notify_one ();

    if (thread_.joinable ()) {
        thread_.detach ();
    }
}



bool
AutoThread::create ()
{
#ifdef _VERBOSE
    Log::Debug ("AutoThread::create invoked.");
#endif

    if (created_) {
        Log::Error ("Duplicate call to AutoThread::create.  Ignoring.");
        return false;
    }


    // Launch the thread.  It will immediately wait on the
    // condition variable until resume() is called.
    //
    thread_ = std::thread ([this] { threadEntry (this); });

    threadId_ = thread_.get_id ();

    // Detach so the thread runs independently, matching original
    // pthread_detach behavior.
    //
    thread_.detach ();

    created_ = true;
    running_ = false;

    ThreadUtils::addThread (threadId_);


    return true;
}



bool
AutoThread::destroy ()
{
#ifdef _VERBOSE
    Log::Debug ("AutoThread::destroy invoked.");
#endif

    // Flag-based stop: set destroyed_ so the thread loop exits
    // on its next check.  This replaces the previous pthread_cancel
    // approach.
    //
    destroyed_ = true;

    // Wake the thread if it is waiting so it can observe the flag.
    //
    {
        std::lock_guard<std::mutex> lk(cvMutex_);
        resumed_ = true;
    }
    cv_.notify_one ();

    created_ = false;
    running_ = false;

    ThreadUtils::deleteThread (threadId_);


    return true;
}



bool
AutoThread::resume ()
{
#ifdef _VERBOSE
    Log::Debug ("AutoThread::resume invoked.");
#endif

    if (!created_) {
        return false;
    }
    if (running_) {
        return false;
    }

    // Signal the waiting thread to wake up.
    //
    {
        std::lock_guard<std::mutex> lk(cvMutex_);
        resumed_ = true;
    }
    cv_.notify_one ();


    return true;
}



bool
AutoThread::isActive ()
{
#ifdef _VERBOSE
    Log::Debug ("AutoThread::isActive invoked.");
#endif

    return running_;
}



bool
AutoThread::getThreadId (t_ThreadId & threadId)
{
#ifdef _VERBOSE
    Log::Debug ("AutoThread::getThreadId invoked.");
#endif

    if (!created_) {
        return false;
    }

    threadId = threadId_;


    return true;
}



void
AutoThread::threadEntry (AutoThread * self)
{
#ifdef _VERBOSE
    Log::Debug ("AutoThread::threadEntry invoked.");
#endif

    // Wait for initial resume before entering the main loop.
    //
    {
        std::unique_lock<std::mutex> lk(self->cvMutex_);
        self->cv_.wait (lk, [self] { return self->resumed_; });
        self->resumed_ = false;
    }

    if (self->destroyed_)
        return;


#ifdef _VERBOSE
    pid_t  pid;
    pid = getpid ();

    Log::Debug ("++> Thread started with ID: "s +
                threadIdToString (self->threadId_) +
                " - Process ID: "s + std::to_string (pid));
#endif


    while (true) {

        self->running_ = true;

        // Invoke derived threadMain ()...
        //
        self->threadMain ();

        // Suspend thread until next resume().
        self->running_ = false;

        // Check if we have been destroyed while running.
        //
        if (self->destroyed_) {
            break;
        }

        // Wait for the next resume() call.
        //
        {
            std::unique_lock<std::mutex> lk(self->cvMutex_);
            self->cv_.wait (lk, [self] { return self->resumed_; });
            self->resumed_ = false;
        }

        if (self->destroyed_) {
            break;
        }
    }


    ThreadUtils::deleteThread (self->threadId_);
}


