/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <JsonReader.h>
#include <Log.h>


JsonReader::JsonReader(const string & json)
    : valid_(false)
{
    if (json.length() > 65536) {
        Log::Error("JsonReader: input exceeds maximum size");
        return;
    }
    try {
        doc_ = nlohmann::json::parse(json);
        valid_ = doc_.is_object();
    } catch (const nlohmann::json::parse_error & e) {
        Log::Error("JsonReader: parse error: "s + e.what());
        return;
    }

    if (valid_ && !checkDepthAndKeys(doc_, 0)) {
        Log::Error("JsonReader: document exceeds nesting depth or key count limits");
        doc_ = nlohmann::json();
        valid_ = false;
    }
}


bool
JsonReader::checkDepthAndKeys(const nlohmann::json & node, int depth)
{
    if (depth > MAX_NESTING_DEPTH)
        return false;

    if (node.is_object()) {
        if (static_cast<int>(node.size()) > MAX_KEYS_PER_OBJECT)
            return false;
        for (const auto & [key, val] : node.items()) {
            if (!checkDepthAndKeys(val, depth + 1))
                return false;
        }
    } else if (node.is_array()) {
        for (const auto & elem : node) {
            if (!checkDepthAndKeys(elem, depth + 1))
                return false;
        }
    }

    return true;
}


bool
JsonReader::getString(const string & key, string & value)
{
    if (!valid_)
        return false;
    auto it = doc_.find(key);
    if (it == doc_.end() || !it->is_string())
        return false;
    value = it->get<string>();
    if (value.length() > MAX_STRING_LENGTH)
        value.resize(MAX_STRING_LENGTH);
    return true;
}


bool
JsonReader::getUlong(const string & key, ulong & value)
{
    if (!valid_)
        return false;
    auto it = doc_.find(key);
    if (it == doc_.end() || !it->is_number_unsigned())
        return false;
    value = it->get<ulong>();
    return true;
}


bool
JsonReader::getBool(const string & key, bool & value)
{
    if (!valid_)
        return false;
    auto it = doc_.find(key);
    if (it == doc_.end() || !it->is_boolean())
        return false;
    value = it->get<bool>();
    return true;
}
