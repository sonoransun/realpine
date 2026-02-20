///////
///
///  Copyright (C) 2026  sonoransun
///
///  Permission is hereby granted, free of charge, to any person obtaining a copy
///  of this software and associated documentation files (the "Software"), to deal
///  in the Software without restriction, including without limitation the rights
///  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
///  copies of the Software, and to permit persons to whom the Software is
///  furnished to do so, subject to the following conditions:
///
///  The above copyright notice and this permission notice shall be included in all
///  copies or substantial portions of the Software.
///
///  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
///  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
///  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
///  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
///  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
///  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
///  SOFTWARE.
///
///////


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
