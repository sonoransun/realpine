/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include "Log.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <sstream>


Log::t_LogLevel          Log::logLevel_s    = t_LogLevel::Debug;
bool                     Log::jsonFormat_s  = false;
thread_local std::string Log::correlationId_s;


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
    spdlog::error("{}", formatWithCorrelation(logMsg));
}


void
Log::Info (std::string_view logMsg)
{
    if (logLevel_s < t_LogLevel::Info)
        return;
    spdlog::info("{}", formatWithCorrelation(logMsg));
}


void
Log::Debug (std::string_view logMsg)
{
    if (logLevel_s < t_LogLevel::Debug)
        return;
    spdlog::debug("{}", formatWithCorrelation(logMsg));
}


void
Log::Error (std::string_view logMsg, t_KvPairs kvPairs)
{
    if (logLevel_s < t_LogLevel::Error)
        return;
    spdlog::error("{}", formatStructured(logMsg, kvPairs));
}


void
Log::Info (std::string_view logMsg, t_KvPairs kvPairs)
{
    if (logLevel_s < t_LogLevel::Info)
        return;
    spdlog::info("{}", formatStructured(logMsg, kvPairs));
}


void
Log::Debug (std::string_view logMsg, t_KvPairs kvPairs)
{
    if (logLevel_s < t_LogLevel::Debug)
        return;
    spdlog::debug("{}", formatStructured(logMsg, kvPairs));
}


void
Log::setCorrelationId (const std::string & id)
{
    correlationId_s = id;
}


void
Log::clearCorrelationId ()
{
    correlationId_s.clear();
}


void
Log::setJsonFormat (bool enable)
{
    jsonFormat_s = enable;
}


std::string
Log::formatStructured (std::string_view  logMsg,
                       t_KvPairs         kvPairs)
{
    if (jsonFormat_s) {
        std::ostringstream oss;
        oss << "{\"msg\":\"" << logMsg << "\"";
        if (!correlationId_s.empty())
            oss << ",\"correlation_id\":\"" << correlationId_s << "\"";
        for (const auto & [k, v] : kvPairs)
            oss << ",\"" << k << "\":\"" << v << "\"";
        oss << "}";
        return oss.str();
    }

    std::string result;
    if (!correlationId_s.empty())
        result += "[" + correlationId_s + "] ";
    result += logMsg;
    for (const auto & [k, v] : kvPairs) {
        result += " ";
        result += k;
        result += "=";
        result += v;
    }
    return result;
}


std::string
Log::formatWithCorrelation (std::string_view logMsg)
{
    if (jsonFormat_s) {
        std::ostringstream oss;
        oss << "{\"msg\":\"" << logMsg << "\"";
        if (!correlationId_s.empty())
            oss << ",\"correlation_id\":\"" << correlationId_s << "\"";
        oss << "}";
        return oss.str();
    }

    if (correlationId_s.empty())
        return std::string(logMsg);

    return "[" + correlationId_s + "] " + std::string(logMsg);
}
