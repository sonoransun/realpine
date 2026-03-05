/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <nlohmann/json.hpp>


class JsonReader
{
  public:

    static constexpr ulong MAX_STRING_LENGTH    = 4096;
    static constexpr int   MAX_NESTING_DEPTH    = 32;
    static constexpr int   MAX_KEYS_PER_OBJECT  = 256;

    explicit JsonReader (const string & json);
    ~JsonReader () = default;

    bool  getString (const string & key, string & value);

    bool  getUlong (const string & key, ulong & value);

    bool  getBool (const string & key, bool & value);


  private:

    static bool  checkDepthAndKeys (const nlohmann::json & node, int depth);

    nlohmann::json  doc_;
    bool  valid_;

};
