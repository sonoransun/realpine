/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <JsonWriter.h>


JsonWriter::JsonWriter ()
    : afterKey_(false)
{
    buffer_.reserve(256);
}


string
JsonWriter::result ()
{
    return buffer_;
}


void
JsonWriter::beginObject ()
{
    if (!afterKey_)
        appendCommaIfNeeded();

    afterKey_ = false;
    buffer_ += '{';
    stack_.push_back({false, false});
}


void
JsonWriter::endObject ()
{
    if (!stack_.empty())
        stack_.pop_back();

    buffer_ += '}';

    // The completed object counts as an element in its parent container
    if (!stack_.empty())
        stack_.back().needsComma = true;
}


void
JsonWriter::beginArray ()
{
    if (!afterKey_)
        appendCommaIfNeeded();

    afterKey_ = false;
    buffer_ += '[';
    stack_.push_back({true, false});
}


void
JsonWriter::endArray ()
{
    if (!stack_.empty())
        stack_.pop_back();

    buffer_ += ']';

    // The completed array counts as an element in its parent container
    if (!stack_.empty())
        stack_.back().needsComma = true;
}


void
JsonWriter::key (const string & k)
{
    appendCommaIfNeeded();
    buffer_ += '"';
    buffer_ += escapeJsonString(k);
    buffer_ += "\":"s;
    afterKey_ = true;
}


void
JsonWriter::value (const string & v)
{
    if (!afterKey_)
        appendCommaIfNeeded();

    afterKey_ = false;
    buffer_ += '"';
    buffer_ += escapeJsonString(v);
    buffer_ += '"';

    if (!stack_.empty())
        stack_.back().needsComma = true;
}


void
JsonWriter::value (ulong v)
{
    if (!afterKey_)
        appendCommaIfNeeded();

    afterKey_ = false;
    buffer_ += std::to_string(v);

    if (!stack_.empty())
        stack_.back().needsComma = true;
}


void
JsonWriter::value (bool v)
{
    if (!afterKey_)
        appendCommaIfNeeded();

    afterKey_ = false;
    buffer_ += v ? "true"s : "false"s;

    if (!stack_.empty())
        stack_.back().needsComma = true;
}


void
JsonWriter::separator ()
{
    // no-op: commas are handled automatically
}


void
JsonWriter::appendCommaIfNeeded ()
{
    if (!stack_.empty() && stack_.back().needsComma)
        buffer_ += ',';
}


string
JsonWriter::escapeJsonString (std::string_view sv)
{
    string escaped;
    escaped.reserve(sv.size());

    for (char c : sv)
    {
        switch (c)
        {
            case '"':   escaped += "\\\""s;  break;
            case '\\':  escaped += "\\\\"s;  break;
            case '\n':  escaped += "\\n"s;   break;
            case '\r':  escaped += "\\r"s;   break;
            case '\t':  escaped += "\\t"s;   break;
            case '\b':  escaped += "\\b"s;   break;
            case '\f':  escaped += "\\f"s;   break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    // Control characters as \u00XX
                    static const char hexDigits[] = "0123456789abcdef";
                    escaped += "\\u00"s;
                    escaped += hexDigits[(static_cast<unsigned char>(c) >> 4) & 0x0F];
                    escaped += hexDigits[static_cast<unsigned char>(c) & 0x0F];
                } else {
                    escaped += c;
                }
                break;
        }
    }

    return escaped;
}
