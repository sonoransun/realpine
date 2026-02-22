/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <DlnaHandler.h>
#include <XmlWriter.h>

#include <cstdio>


static void
addAction (XmlWriter & xml, const string & name,
           const string * inArgs, int inCount,
           const string * outArgs, int outCount)
{
    xml.beginElement("action");
    xml.simpleElement("name", name);
    xml.beginElement("argumentList");

    for (int i = 0; i < inCount; i++) {
        xml.beginElement("argument");
        xml.simpleElement("name", inArgs[i]);
        xml.simpleElement("direction", "in");
        xml.endElement("argument");
    }

    for (int i = 0; i < outCount; i++) {
        xml.beginElement("argument");
        xml.simpleElement("name", outArgs[i]);
        xml.simpleElement("direction", "out");
        xml.endElement("argument");
    }

    xml.endElement("argumentList");
    xml.endElement("action");
}


string
DlnaHandler::deviceDescription (const string & uuid, const string & baseUrl)
{
    XmlWriter xml;
    xml.declaration();
    xml.beginElement("root", "urn:schemas-upnp-org:device-1-0");

    xml.beginElement("specVersion");
    xml.simpleElement("major", "1");
    xml.simpleElement("minor", "0");
    xml.endElement("specVersion");

    xml.simpleElement("URLBase", baseUrl);

    xml.beginElement("device");
    xml.simpleElement("deviceType", "urn:schemas-upnp-org:device:MediaServer:1");
    xml.simpleElement("friendlyName", "Alpine Media Server");
    xml.simpleElement("manufacturer", "Alpine Project");
    xml.simpleElement("modelName", "Alpine DLNA Bridge");
    xml.simpleElement("modelNumber", "1.0");
    xml.simpleElement("UDN", uuid);

    xml.beginElement("serviceList");

    xml.beginElement("service");
    xml.simpleElement("serviceType", "urn:schemas-upnp-org:service:ContentDirectory:1");
    xml.simpleElement("serviceId", "urn:upnp-org:serviceId:ContentDirectory");
    xml.simpleElement("SCPDURL", "/cds.xml");
    xml.simpleElement("controlURL", "/control/cds");
    xml.simpleElement("eventSubURL", "/event/cds");
    xml.endElement("service");

    xml.beginElement("service");
    xml.simpleElement("serviceType", "urn:schemas-upnp-org:service:ConnectionManager:1");
    xml.simpleElement("serviceId", "urn:upnp-org:serviceId:ConnectionManager");
    xml.simpleElement("SCPDURL", "/cms.xml");
    xml.simpleElement("controlURL", "/control/cms");
    xml.simpleElement("eventSubURL", "/event/cms");
    xml.endElement("service");

    xml.endElement("serviceList");
    xml.endElement("device");
    xml.endElement("root");

    return xml.result();
}


string
DlnaHandler::cdsServiceDescription ()
{
    static const string cached = []() {
        XmlWriter xml;
        xml.declaration();
        xml.beginElement("scpd", "urn:schemas-upnp-org:service-1-0");

        xml.beginElement("specVersion");
        xml.simpleElement("major", "1");
        xml.simpleElement("minor", "0");
        xml.endElement("specVersion");

        xml.beginElement("actionList");

        string browseIn[]  = {"ObjectID", "BrowseFlag", "Filter",
                              "StartingIndex", "RequestedCount", "SortCriteria"};
        string browseOut[] = {"Result", "NumberReturned", "TotalMatches", "UpdateID"};
        addAction(xml, "Browse", browseIn, 6, browseOut, 4);

        string updateOut[] = {"Id"};
        addAction(xml, "GetSystemUpdateID", nullptr, 0, updateOut, 1);

        string searchOut[] = {"SearchCaps"};
        addAction(xml, "GetSearchCapabilities", nullptr, 0, searchOut, 1);

        string sortOut[] = {"SortCaps"};
        addAction(xml, "GetSortCapabilities", nullptr, 0, sortOut, 1);

        xml.endElement("actionList");

        xml.beginElement("serviceStateTable");
        xml.selfClosing("stateVariable");
        xml.endElement("serviceStateTable");

        xml.endElement("scpd");
        return xml.result();
    }();

    return cached;
}


