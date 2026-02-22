/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <GuiHelpers.h>
#include <chrono>
#include <ctime>
#include <cstdio>



void
addLog (AppState &            state,
        LogLevel              level,
        const std::string &   msg)
{
    auto now = std::chrono::system_clock::now();
    auto tt = std::chrono::system_clock::to_time_t(now);
    struct tm tmBuf;
    localtime_r(&tt, &tmBuf);

    char timeBuf[64];
    snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d:%02d",
             tmBuf.tm_hour, tmBuf.tm_min, tmBuf.tm_sec);

    state.logEntries.push_back({timeBuf, level, msg});

    // Keep log from growing unbounded
    if (state.logEntries.size() > 1000)
        state.logEntries.erase(state.logEntries.begin());
}



std::string
formatJson (const std::string &  json)
{
    std::string out;
    out.reserve(json.size() * 2);

    int indent = 0;
    bool inString = false;

    for (size_t i = 0; i < json.size(); ++i)
    {
        char c = json[i];

        if (inString)
        {
            out += c;
            if (c == '"' && (i == 0 || json[i - 1] != '\\'))
                inString = false;
            continue;
        }

        switch (c)
        {
            case '"':
                out += c;
                inString = true;
                break;

            case '{':
            case '[':
                out += c;
                out += '\n';
                ++indent;
                for (int j = 0; j < indent; ++j) out += "  ";
                break;

            case '}':
            case ']':
                out += '\n';
                --indent;
                for (int j = 0; j < indent; ++j) out += "  ";
                out += c;
                break;

            case ',':
                out += c;
                out += '\n';
                for (int j = 0; j < indent; ++j) out += "  ";
                break;

            case ':':
                out += ": ";
                break;

            default:
                if (c != ' ' && c != '\t' && c != '\n' && c != '\r')
                    out += c;
                break;
        }
    }

    return out;
}



void
parseJsonArray (const std::string &         json,
                std::vector<std::string> &  items)
{
    items.clear();

    // Find the array boundaries
    size_t arrStart = json.find('[');
    if (arrStart == std::string::npos)
    {
        // Not an array — treat the whole thing as a single item
        if (!json.empty())
            items.push_back(json);
        return;
    }

    size_t arrEnd = json.rfind(']');
    if (arrEnd == std::string::npos || arrEnd <= arrStart)
        return;

    // Walk through and split on top-level objects
    int depth = 0;
    size_t objStart = 0;
    bool inString = false;

    for (size_t i = arrStart + 1; i < arrEnd; ++i)
    {
        char c = json[i];

        if (inString)
        {
            if (c == '"' && json[i - 1] != '\\')
                inString = false;
            continue;
        }

        if (c == '"')
        {
            inString = true;
            continue;
        }

        if (c == '{' || c == '[')
        {
            if (depth == 0)
                objStart = i;
            ++depth;
        }
        else if (c == '}' || c == ']')
        {
            --depth;
            if (depth == 0)
                items.push_back(json.substr(objStart, i - objStart + 1));
        }
    }
}
