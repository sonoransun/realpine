/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <string>
#include <string_view>
#include <format>
#include <initializer_list>
#include <utility>


class Log
{
  public:

    using t_KvPair  = std::pair<std::string_view, std::string_view>;
    using t_KvPairs = std::initializer_list<t_KvPair>;

    enum class t_LogLevel { Silent, Error, Info, Debug };

    static bool initialize (const std::string & logFileName,
                            t_LogLevel          logLevel);

    static void setLogLevel (t_LogLevel logLevel);

    static t_LogLevel getLogLevel ();

    static std::string_view logLevelToString (t_LogLevel logLevel);

    static bool stringToLogLevel (std::string_view  logLevelStr,
                                  t_LogLevel &      logLevel);

    static void Error (std::string_view logMsg);

    static void Info (std::string_view logMsg);

    static void Debug (std::string_view logMsg);

    static void Error (std::string_view logMsg, t_KvPairs kvPairs);

    static void Info (std::string_view logMsg, t_KvPairs kvPairs);

    static void Debug (std::string_view logMsg, t_KvPairs kvPairs);

    // std::format-based overloads — prefer these over string concatenation
    template <typename... Args>
    static void Error (std::format_string<Args...> fmt, Args &&... args)
    {
        Error(std::format(fmt, std::forward<Args>(args)...));
    }

    template <typename... Args>
    static void Info (std::format_string<Args...> fmt, Args &&... args)
    {
        Info(std::format(fmt, std::forward<Args>(args)...));
    }

    template <typename... Args>
    static void Debug (std::format_string<Args...> fmt, Args &&... args)
    {
        Debug(std::format(fmt, std::forward<Args>(args)...));
    }

    static void setCorrelationId (const std::string & id);

    static void clearCorrelationId ();

    static void setJsonFormat (bool enable);


  private:

    static t_LogLevel    logLevel_s;
    static bool          jsonFormat_s;

    static thread_local std::string  correlationId_s;

    static std::string  formatStructured (std::string_view  logMsg,
                                          t_KvPairs         kvPairs);

    static std::string  formatWithCorrelation (std::string_view logMsg);

};
