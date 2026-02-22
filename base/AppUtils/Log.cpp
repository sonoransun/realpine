/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <Platform.h>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <Log.h>

#ifdef _CONCURRENT
#include <ThreadUtils.h>
#include <MutexLock.h>
#endif


std::unique_ptr<std::ofstream>  Log::logFile_s;
Log::t_LogLevel       Log::logLevel_s = Log::t_LogLevel::Debug;
string                Log::logFileName_s;

#ifdef _CONCURRENT
Mutex                 Log::syncLock_s;
#endif


const char * dayArray[] =
    { "Sun", "Mon", "Tue", "Wed", "Thr", "Fri", "Sat" };

const char * monthArray[] =
    { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };



// Ctor/dtor defaulted in header



bool 
Log::initialize (const string &  logFileName,
                 t_LogLevel      logLevel)
{
    logFile_s = std::make_unique<std::ofstream>(logFileName, std::ios::app);
    logLevel_s = logLevel;
    logFileName_s = logFileName;

    Log::write ("\n"
                "========================================\n"
                ">> Application Started\n   "s +
                    Log::timestamp() +
                "\n========================================\n"s);

    return true;
}



void 
Log::setLogLevel (t_LogLevel  logLevel)
{
    logLevel_s = logLevel;
}



bool
Log::stringToLogLevel (std::string_view  logLevelStr,
                       t_LogLevel &      logLevel)
{
    bool  isValid = false;


    if ( (logLevelStr == "Silent") ||
         (logLevelStr == "silent")  ) {
        isValid  = true;
        logLevel = t_LogLevel::Silent;
    }
    else if ( (logLevelStr == "Error") ||
              (logLevelStr == "error")  ) {
        isValid  = true;
        logLevel = t_LogLevel::Error;
    }
    else if ( (logLevelStr == "Info") ||
              (logLevelStr == "info")  ) {
        isValid  = true;
        logLevel = t_LogLevel::Info;
    }
    else if ( (logLevelStr == "Debug") ||
              (logLevelStr == "debug")  ) {
        isValid  = true;
        logLevel = t_LogLevel::Debug;
    }


    return isValid;
}



void
Log::Error (std::string_view  logMsg)
{
    if (logLevel_s == t_LogLevel::Silent)
        return;

    Log::write (Log::timestamp() + " Error: " + std::string(logMsg));
}



void
Log::Info (std::string_view  logMsg)
{
    if ( (logLevel_s != t_LogLevel::Info) &&
         (logLevel_s != t_LogLevel::Debug) )
        return;

    Log::write (Log::timestamp() + "  Info: " + std::string(logMsg));
}



void
Log::Debug (std::string_view  logMsg)
{
    if (logLevel_s != t_LogLevel::Debug)
        return;

    Log::write (Log::timestamp() + " Debug: " + std::string(logMsg));
}



string
Log::timestamp ()
{
    const int   buffMax = 128;
    char        tmpBuff[buffMax];
    struct tm   today;
    time_t      now;
    struct timeval  hrTime;

    gettimeofday (&hrTime, 0);

    time(&now);
    localtime_r (&now, &today);

#ifndef _CONCURRENT
    snprintf (tmpBuff, buffMax, "[%s %s %2.2d %2.2d:%2.2d:%2.2d-%6.6lu]",
              dayArray[(today.tm_wday)], monthArray[(today.tm_mon)],
              today.tm_mday,
              today.tm_hour, today.tm_min, today.tm_sec, static_cast<unsigned long>(hrTime.tv_usec));
#else
    // if multi-threaded, add thread index to output...
    //
    ulong  threadInd;
    threadInd = ThreadUtils::getThreadIndex ();

    char pad[] = "  ";
    if (threadInd > 99)
        pad[0] = 0;
    else if (threadInd > 9)
        pad[1] = 0;

    snprintf (tmpBuff, buffMax, "[(t%s%lu)  %s %s %2.2d %2.2d:%2.2d:%2.2d-%6.6lu]",
              pad, threadInd,
              dayArray[(today.tm_wday)], monthArray[(today.tm_mon)],
              today.tm_mday,
              today.tm_hour, today.tm_min, today.tm_sec, static_cast<unsigned long>(hrTime.tv_usec));
#endif


    return std::string(tmpBuff);
}



void
Log::write (std::string_view  logMsg)
{
#ifdef _CONCURRENT
    MutexLock  lock (syncLock_s);
#endif

    if (logFile_s) {
        *logFile_s << logMsg << std::endl;
    }
    else {
        // Not initialized yet, output to stderr...
        //
        std::cerr << logMsg << std::endl;
    }
}



