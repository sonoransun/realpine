/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <PeerHandler.h>
#include <DtcpStackInterface.h>
#include <JsonWriter.h>
#include <Log.h>
#include <cstdlib>


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
    DtcpStackInterface::t_DtcpPeerIdList peerIds;

    if (!DtcpStackInterface::getAllDtcpPeerIds(peerIds))
        return HttpResponse::serverError("Failed to get peer list");

    JsonWriter writer;
    writer.beginObject();
    writer.key("peers");
    writer.beginArray();

    for (const auto& peerId : peerIds)
    {
        DtcpStackInterface::t_DtcpPeerStatus status;
        if (!DtcpStackInterface::getDtcpPeerStatus(peerId, status))
            continue;

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
    }

    writer.endArray();
    writer.endObject();

    return HttpResponse::ok(writer.result());
}


HttpResponse
PeerHandler::getPeer (const HttpRequest & request,
                      const std::unordered_map<string, string> & params)
{
    std::unordered_map<string, string>::const_iterator it = params.find("id");
    if (it == params.end())
        return HttpResponse::badRequest("Missing peer id");

    ulong peerId = strtoul(it->second.c_str(), 0, 10);

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