string
DlnaHandler::cmsServiceDescription ()
{
    static const string cached = []() {
        XmlWriter xml;
        xml.declaration();
        xml.beginElement("scpd", "urn:schemas-upnp-org:service-1-0");

        xml.beginElement("specVersion");
        xml.simpleElement("major", "1");
        xml.simpleElement("minor", "0");
        xml.endElement("specVersion");

        xml.beginElement("actionList");

        string protoOut[] = {"Source", "Sink"};
        addAction(xml, "GetProtocolInfo", nullptr, 0, protoOut, 2);

        string connIdsOut[] = {"ConnectionIDs"};
        addAction(xml, "GetCurrentConnectionIDs", nullptr, 0, connIdsOut, 1);

        string connInfoIn[] = {"ConnectionID"};
        addAction(xml, "GetCurrentConnectionInfo", connInfoIn, 1, nullptr, 0);

        xml.endElement("actionList");

        xml.beginElement("serviceStateTable");
        xml.selfClosing("stateVariable");
        xml.endElement("serviceStateTable");

        xml.endElement("scpd");
        return xml.result();
    }();

    return cached;
}


string
DlnaHandler::handleCdsAction (const string & soapBody,
                               const string & soapAction,
                               ContentStore & store,
                               const string & baseUrl,
                               bool transcodeEnabled)
{
    string svcType = "urn:schemas-upnp-org:service:ContentDirectory:1";

    if (soapAction.contains("Browse"))
        return actionBrowse(soapBody, store, baseUrl, transcodeEnabled);

    if (soapAction.contains("GetSystemUpdateID")) {
        string body = "<Id>"s + std::to_string(store.getSystemUpdateId()) + "</Id>";
        return wrapSoapResponse("GetSystemUpdateID", svcType, body);
    }

    if (soapAction.contains("GetSearchCapabilities"))
        return wrapSoapResponse("GetSearchCapabilities", svcType,
                                "<SearchCaps></SearchCaps>");

    if (soapAction.contains("GetSortCapabilities"))
        return wrapSoapResponse("GetSortCapabilities", svcType,
                                "<SortCaps></SortCaps>");

    return wrapSoapResponse("Unknown", svcType, "");
}


string
DlnaHandler::handleCmsAction (const string & soapBody,
                               const string & soapAction)
{
    string svcType = "urn:schemas-upnp-org:service:ConnectionManager:1";

    if (soapAction.contains("GetProtocolInfo"))
        return actionGetProtocolInfo();

    if (soapAction.contains("GetCurrentConnectionIDs"))
        return wrapSoapResponse("GetCurrentConnectionIDs", svcType,
                                "<ConnectionIDs>0</ConnectionIDs>");

    if (soapAction.contains("GetCurrentConnectionInfo")) {
        return wrapSoapResponse("GetCurrentConnectionInfo", svcType,
            "<RcsID>-1</RcsID>"
            "<AVTransportID>-1</AVTransportID>"
            "<ProtocolInfo></ProtocolInfo>"
            "<PeerConnectionManager></PeerConnectionManager>"
            "<PeerConnectionID>-1</PeerConnectionID>"
            "<Direction>Output</Direction>"
            "<Status>OK</Status>");
    }

    return wrapSoapResponse("Unknown", svcType, "");
}


string
DlnaHandler::actionBrowse (const string & soapBody,
                            ContentStore & store,
                            const string & baseUrl,
                            bool transcodeEnabled)
{
    string svcType = "urn:schemas-upnp-org:service:ContentDirectory:1";

    string objectId    = extractXmlValue(soapBody, "ObjectID");
    string browseFlag  = extractXmlValue(soapBody, "BrowseFlag");
    string startIdxStr = extractXmlValue(soapBody, "StartingIndex");
    string reqCountStr = extractXmlValue(soapBody, "RequestedCount");

    ulong startIdx = startIdxStr.empty() ? 0 : strtoul(startIdxStr.c_str(), nullptr, 10);
    ulong reqCount = reqCountStr.empty() ? 0 : strtoul(reqCountStr.c_str(), nullptr, 10);

    if (browseFlag == "BrowseMetadata" && (objectId == "0" || objectId.empty())) {
        string didl = buildDidlContainer(store.getItemCount());
        string body = "<Result>"s + didl + "</Result>"
            "<NumberReturned>1</NumberReturned>"
            "<TotalMatches>1</TotalMatches>"
            "<UpdateID>" + std::to_string(store.getSystemUpdateId()) + "</UpdateID>";
        return wrapSoapResponse("Browse", svcType, body);
    }

    vector<ContentStore::MediaItem> items;
    store.getAllItems(items);

    ulong totalMatches = items.size();
    if (reqCount == 0) reqCount = totalMatches;

    string didlItems;
    ulong returned = 0;

    for (ulong i = startIdx; i < items.size() && returned < reqCount; i++) {
        didlItems += buildDidlItem(items[i], baseUrl, transcodeEnabled);
        returned++;
    }

    string didl =
        "&lt;DIDL-Lite xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/\""
        " xmlns:dc=\"http://purl.org/dc/elements/1.1/\""
        " xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\""
        " xmlns:dlna=\"urn:schemas-dlna-org:metadata-1-0/\"&gt;"s +
        didlItems +
        "&lt;/DIDL-Lite&gt;";

    string body = "<Result>"s + didl + "</Result>"
        "<NumberReturned>" + std::to_string(returned) + "</NumberReturned>"
        "<TotalMatches>" + std::to_string(totalMatches) + "</TotalMatches>"
        "<UpdateID>" + std::to_string(store.getSystemUpdateId()) + "</UpdateID>";

    return wrapSoapResponse("Browse", svcType, body);
}


