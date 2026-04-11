/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <unistd.h>


namespace TermColor {

inline string
red(const string & s)
{
    return "\033[31m"s + s + "\033[0m"s;
}
inline string
green(const string & s)
{
    return "\033[32m"s + s + "\033[0m"s;
}
inline string
yellow(const string & s)
{
    return "\033[33m"s + s + "\033[0m"s;
}
inline string
bold(const string & s)
{
    return "\033[1m"s + s + "\033[0m"s;
}

inline bool
isTerminal()
{
    return isatty(STDOUT_FILENO) != 0;
}

}  // namespace TermColor
