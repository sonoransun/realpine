/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <StatusHandler.h>
#include <JsonWriter.h>


void
StatusHandler::registerRoutes (HttpRouter & router)
{
    router.addRoute("GET", "/status", getStatus);
}


HttpResponse
StatusHandler::getStatus (const HttpRequest & request,
                          const std::unordered_map<string, string> & params)
{
    JsonWriter writer;
    writer.beginObject();
    writer.key("status");
    writer.value("running");
    writer.key("version");
    writer.value("devel-00019");
    writer.endObject();

    return HttpResponse::ok(writer.result());
}
