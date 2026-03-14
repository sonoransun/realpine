/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <StringUtils.h>
#include <cstdio>


string StringUtils::sanitizeForLog(std::string_view input)
{
    string result;
    result.reserve(input.size());

    for (auto ch : input)
    {
        auto uch = static_cast<uchar>(ch);

        if ((uch >= 0x20 && uch != 0x7F) || ch == '\n' || ch == '\r' || ch == '\t') {
            result += ch;
        } else {
            char buf[7];
            static_cast<void>(std::snprintf(buf, sizeof(buf), "<0x%02X>", uch));
            result += buf;
        }
    }

    return result;
}