string
DlnaHandler::actionGetProtocolInfo ()
{
    string svcType = "urn:schemas-upnp-org:service:ConnectionManager:1";

    return wrapSoapResponse("GetProtocolInfo", svcType,
        "<Source>"
        "http-get:*:video/mp4:*,"
        "http-get:*:video/x-matroska:*,"
        "http-get:*:video/x-msvideo:*,"
        "http-get:*:video/quicktime:*,"
        "http-get:*:video/webm:*,"
        "http-get:*:video/mpeg:*,"
        "http-get:*:video/mp2t:*"
        "</Source>"
        "<Sink></Sink>");
}


string
DlnaHandler::buildDidlItem (const ContentStore::MediaItem & item,
                             const string & baseUrl,
                             bool transcodeEnabled)
{
    string mediaUrl = baseUrl + "/media/" + item.id;

    string result =
        "&lt;item id=\""s + item.id + "\" parentID=\"0\" restricted=\"1\"&gt;"
        "&lt;dc:title&gt;" + item.title + "&lt;/dc:title&gt;"
        "&lt;upnp:class&gt;object.item.videoItem&lt;/upnp:class&gt;"
        "&lt;res protocolInfo=\"http-get:*:" + item.mimeType + ":DLNA.ORG_OP=01;DLNA.ORG_CI=0\""
        " size=\"" + std::to_string(item.fileSize) + "\"&gt;" + mediaUrl + "&lt;/res&gt;";

    if (transcodeEnabled) {
        result +=
            "&lt;res protocolInfo=\"http-get:*:video/mp2t:DLNA.ORG_OP=00;DLNA.ORG_CI=1\"&gt;"s +
            mediaUrl + "/transcode&lt;/res&gt;";
    }

    result += "&lt;/item&gt;";
    return result;
}


string
DlnaHandler::buildDidlContainer (ulong childCount)
{
    return
        "&lt;DIDL-Lite xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/\""
        " xmlns:dc=\"http://purl.org/dc/elements/1.1/\""
        " xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\"&gt;"
        "&lt;container id=\"0\" parentID=\"-1\" childCount=\""s + std::to_string(childCount) + "\" restricted=\"1\"&gt;"
        "&lt;dc:title&gt;Root&lt;/dc:title&gt;"
        "&lt;upnp:class&gt;object.container&lt;/upnp:class&gt;"
        "&lt;/container&gt;"
        "&lt;/DIDL-Lite&gt;";
}


string
DlnaHandler::wrapSoapResponse (const string & action,
                                const string & svcType,
                                const string & body)
{
    return
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\""
        " s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">"
        "<s:Body>"
        "<u:"s + action + "Response xmlns:u=\"" + svcType + "\">" +
        body +
        "</u:" + action + "Response>"
        "</s:Body>"
        "</s:Envelope>";
}


string
DlnaHandler::extractXmlValue (const string & xml, const string & tag)
{
    string openTag = "<" + tag + ">";
    string closeTag = "</" + tag + ">";

    auto startPos = xml.find(openTag);
    if (startPos == string::npos) {
        auto colonPos = xml.find(":" + tag + ">");
        if (colonPos == string::npos)
            return "";
        auto ltPos = xml.rfind('<', colonPos);
        if (ltPos == string::npos)
            return "";
        startPos = ltPos;
        openTag = xml.substr(ltPos, colonPos - ltPos + tag.length() + 2);
        auto nsEnd = xml.find(':', ltPos + 1);
        string prefix = xml.substr(ltPos + 1, nsEnd - ltPos - 1);
        closeTag = "</" + prefix + ":" + tag + ">";
    }

    startPos += openTag.length();
    auto endPos = xml.find(closeTag, startPos);

    if (endPos == string::npos)
        return "";

    return xml.substr(startPos, endPos - startPos);
}
