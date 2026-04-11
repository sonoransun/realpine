/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <sstream>
#include <thread>

using t_ThreadId = std::thread::id;


// Utility to convert std::thread::id to string for logging.
// std::thread::id does not support std::to_string, so we stream it instead.
//
inline std::string
threadIdToString(const t_ThreadId & id)
{
    std::ostringstream oss;
    oss << id;
    return oss.str();
}
