/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>


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

    void  separator ();


  private:

    static string  escape (const string & s);

    string  buffer_;
    bool    needsComma_;

};
