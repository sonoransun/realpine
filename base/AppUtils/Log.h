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

