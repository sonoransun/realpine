/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <string>
#include <string_view>


class StringUtils
{
  public:
    static string sanitizeForLog(std::string_view input);
};
