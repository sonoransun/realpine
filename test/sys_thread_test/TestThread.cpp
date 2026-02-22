/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <TestThread.h>
#include <Log.h>
#include <unistd.h>


TestThread::TestThread (const string  message)
{
#ifdef _VERBOSE
    Log::Debug ("TestThread constructor invoked.");
#endif

    msg_ = message;
}



TestThread::~TestThread ()
{
#ifdef _VERBOSE
    Log::Debug ("TestThread destructor invoked.");
#endif
}



void 
TestThread::threadMain ()
{
#ifdef _VERBOSE
    Log::Debug ("TestThread::threadMain invoked.");
#endif

    Log::Info (msg_);

    sleep (10);
}


