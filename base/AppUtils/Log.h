/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <string>
#include <string_view>


class Log
{
  public:

    enum class t_LogLevel { Silent, Error, Info, Debug };

    static bool initialize (const std::string & logFileName,
                            t_LogLevel          logLevel);

    static void setLogLevel (t_LogLevel logLevel);

    static bool stringToLogLevel (std::string_view  logLevelStr,
                                  t_LogLevel &      logLevel);

    static void Error (std::string_view logMsg);

    static void Info (std::string_view logMsg);

    static void Debug (std::string_view logMsg);


  private:

    static t_LogLevel logLevel_s;

};
