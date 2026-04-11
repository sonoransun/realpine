/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AutoThread.h>
#include <Log.h>
#include <Platform.h>
#include <StringUtils.h>
#include <ThreadUtils.h>


AutoThread::AutoThread()
{
#ifdef _VERBOSE
    Log::Debug("AutoThread constructor invoked.");
#endif
}


AutoThread::~AutoThread()
{
#ifdef _VERBOSE
    Log::Debug("AutoThread destructor invoked.");
#endif

    // Signal the thread to exit if it is still running.
    // The jthread destructor will call request_stop() + join()
    // automatically, but we also set our own flags and wake the
    // CV so the thread loop observes the stop promptly.
    //
    destroyed_ = true;

    {
        std::lock_guard<std::mutex> lk(cvMutex_);
        resumed_ = true;
    }
    cv_.notify_one();

    // request_stop() ensures the stop_token is signalled before
    // the jthread destructor joins.
    //
    if (thread_.joinable()) {
        thread_.request_stop();
    }
    // jthread destructor handles the join.
}


void
AutoThread::threadMain(std::stop_token /* stopToken */)
{
    // Default implementation: delegate to the legacy threadMain().
    // Subclasses that want cooperative cancellation should override
    // this overload and check stopToken.stop_requested() directly.
    //
    threadMain();
}


bool
AutoThread::create()
{
#ifdef _VERBOSE
    Log::Debug("AutoThread::create invoked.");
#endif

    if (created_) {
        Log::Error("Duplicate call to AutoThread::create.  Ignoring.");
        return false;
    }


    // Launch the thread via std::jthread.  The jthread constructor
    // passes the stop_token as the first argument to the callable.
    //
    thread_ = std::jthread([this](std::stop_token st) { threadEntry(std::move(st), this); });

    threadId_ = thread_.get_id();

    created_ = true;
    running_ = false;

    ThreadUtils::addThread(threadId_);


    return true;
}


bool
AutoThread::destroy()
{
#ifdef _VERBOSE
    Log::Debug("AutoThread::destroy invoked.");
#endif

    // Flag-based stop: set destroyed_ so the thread loop exits
    // on its next check.
    //
    destroyed_ = true;

    // Request cooperative cancellation via the stop token.
    //
    thread_.request_stop();

    // Wake the thread if it is waiting so it can observe the stop.
    //
    {
        std::lock_guard<std::mutex> lk(cvMutex_);
        resumed_ = true;
    }
    cv_.notify_one();

    // Join the thread so it completes cleanly before we return.
    //
    if (thread_.joinable()) {
        thread_.join();
    }

    created_ = false;
    running_ = false;

    ThreadUtils::deleteThread(threadId_);


    return true;
}


bool
AutoThread::resume()
{
#ifdef _VERBOSE
    Log::Debug("AutoThread::resume invoked.");
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
    cv_.notify_one();


    return true;
}


bool
AutoThread::isActive()
{
#ifdef _VERBOSE
    Log::Debug("AutoThread::isActive invoked.");
#endif

    return running_;
}


bool
AutoThread::getThreadId(t_ThreadId & threadId)
{
#ifdef _VERBOSE
    Log::Debug("AutoThread::getThreadId invoked.");
#endif

    if (!created_) {
        return false;
    }

    threadId = threadId_;


    return true;
}


void
AutoThread::threadEntry(std::stop_token stopToken, AutoThread * self)
{
#ifdef _VERBOSE
    Log::Debug("AutoThread::threadEntry invoked.");
#endif

    // Wait for initial resume before entering the main loop.
    //
    {
        std::unique_lock<std::mutex> lk(self->cvMutex_);
        self->cv_.wait(lk, [self, &stopToken] { return self->resumed_ || stopToken.stop_requested(); });
        self->resumed_ = false;
    }

    if (self->destroyed_ || stopToken.stop_requested())
        return;


#ifdef _VERBOSE
    pid_t pid;
    pid = getpid();

    Log::Debug("++> Thread started with ID: "s + threadIdToString(self->threadId_) + " - Process ID: "s +
               std::to_string(pid));
#endif


    while (true) {

        self->running_ = true;

        // Invoke derived threadMain (stop_token)...
        //
        self->threadMain(stopToken);

        // Suspend thread until next resume().
        self->running_ = false;

        // Check if we have been destroyed or stop requested.
        //
        if (self->destroyed_ || stopToken.stop_requested()) {
            break;
        }

        // Wait for the next resume() call.
        //
        {
            std::unique_lock<std::mutex> lk(self->cvMutex_);
            self->cv_.wait(lk, [self, &stopToken] { return self->resumed_ || stopToken.stop_requested(); });
            self->resumed_ = false;
        }

        if (self->destroyed_ || stopToken.stop_requested()) {
            break;
        }
    }


    ThreadUtils::deleteThread(self->threadId_);
}
