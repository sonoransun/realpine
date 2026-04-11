/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <HttpRequest.h>
#include <HttpResponse.h>
#include <HttpRouter.h>
#include <cstddef>
#include <cstdint>


static HttpResponse
dummyHandler(const HttpRequest & request, const std::unordered_map<string, string> & params)
{
    return HttpResponse::ok("{}");
}


extern "C" int
LLVMFuzzerTestOneInput(const uint8_t * data, size_t size)
{
    HttpRouter router;
    router.addRoute("GET", "/test/:id", dummyHandler);
    router.addRoute("POST", "/query/start", dummyHandler);
    router.addRoute("GET", "/status", dummyHandler);

    HttpRequest request;
    if (HttpRequest::parse(data, static_cast<ulong>(size), request))
        router.dispatch(request);

    return 0;
}
