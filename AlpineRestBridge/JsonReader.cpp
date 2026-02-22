/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <JsonReader.h>
#include <cstdlib>


JsonReader::JsonReader (const string & json)
    : json_(json)
{
    if (json_.length() > 65536)
        json_.resize(65536);
}


// Dtor defaulted in header


void
JsonReader::skipWhitespace (const string & s, ulong & pos)
{
    while (pos < s.length() &&
           (s[pos] == ' ' || s[pos] == '\t' || s[pos] == '\n' || s[pos] == '\r'))
    {
        ++pos;
    }
}


bool
JsonReader::findKey (const string & key, ulong & valuePos)
{
    // Build the search token: "key"
    string token = "\"" + key + "\"";

    ulong pos = json_.find(token);
    if (pos == string::npos)
        return false;

    // Move past the key token
    pos += token.length();

    // Skip whitespace
    skipWhitespace(json_, pos);

    // Expect a colon
    if (pos >= json_.length() || json_[pos] != ':')
        return false;

    ++pos;

    // Skip whitespace after colon
    skipWhitespace(json_, pos);

    if (pos >= json_.length())
        return false;

    valuePos = pos;
    return true;
}


bool
JsonReader::getString (const string & key, string & value)
{
    ulong pos = 0;
    if (!findKey(key, pos))
        return false;

    if (json_[pos] != '"')
        return false;

    ++pos;  // skip opening quote

    value.clear();

    while (pos < json_.length() && json_[pos] != '"')
    {
        if (value.length() >= MAX_STRING_LENGTH)
            return false;

        if (json_[pos] == '\\' && (pos + 1) < json_.length())
        {
            ++pos;
            switch (json_[pos])
            {
                case '"':   value += '"';   break;
                case '\\':  value += '\\';  break;
                case 'n':   value += '\n';  break;
                case 'r':   value += '\r';  break;
                case 't':   value += '\t';  break;
                default:    value += json_[pos]; break;
            }
        }
        else
        {
            value += json_[pos];
        }
        ++pos;
    }

    return true;
}


bool
JsonReader::getUlong (const string & key, ulong & value)
{
    ulong pos = 0;
    if (!findKey(key, pos))
        return false;

    if (pos >= json_.length())
        return false;

    // Read digits
    if (json_[pos] < '0' || json_[pos] > '9')
        return false;

    string numStr;
    while (pos < json_.length() && json_[pos] >= '0' && json_[pos] <= '9')
    {
        numStr += json_[pos];
        ++pos;
    }

    value = strtoul(numStr.c_str(), 0, 10);
    return true;
}


bool
JsonReader::getBool (const string & key, bool & value)
{
    ulong pos = 0;
    if (!findKey(key, pos))
        return false;

    if (pos + 4 <= json_.length() && json_.substr(pos, 4) == "true")
    {
        value = true;
        return true;
    }

    if (pos + 5 <= json_.length() && json_.substr(pos, 5) == "false")
    {
        value = false;
        return true;
    }

    return false;
}
