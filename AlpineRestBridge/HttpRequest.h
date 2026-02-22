/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <unordered_map>


class HttpRequest
{
  public:

    HttpRequest () = default;
    ~HttpRequest () = default;

    static bool  parse (const byte *  data,
                        ulong         dataLength,
                        HttpRequest & request);

    string  method;
    string  path;
    string  body;

    std::unordered_map<string, string>  headers;

};
