/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <string_view>


class JsonWriter
{
  public:
    JsonWriter();
    ~JsonWriter() = default;

    string result();

    void beginObject();
    void endObject();

    void beginArray();
    void endArray();

    void key(const string & k);

    void value(const string & v);
    void value(ulong v);
    void value(bool v);

    void separator();  // no-op — commas are handled automatically


  private:
    struct ContainerState
    {
        bool isArray;     // true = array, false = object
        bool needsComma;  // true if next key (object) or value (array) needs a leading comma
    };

    void appendCommaIfNeeded();
    string escapeJsonString(std::string_view sv);

    string buffer_;
    vector<ContainerState> stack_;
    bool afterKey_;  // true immediately after key() — suppresses comma for the value
};
