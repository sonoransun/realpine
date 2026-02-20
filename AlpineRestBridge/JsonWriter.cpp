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


#include <JsonWriter.h>
#include <cstdio>


JsonWriter::JsonWriter ()
    : needsComma_(false)
{
}


// Dtor defaulted in header


string
JsonWriter::result ()
{
    return buffer_;
}


void
JsonWriter::beginObject ()
{
    if (needsComma_)
        buffer_ += ",";

    buffer_ += "{";
    needsComma_ = false;
}


void
JsonWriter::endObject ()
{
    buffer_ += "}";
    needsComma_ = true;
}


void
JsonWriter::beginArray ()
{
    if (needsComma_)
        buffer_ += ",";

    buffer_ += "[";
    needsComma_ = false;
}


void
JsonWriter::endArray ()
{
    buffer_ += "]";
    needsComma_ = true;
}


void
JsonWriter::key (const string & k)
{
    if (needsComma_)
        buffer_ += ",";

    buffer_ += "\"";
    buffer_ += escape(k);
    buffer_ += "\":";
    needsComma_ = false;
}


void
JsonWriter::value (const string & v)
{
    if (needsComma_)
        buffer_ += ",";

    buffer_ += "\"";
    buffer_ += escape(v);
    buffer_ += "\"";
    needsComma_ = true;
}


void
JsonWriter::value (ulong v)
{
    if (needsComma_)
        buffer_ += ",";

    char numBuf[32];
    snprintf(numBuf, sizeof(numBuf), "%lu", v);
    buffer_ += numBuf;
    needsComma_ = true;
}


void
JsonWriter::value (bool v)
{
    if (needsComma_)
        buffer_ += ",";

    buffer_ += (v ? "true" : "false");
    needsComma_ = true;
}


void
JsonWriter::separator ()
{
    buffer_ += ",";
    needsComma_ = false;
}


string
JsonWriter::escape (const string & s)
{
    string out;
    out.reserve(s.length());

    for (ulong i = 0; i < s.length(); ++i)
    {
        char c = s[i];

        switch (c)
        {
            case '"':   out += "\\\"";  break;
            case '\\':  out += "\\\\";  break;
            case '\n':  out += "\\n";   break;
            case '\r':  out += "\\r";   break;
            case '\t':  out += "\\t";   break;
            default:
                if (c >= 0 && c < 0x20)
                {
                    char hex[8];
                    snprintf(hex, sizeof(hex), "\\u%04x", (uint)(uchar)c);
                    out += hex;
                }
                else
                {
                    out += c;
                }
                break;
        }
    }

    return out;
}
