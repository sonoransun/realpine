/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <PeerHandler.h>
#include <DtcpStackInterface.h>
#include <AlpineRatingEngine.h>
#include <JsonWriter.h>
#include <Log.h>
#include <SafeParse.h>

#ifdef ALPINE_TRACING_ENABLED
#include <Tracing.h>
#endif


void
PeerHandler::registerRoutes (HttpRouter & router)
{
    router.addRoute("GET", "/peers",     getAllPeers);
    router.addRoute("GET", "/peers/:id", getPeer);
}


HttpResponse
PeerHandler::getAllPeers (const HttpRequest & request,
                         const std::unordered_map<string, string> & params)
{
#ifdef ALPINE_TRACING_ENABLED
    ScopedSpan span("peer.getAll"s);
#endif

    // Parse pagination parameters
    ulong limit  = 100;
    ulong offset = 0;

    auto limitIt = request.queryParams.find("limit");
    if (limitIt != request.queryParams.end()) {
        auto parsed = parseUlong(limitIt->second);
        if (parsed)
            limit = std::min(*parsed, 1000UL);
    }

    auto offsetIt = request.queryParams.find("offset");
    if (offsetIt != request.queryParams.end()) {
        auto parsed = parseUlong(offsetIt->second);
        if (parsed)
            offset = *parsed;
    }

    // Parse filter parameters
    string statusFilter;
    auto statusIt = request.queryParams.find("status");
    if (statusIt != request.queryParams.end())
        statusFilter = statusIt->second;

    double minScore = -1.0;
    auto minScoreIt = request.queryParams.find("minScore");
    if (minScoreIt != request.queryParams.end()) {
        auto parsed = parseDouble(minScoreIt->second);
        if (parsed)
            minScore = *parsed;
    }

    DtcpStackInterface::t_DtcpPeerIdList peerIds;

    if (!DtcpStackInterface::getAllDtcpPeerIds(peerIds))
        return HttpResponse::serverError("Failed to get peer list");

    // Apply filters and collect matching peers
    struct PeerEntry {
        ulong                               peerId;
        DtcpStackInterface::t_DtcpPeerStatus status;
        double                              score;
    };

    vector<PeerEntry> filtered;
    filtered.reserve(peerIds.size());

    for (const auto& peerId : peerIds)
    {
        DtcpStackInterface::t_DtcpPeerStatus status;
        if (!DtcpStackInterface::getDtcpPeerStatus(peerId, status))
            continue;

        // Status filter
        if (!statusFilter.empty()) {
            bool active = DtcpStackInterface::peerIsActive(peerId);
            if (statusFilter == "active"s && !active)
                continue;
            if (statusFilter == "inactive"s && active)
                continue;
        }

        // Score filter
        double score = AlpineRatingEngine::getScore(peerId);
        if (minScore >= 0.0 && score < minScore)
            continue;

        filtered.push_back({peerId, status, score});
    }

    ulong total = filtered.size();

    // Apply pagination
    ulong startIdx = std::min(offset, total);
    ulong endIdx   = std::min(startIdx + limit, total);

    JsonWriter writer;
    writer.beginObject();
    writer.key("data");
    writer.beginArray();

    for (ulong i = startIdx; i < endIdx; ++i)
    {
        const auto& entry = filtered[i];

        writer.beginObject();
        writer.key("peerId");
        writer.value(entry.peerId);
        writer.key("ipAddress");
        writer.value(entry.status.ipAddress);
        writer.key("port");
        writer.value((ulong)entry.status.port);
        writer.key("lastRecvTime");
        writer.value(entry.status.lastRecvTime);
        writer.key("lastSendTime");
        writer.value(entry.status.lastSendTime);
        writer.key("avgBandwidth");
        writer.value(entry.status.avgBandwidth);
        writer.key("peakBandwidth");
        writer.value(entry.status.peakBandwidth);
        writer.endObject();
    }

    writer.endArray();
    writer.key("total");
    writer.value(total);
    writer.key("limit");
    writer.value(limit);
    writer.key("offset");
    writer.value(offset);
    writer.endObject();

    return HttpResponse::ok(writer.result());
}


HttpResponse
PeerHandler::getPeer (const HttpRequest & request,
                      const std::unordered_map<string, string> & params)
{
#ifdef ALPINE_TRACING_ENABLED
    ScopedSpan span("peer.get"s);
#endif

    std::unordered_map<string, string>::const_iterator it = params.find("id");
    if (it == params.end())
        return HttpResponse::badRequest("Missing peer id");

    auto parsedId = parseUlong(it->second);
    if (!parsedId)
        return HttpResponse::badRequest("Invalid peer id");
    ulong peerId = *parsedId;

    DtcpStackInterface::t_DtcpPeerStatus status;
    if (!DtcpStackInterface::getDtcpPeerStatus(peerId, status))
        return HttpResponse::notFound();

    JsonWriter writer;
    writer.beginObject();
    writer.key("peerId");
    writer.value(peerId);
    writer.key("ipAddress");
    writer.value(status.ipAddress);
    writer.key("port");
    writer.value((ulong)status.port);
    writer.key("lastRecvTime");
    writer.value(status.lastRecvTime);
    writer.key("lastSendTime");
    writer.value(status.lastSendTime);
    writer.key("avgBandwidth");
    writer.value(status.avgBandwidth);
    writer.key("peakBandwidth");
    writer.value(status.peakBandwidth);
    writer.endObject();

    return HttpResponse::ok(writer.result());
}
