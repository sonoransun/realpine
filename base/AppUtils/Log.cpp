/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include "Log.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sink.h>


Log::t_LogLevel Log::logLevel_s = t_LogLevel::Debug;


bool
Log::initialize (const std::string & logFileName,
                 t_LogLevel          logLevel)
{
    try {
        auto fileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            logFileName, 10 * 1024 * 1024, 3);
        auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

        auto logger = std::make_shared<spdlog::logger>("alpine",
            spdlog::sinks_init_list{fileSink, consoleSink});
        logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%t] [%l] %v");
        logger->flush_on(spdlog::level::err);

        spdlog::set_default_logger(logger);
        setLogLevel(logLevel);
        spdlog::info("Log initialized: {}", logFileName);
        return true;
    } catch (const spdlog::spdlog_ex &) {
        return false;
    }
}


void
Log::setLogLevel (t_LogLevel logLevel)
{
    logLevel_s = logLevel;
    switch (logLevel) {
        case t_LogLevel::Silent: spdlog::set_level(spdlog::level::off);   break;
        case t_LogLevel::Error:  spdlog::set_level(spdlog::level::err);   break;
        case t_LogLevel::Info:   spdlog::set_level(spdlog::level::info);  break;
        case t_LogLevel::Debug:  spdlog::set_level(spdlog::level::debug); break;
    }
}


bool
Log::stringToLogLevel (std::string_view  logLevelStr,
                       t_LogLevel &      logLevel)
{
    if (logLevelStr == "silent" || logLevelStr == "Silent") {
        logLevel = t_LogLevel::Silent;
    } else if (logLevelStr == "error" || logLevelStr == "Error") {
        logLevel = t_LogLevel::Error;
    } else if (logLevelStr == "info" || logLevelStr == "Info") {
        logLevel = t_LogLevel::Info;
    } else if (logLevelStr == "debug" || logLevelStr == "Debug") {
        logLevel = t_LogLevel::Debug;
    } else {
        return false;
    }
    return true;
}


void
Log::Error (std::string_view logMsg)
{
    if (logLevel_s < t_LogLevel::Error)
        return;
    spdlog::error("{}", logMsg);
}


void
Log::Info (std::string_view logMsg)
{
    if (logLevel_s < t_LogLevel::Info)
        return;
    spdlog::info("{}", logMsg);
}


void
Log::Debug (std::string_view logMsg)
{
    if (logLevel_s < t_LogLevel::Debug)
        return;
    spdlog::debug("{}", logMsg);
}
