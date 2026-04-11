/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <Compression.h>
#include <algorithm>
#include <zlib.h>


string
Compression::selectEncoding(std::string_view acceptEncoding)
{
    if (acceptEncoding.empty())
        return "identity"s;

    // Parse Accept-Encoding tokens and look for gzip
    // Format: "gzip, deflate" or "br, gzip;q=0.8, identity;q=0.1"
    auto pos = acceptEncoding.find("gzip"s);
    if (pos != std::string_view::npos) {
        // Check that "gzip" is a complete token (not a substring of something else)
        bool validStart = (pos == 0 || acceptEncoding[pos - 1] == ' ' || acceptEncoding[pos - 1] == ',');
        bool validEnd = (pos + 4 >= acceptEncoding.size() || acceptEncoding[pos + 4] == ',' ||
                         acceptEncoding[pos + 4] == ';' || acceptEncoding[pos + 4] == ' ');
        if (validStart && validEnd)
            return "gzip"s;
    }

    return "identity"s;
}


string
Compression::gzipCompress(std::string_view input)
{
    if (input.empty())
        return ""s;

    z_stream zs{};
    // MAX_WBITS | 16 enables gzip wrapper
    if (deflateInit2(&zs, Z_DEFAULT_COMPRESSION, Z_DEFLATED, MAX_WBITS | 16, 8, Z_DEFAULT_STRATEGY) != Z_OK) {
        return ""s;
    }

    zs.next_in = reinterpret_cast<Bytef *>(const_cast<char *>(input.data()));
    zs.avail_in = static_cast<uInt>(input.size());

    // Allocate output buffer — compressed data is typically smaller,
    // but allocate enough for the worst case
    ulong bound = deflateBound(&zs, static_cast<ulong>(input.size()));
    string output(bound, '\0');

    zs.next_out = reinterpret_cast<Bytef *>(output.data());
    zs.avail_out = static_cast<uInt>(output.size());

    int ret = deflate(&zs, Z_FINISH);
    deflateEnd(&zs);

    if (ret != Z_STREAM_END)
        return ""s;

    output.resize(zs.total_out);
    return output;
}
