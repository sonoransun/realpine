/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>


class JsonReader
{
  public:

    JsonReader (const string & json);
    ~JsonReader () = default;

    bool  getString (const string & key, string & value);

    bool  getUlong (const string & key, ulong & value);

    bool  getBool (const string & key, bool & value);


  private:

    bool  findKey (const string & key, ulong & valuePos);

    static void  skipWhitespace (const string & s, ulong & pos);

    string  json_;

};
