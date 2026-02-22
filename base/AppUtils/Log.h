/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <string>
#include <string_view>
#include <memory>

#ifdef _CONCURRENT
#include <Mutex.h>
#endif


#include <fstream>


class Log
{
  public:

    Log () = default;
    ~Log () = default;


    enum class t_LogLevel { Silent, Error, Info, Debug };


    static bool initialize (const string &  logFileName,
                            t_LogLevel      logLevel);

    static void setLogLevel (t_LogLevel  logLevel);

    static bool stringToLogLevel (std::string_view  logLevelStr,
                                  t_LogLevel &      logLevel);

    static void Error (std::string_view  logMsg);

    static void Info (std::string_view  logMsg);

    static void Debug (std::string_view  logMsg);


  private:

    static string  timestamp ();

    static void  write (std::string_view  logMsg);


    static std::unique_ptr<std::ofstream>  logFile_s;
    static t_LogLevel    logLevel_s;
    static string        logFileName_s;

#ifdef _CONCURRENT
    static Mutex         syncLock_s;
#endif

};

