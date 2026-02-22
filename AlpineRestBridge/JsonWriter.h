/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <nlohmann/json.hpp>
#include <stack>
#include <variant>


class JsonWriter
{
  public:

    JsonWriter ();
    ~JsonWriter () = default;

    string  result ();

    void  beginObject ();
    void  endObject ();

    void  beginArray ();
    void  endArray ();

    void  key (const string & k);

    void  value (const string & v);
    void  value (ulong v);
    void  value (bool v);

    void  separator ();  // no-op with nlohmann


  private:

    nlohmann::json  root_;
    std::stack<nlohmann::json *>  stack_;
    string  currentKey_;

};
