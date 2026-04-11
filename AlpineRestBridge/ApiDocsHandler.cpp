/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <ApiDocsHandler.h>
#include <JsonWriter.h>


HttpRouter * ApiDocsHandler::router_s = nullptr;


void
ApiDocsHandler::registerRoutes(HttpRouter & router)
{
    router_s = &router;
    router.addRoute("GET", "/api", getApiDocs);
}


HttpResponse
ApiDocsHandler::getApiDocs(const HttpRequest & request, const std::unordered_map<string, string> & params)
{
    JsonWriter writer;
    writer.beginObject();
    writer.key("routes");
    writer.beginArray();

    if (router_s) {
        for (const auto & route : router_s->getRoutes()) {
            writer.beginObject();
            writer.key("method");
            writer.value(route.method);
            writer.key("path");
            writer.value(route.pattern);
            writer.key("description");
            writer.value(route.description);
            writer.endObject();
        }
    }

    writer.endArray();
    writer.endObject();

    return HttpResponse::ok(writer.result());
}
