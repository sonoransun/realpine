///////
///
///  Copyright (C) 2026  sonoransun
///
///  Permission is hereby granted, free of charge, to any person obtaining a copy
///  of this software and associated documentation files (the "Software"), to deal
///  in the Software without restriction, including without limitation the rights
///  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
///  copies of the Software, and to permit persons to whom the Software is
///  furnished to do so, subject to the following conditions:
///
///  The above copyright notice and this permission notice shall be included in all
///  copies or substantial portions of the Software.
///
///  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
///  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
///  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
///  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
///  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
///  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
///  SOFTWARE.
///
///////


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

    // If the thread is still joinable, detach it to avoid
    // std::jthread destructor calling request_stop + join
    // on a potentially blocked thread during destruction.
    //
    if (thread_.joinable ()) {
        thread_.detach ();
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

    // Detach so the thread runs independently, matching original
    // pthread_detach behavior.
    //
    thread_.detach ();

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


