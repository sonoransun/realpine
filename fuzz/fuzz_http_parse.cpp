/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <HttpRequest.h>
#include <cstddef>
#include <cstdint>


extern "C" int
LLVMFuzzerTestOneInput(const uint8_t * data, size_t size)
{
    HttpRequest request;
    HttpRequest::parse(data, static_cast<ulong>(size), request);
    return 0;
}
