/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <Common.h>
#include <iostream>
#include <Log.h>
#include <StringUtils.h>
#include <TestThread.h>
#include <list>
#include <unistd.h>


int 
main (int argc, char *argv[])
{
    string logFilename;
    int debugLevel;

    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <debug filename> <debugLevel 1-4> <thread count>" << std::endl;
        return 1;
    }
    logFilename = argv[1];
    debugLevel = atoi (argv[2]);

    if (debugLevel == 1) {
        Log::initialize (logFilename, Log::t_LogLevel::Silent);
    }
    else if (debugLevel == 2) {
        Log::initialize (logFilename, Log::t_LogLevel::Error);
    }
    else if (debugLevel == 3) {
        Log::initialize (logFilename, Log::t_LogLevel::Info);
    }
    else if (debugLevel == 4) {
        Log::initialize (logFilename, Log::t_LogLevel::Debug);
    }
    else {
        std::cout << "Invalid log level." << std::endl;
        return 1;
    }
    int threadCount;
    threadCount = atol(argv[3]);



    Log::Info ("Starting thread test."s +
               "\nThreads to Create: "s + std::to_string (threadCount));

    
    using t_ThreadList = list<TestThread *>;

    t_ThreadList  threadList;
    TestThread *  currThread;

    Log::Debug ("Creating thread objects...");

    int i;
    for (i = 0; i < threadCount; i++) {

        string currMsg = "This is thread number: "s + std::to_string(i+1);
        currThread = new TestThread (currMsg);
        currThread->setDeleteOnExit (true);

        threadList.push_back (currThread);
    }

    Log::Debug ("Starting threads...");

       
    bool status;
    i = 1;

    for (auto& item : threadList) {
        status = item->run ();

        if (!status) {
            Log::Error ("Start failed for thread "s + std::to_string(i));
            return false;
        }

        i++;
    }
 
    Log::Debug ("Finished.  Pausing in main thread.");

    sleep (3600);

 
    return 0;
}


