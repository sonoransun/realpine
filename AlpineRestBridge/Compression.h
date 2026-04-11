/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <string_view>


class Compression
{
  public:
    // Select best encoding from Accept-Encoding header value
    static string selectEncoding(std::string_view acceptEncoding);

    // Compress data with gzip. Returns empty string on failure.
    static string gzipCompress(std::string_view input);

    // Minimum body size to compress (bytes)
    static constexpr ulong MIN_COMPRESS_SIZE = 1024;
};
