///////
///
///  Copyright (C) 2026  sonoransun
///
///  Permission is hereby granted, free of charge, to any person obtaining a copy
///  of this software and associated documentation files (the "Software"), to deal
///  in the Software without restriction, including without limitation the rights
///  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
///  copies of the Software, and to permit persons to whom the Software is
///  furnished to do so, subject to the following conditions:
///
///  The above copyright notice and this permission notice shall be included in all
///  copies or substantial portions of the Software.
///
///  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
///  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
///  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
///  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
///  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
///  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
///  SOFTWARE.
///
///////


#include <JsonReader.h>
#include <cstdlib>


JsonReader::JsonReader (const string & json)
    : json_(json)
{
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
