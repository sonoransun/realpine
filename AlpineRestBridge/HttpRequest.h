/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <unordered_map>


class HttpRequest
{
  public:
    static constexpr int MAX_HEADER_COUNT = 100;
    static constexpr ulong MAX_HEADER_NAME_LENGTH = 256;
    static constexpr ulong MAX_HEADER_VALUE_LENGTH = 8192;

    HttpRequest() = default;
    ~HttpRequest() = default;

    [[nodiscard]] static bool parse(const byte * data, ulong dataLength, HttpRequest & request);

    string method;
    string path;
    string body;

    std::unordered_map<string, string> headers;
    std::unordered_map<string, string> queryParams;
};
