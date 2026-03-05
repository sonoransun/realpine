/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <charconv>
#include <optional>


inline std::optional<ulong> parseUlong (const string & s)
{
    if (s.empty()) return std::nullopt;
    ulong value = 0;
    auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), value);
    if (ec != std::errc{} || ptr != s.data() + s.size())
        return std::nullopt;
    return value;
}


inline std::optional<ushort> parseUshort (const string & s)
{
    auto result = parseUlong(s);
    if (!result || *result > 65535)
        return std::nullopt;
    return static_cast<ushort>(*result);
}


inline std::optional<int> parseInt (const string & s)
{
    if (s.empty()) return std::nullopt;
    int value = 0;
    auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), value);
    if (ec != std::errc{} || ptr != s.data() + s.size())
        return std::nullopt;
    return value;
}
