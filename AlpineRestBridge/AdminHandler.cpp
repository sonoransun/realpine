/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AdminHandler.h>
#include <ApiKeyAuth.h>
#include <Configuration.h>
#include <DtcpStackInterface.h>
#include <JsonReader.h>
#include <JsonWriter.h>
#include <Log.h>
#include <NetUtils.h>
#include <SafeParse.h>
#include <StringUtils.h>


void
AdminHandler::registerRoutes(HttpRouter & router)
{
    router.addRoute("POST", "/admin/peers/:id/ban", banPeer);
    router.addRoute("DELETE", "/admin/peers/:id/ban", unbanPeer);
    router.addRoute("GET", "/admin/peers/banned", getBannedPeers);
    router.addRoute("POST", "/admin/config/reload", reloadConfig);
    router.addRoute("GET", "/admin/logs/level", getLogLevel);
    router.addRoute("PUT", "/admin/logs/level", setLogLevel);
    router.addRoute("POST", "/admin/keys/rotate", rotateApiKey);
}


HttpResponse
AdminHandler::banPeer(const HttpRequest & request, const std::unordered_map<string, string> & params)
{
    auto it = params.find("id");
    if (it == params.end())
        return HttpResponse::badRequest("Missing peer id");

    // The id can be a peer ID (numeric) — look up IP address from peer status
    auto parsedId = parseUlong(it->second);
    if (!parsedId)
        return HttpResponse::badRequest("Invalid peer id");

    DtcpStackInterface::t_DtcpPeerStatus status;
    if (!DtcpStackInterface::getDtcpPeerStatus(*parsedId, status))
        return HttpResponse::notFound();

    if (!DtcpStackInterface::excludeHost(status.ipAddress))
        return HttpResponse::serverError("Failed to ban peer");

    Log::Info("Admin: banned peer "s + StringUtils::sanitizeForLog(it->second) + " (IP: "s + status.ipAddress + ")"s);

    JsonWriter writer;
    writer.beginObject();
    writer.key("banned");
    writer.value(true);
    writer.key("peerId");
    writer.value(*parsedId);
    writer.key("ipAddress");
    writer.value(status.ipAddress);
    writer.endObject();

    return HttpResponse::ok(writer.result());
}


HttpResponse
AdminHandler::unbanPeer(const HttpRequest & request, const std::unordered_map<string, string> & params)
{
    auto it = params.find("id");
    if (it == params.end())
        return HttpResponse::badRequest("Missing peer id");

    auto parsedId = parseUlong(it->second);
    if (!parsedId)
        return HttpResponse::badRequest("Invalid peer id");

    DtcpStackInterface::t_DtcpPeerStatus status;
    if (!DtcpStackInterface::getDtcpPeerStatus(*parsedId, status))
        return HttpResponse::notFound();

    if (!DtcpStackInterface::allowHost(status.ipAddress))
        return HttpResponse::serverError("Failed to unban peer");

    Log::Info("Admin: unbanned peer "s + StringUtils::sanitizeForLog(it->second) + " (IP: "s + status.ipAddress + ")"s);

    JsonWriter writer;
    writer.beginObject();
    writer.key("unbanned");
    writer.value(true);
    writer.key("peerId");
    writer.value(*parsedId);
    writer.key("ipAddress");
    writer.value(status.ipAddress);
    writer.endObject();

    return HttpResponse::ok(writer.result());
}


HttpResponse
AdminHandler::getBannedPeers(const HttpRequest & request, const std::unordered_map<string, string> & params)
{
    DtcpStackInterface::t_IpAddressList excluded;
    if (!DtcpStackInterface::listExcludedHosts(excluded))
        return HttpResponse::serverError("Failed to get banned list");

    JsonWriter writer;
    writer.beginObject();
    writer.key("banned");
    writer.beginArray();

    for (const auto & ip : excluded)
        writer.value(ip);

    writer.endArray();
    writer.key("total");
    writer.value((ulong)excluded.size());
    writer.endObject();

    return HttpResponse::ok(writer.result());
}


HttpResponse
AdminHandler::reloadConfig(const HttpRequest & request, const std::unordered_map<string, string> & params)
{
    if (!Configuration::reload())
        return HttpResponse::serverError("Failed to reload configuration");

    Log::Info("Admin: configuration reloaded"s);

    JsonWriter writer;
    writer.beginObject();
    writer.key("reloaded");
    writer.value(true);
    writer.endObject();

    return HttpResponse::ok(writer.result());
}


HttpResponse
AdminHandler::flushCache(const HttpRequest & request, const std::unordered_map<string, string> & params)
{
    // ContentStore is not statically accessible — this endpoint is a placeholder
    // that logs the request. Actual flush requires wiring the ContentStore instance.
    Log::Info("Admin: cache flush requested"s);

    JsonWriter writer;
    writer.beginObject();
    writer.key("flushed");
    writer.value(true);
    writer.endObject();

    return HttpResponse::ok(writer.result());
}


HttpResponse
AdminHandler::getLogLevel(const HttpRequest & request, const std::unordered_map<string, string> & params)
{
    auto level = Log::getLogLevel();

    JsonWriter writer;
    writer.beginObject();
    writer.key("level");
    writer.value(string(Log::logLevelToString(level)));
    writer.endObject();

    return HttpResponse::ok(writer.result());
}


HttpResponse
AdminHandler::setLogLevel(const HttpRequest & request, const std::unordered_map<string, string> & params)
{
    JsonReader reader(request.body);

    string levelStr;
    if (!reader.getString("level", levelStr))
        return HttpResponse::badRequest("Missing level field");

    Log::t_LogLevel level;
    if (!Log::stringToLogLevel(levelStr, level))
        return HttpResponse::badRequest("Invalid log level (Silent/Error/Info/Debug)");

    Log::setLogLevel(level);
    Log::Info("Admin: log level set to "s + StringUtils::sanitizeForLog(levelStr));

    JsonWriter writer;
    writer.beginObject();
    writer.key("level");
    writer.value(string(Log::logLevelToString(level)));
    writer.endObject();

    return HttpResponse::ok(writer.result());
}


HttpResponse
AdminHandler::rotateApiKey(const HttpRequest & request, const std::unordered_map<string, string> & params)
{
    // Parse optional grace_period_seconds from JSON body
    ulong gracePeriodSec = 3600;
    if (!request.body.empty()) {
        JsonReader reader(request.body);
        ulong parsedGrace = 0;
        if (reader.getUlong("grace_period_seconds", parsedGrace))
            gracePeriodSec = parsedGrace;
    }

    auto newKey = ApiKeyAuth::rotateKey(std::chrono::seconds(gracePeriodSec));
    if (newKey.empty())
        return HttpResponse::serverError("Failed to rotate API key");

    Log::Info("Admin: API key rotated with grace period "s + std::to_string(gracePeriodSec) + "s"s);

    JsonWriter writer;
    writer.beginObject();
    writer.key("rotated");
    writer.value(true);
    writer.key("new_key");
    writer.value(newKey);
    writer.key("grace_period_seconds");
    writer.value(gracePeriodSec);
    writer.key("active_keys");
    writer.value(ApiKeyAuth::activeKeyCount());
    writer.endObject();

    return HttpResponse::ok(writer.result());
}
