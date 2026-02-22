/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <JsonRpcHandler.h>
#include <AlpineStackInterface.h>
#include <DtcpStackInterface.h>
#include <JsonReader.h>
#include <JsonWriter.h>
#include <Log.h>
#include <Configuration.h>
#include <NetUtils.h>
#include <cstdlib>
#include <climits>



// ---------------------------------------------------------------------------
//  JSON-RPC helper functions
// ---------------------------------------------------------------------------

static string
jsonRpcError (int code, const string & message, const string & id)
{
    string escaped;
    escaped.reserve(message.length());
    for (char c : message) {
        switch (c) {
            case '"':  escaped += "\\\""; break;
            case '\\': escaped += "\\\\"; break;
            case '\n': escaped += "\\n";  break;
            case '\r': escaped += "\\r";  break;
            case '\t': escaped += "\\t";  break;
            default:   escaped += c;      break;
        }
    }
    string codeStr = std::to_string(code);
    return "{\"jsonrpc\":\"2.0\",\"error\":{\"code\":"s + codeStr +
           ",\"message\":\""s + escaped + "\"},\"id\":"s + id + "}";
}


static string
jsonRpcResult (const string & resultJson, const string & id)
{
    return "{\"jsonrpc\":\"2.0\",\"result\":"s + resultJson +
           ",\"id\":"s + id + "}";
}



// ---------------------------------------------------------------------------
//  RPC method handlers (file-local)
// ---------------------------------------------------------------------------

// Query methods
// ---

static bool
rpcStartQuery (const string & body, string & result)
{
    JsonReader reader(body);

    string queryString;
    if (!reader.getString("queryString", queryString))
        return false;

    if (queryString.length() > 1024)
        return false;

    AlpineStackInterface::t_QueryOptions options;
    reader.getString("groupName", options.groupName);

    ulong autoHaltLimit = 0;
    if (reader.getUlong("autoHaltLimit", autoHaltLimit))
        options.autoHaltLimit = autoHaltLimit;
    else
        options.autoHaltLimit = 0;

    ulong peerDescMax = 0;
    if (reader.getUlong("peerDescMax", peerDescMax))
        options.peerDescMax = peerDescMax;
    else
        options.peerDescMax = 0;

    options.autoDownload = false;
    options.optionId = 0;

    ulong queryId = 0;
    if (!AlpineStackInterface::startQuery(options, queryString, queryId))
        return false;

    JsonWriter writer;
    writer.beginObject();
    writer.key("queryId");
    writer.value(queryId);
    writer.endObject();
    result = writer.result();
    return true;
}


static bool
rpcQueryInProgress (const string & body, string & result)
{
    JsonReader reader(body);

    ulong queryId = 0;
    if (!reader.getUlong("queryId", queryId))
        return false;

    bool inProgress = AlpineStackInterface::queryInProgress(queryId);

    JsonWriter writer;
    writer.beginObject();
    writer.key("queryId");
    writer.value(queryId);
    writer.key("inProgress");
    writer.value(inProgress);
    writer.endObject();
    result = writer.result();
    return true;
}


static bool
rpcGetQueryStatus (const string & body, string & result)
{
    JsonReader reader(body);

    ulong queryId = 0;
    if (!reader.getUlong("queryId", queryId))
        return false;

    AlpineStackInterface::t_QueryStatus status;
    if (!AlpineStackInterface::getQueryStatus(queryId, status))
        return false;

    JsonWriter writer;
    writer.beginObject();
    writer.key("queryId");
    writer.value(queryId);
    writer.key("totalPeers");
    writer.value(status.totalPeers);
    writer.key("peersQueried");
    writer.value(status.peersQueried);
    writer.key("numPeerResponses");
    writer.value(status.numPeerResponses);
    writer.key("totalHits");
    writer.value(status.totalHits);
    writer.endObject();
    result = writer.result();
    return true;
}


static bool
rpcPauseQuery (const string & body, string & result)
{
    JsonReader reader(body);

    ulong queryId = 0;
    if (!reader.getUlong("queryId", queryId))
        return false;

    if (!AlpineStackInterface::pauseQuery(queryId))
        return false;

    JsonWriter writer;
    writer.beginObject();
    writer.key("success");
    writer.value(true);
    writer.endObject();
    result = writer.result();
    return true;
}


static bool
rpcResumeQuery (const string & body, string & result)
{
    JsonReader reader(body);

    ulong queryId = 0;
    if (!reader.getUlong("queryId", queryId))
        return false;

    if (!AlpineStackInterface::resumeQuery(queryId))
        return false;

    JsonWriter writer;
    writer.beginObject();
    writer.key("success");
    writer.value(true);
    writer.endObject();
    result = writer.result();
    return true;
}


static bool
rpcCancelQuery (const string & body, string & result)
{
    JsonReader reader(body);

    ulong queryId = 0;
    if (!reader.getUlong("queryId", queryId))
        return false;

    if (!AlpineStackInterface::cancelQuery(queryId))
        return false;

    JsonWriter writer;
    writer.beginObject();
    writer.key("cancelled");
    writer.value(true);
    writer.key("queryId");
    writer.value(queryId);
    writer.endObject();
    result = writer.result();
    return true;
}


static bool
rpcGetQueryResults (const string & body, string & result)
{
    JsonReader reader(body);

    ulong queryId = 0;
    if (!reader.getUlong("queryId", queryId))
        return false;

    AlpineStackInterface::t_PeerResourcesIndex results;
    if (!AlpineStackInterface::getQueryResults(queryId, results))
        return false;

    JsonWriter writer;
    writer.beginObject();
    writer.key("queryId");
    writer.value(queryId);
    writer.key("peers");
    writer.beginArray();

    for (const auto& peerResult : results)
    {
        writer.beginObject();
        writer.key("peerId");
        writer.value(peerResult.first);
        writer.key("resources");
        writer.beginArray();

        for (const auto& res : peerResult.second.resourceDescList)
        {
            writer.beginObject();
            writer.key("resourceId");
            writer.value(res.resourceId);
            writer.key("size");
            writer.value(res.size);
            writer.key("description");
            writer.value(res.description);
            writer.key("locators");
            writer.beginArray();

            for (const auto& loc : res.locators)
            {
                writer.value(loc);
            }

            writer.endArray();
            writer.endObject();
        }

        writer.endArray();
        writer.endObject();
    }

    writer.endArray();
    writer.endObject();
    result = writer.result();
    return true;
}



// Peer methods
// ---

static bool
rpcGetAllPeers (const string & body, string & result)
{
    DtcpStackInterface::t_DtcpPeerIdList peerIds;
    if (!DtcpStackInterface::getAllDtcpPeerIds(peerIds))
        return false;

    JsonWriter writer;
    writer.beginObject();
    writer.key("peerIds");
    writer.beginArray();

    for (const auto& id : peerIds)
    {
        writer.value(id);
    }

    writer.endArray();
    writer.endObject();
    result = writer.result();
    return true;
}


static bool
rpcGetPeer (const string & body, string & result)
{
    JsonReader reader(body);

    ulong peerId = 0;
    if (!reader.getUlong("peerId", peerId))
        return false;

    DtcpStackInterface::t_DtcpPeerStatus status;
    if (!DtcpStackInterface::getDtcpPeerStatus(peerId, status))
        return false;

    JsonWriter writer;
    writer.beginObject();
    writer.key("peerId");
    writer.value(peerId);
    writer.key("ipAddress");
    writer.value(status.ipAddress);
    writer.key("port");
    writer.value(static_cast<ulong>(status.port));
    writer.key("lastRecvTime");
    writer.value(status.lastRecvTime);
    writer.key("lastSendTime");
    writer.value(status.lastSendTime);
    writer.key("avgBandwidth");
    writer.value(status.avgBandwidth);
    writer.key("peakBandwidth");
    writer.value(status.peakBandwidth);
    writer.endObject();
    result = writer.result();
    return true;
}


static bool
rpcAddPeer (const string & body, string & result)
{
    JsonReader reader(body);

    string ipAddress;
    if (!reader.getString("ipAddress", ipAddress))
        return false;

    ulong portVal = 0;
    if (!reader.getUlong("port", portVal))
        return false;

    ulong validatedIp = 0;
    if (!NetUtils::stringIpToLong(ipAddress, validatedIp))
        return false;

    if (portVal == 0 || portVal > 65535)
        return false;

    ushort port = static_cast<ushort>(portVal);
    port = htons(port);

    if (!DtcpStackInterface::addDtcpPeer(ipAddress, port))
        return false;

    JsonWriter writer;
    writer.beginObject();
    writer.key("success");
    writer.value(true);
    writer.endObject();
    result = writer.result();
    return true;
}


static bool
rpcGetPeerId (const string & body, string & result)
{
    JsonReader reader(body);

    string ipAddress;
    if (!reader.getString("ipAddress", ipAddress))
        return false;

    ulong portVal = 0;
    if (!reader.getUlong("port", portVal))
        return false;

    ulong validatedIp = 0;
    if (!NetUtils::stringIpToLong(ipAddress, validatedIp))
        return false;
    if (portVal == 0 || portVal > 65535)
        return false;

    ushort port = static_cast<ushort>(portVal);
    port = htons(port);

    ulong peerId = 0;
    if (!DtcpStackInterface::getDtcpPeerId(ipAddress, port, peerId))
        return false;

    JsonWriter writer;
    writer.beginObject();
    writer.key("peerId");
    writer.value(peerId);
    writer.endObject();
    result = writer.result();
    return true;
}


static bool
rpcActivatePeer (const string & body, string & result)
{
    JsonReader reader(body);

    ulong peerId = 0;
    if (!reader.getUlong("peerId", peerId))
        return false;

    if (!DtcpStackInterface::activateDtcpPeer(peerId))
        return false;

    JsonWriter writer;
    writer.beginObject();
    writer.key("success");
    writer.value(true);
    writer.endObject();
    result = writer.result();
    return true;
}


static bool
rpcDeactivatePeer (const string & body, string & result)
{
    JsonReader reader(body);

    ulong peerId = 0;
    if (!reader.getUlong("peerId", peerId))
        return false;

    if (!DtcpStackInterface::deactivateDtcpPeer(peerId))
        return false;

    JsonWriter writer;
    writer.beginObject();
    writer.key("success");
    writer.value(true);
    writer.endObject();
    result = writer.result();
    return true;
}


static bool
rpcPingPeer (const string & body, string & result)
{
    JsonReader reader(body);

    ulong peerId = 0;
    if (!reader.getUlong("peerId", peerId))
        return false;

    if (!DtcpStackInterface::pingDtcpPeer(peerId))
        return false;

    JsonWriter writer;
    writer.beginObject();
    writer.key("success");
    writer.value(true);
    writer.endObject();
    result = writer.result();
    return true;
}



// Network filter methods
// ---

static bool
rpcExcludeHost (const string & body, string & result)
{
    JsonReader reader(body);

    string ipAddress;
    if (!reader.getString("ipAddress", ipAddress))
        return false;

    ulong validatedIp = 0;
    if (!NetUtils::stringIpToLong(ipAddress, validatedIp))
        return false;

    if (!DtcpStackInterface::excludeHost(ipAddress))
        return false;

    JsonWriter writer;
    writer.beginObject();
    writer.key("success");
    writer.value(true);
    writer.endObject();
    result = writer.result();
    return true;
}


static bool
rpcExcludeSubnet (const string & body, string & result)
{
    JsonReader reader(body);

    string subnetIpAddress;
    if (!reader.getString("subnetIpAddress", subnetIpAddress))
        return false;

    string subnetMask;
    if (!reader.getString("subnetMask", subnetMask))
        return false;

    ulong validatedIp = 0;
    if (!NetUtils::stringIpToLong(subnetIpAddress, validatedIp))
        return false;
    ulong validatedMask = 0;
    if (!NetUtils::stringIpToLong(subnetMask, validatedMask))
        return false;

    if (!DtcpStackInterface::excludeSubnet(subnetIpAddress, subnetMask))
        return false;

    JsonWriter writer;
    writer.beginObject();
    writer.key("success");
    writer.value(true);
    writer.endObject();
    result = writer.result();
    return true;
}


static bool
rpcAllowHost (const string & body, string & result)
{
    JsonReader reader(body);

    string ipAddress;
    if (!reader.getString("ipAddress", ipAddress))
        return false;

    ulong validatedIp = 0;
    if (!NetUtils::stringIpToLong(ipAddress, validatedIp))
        return false;

    if (!DtcpStackInterface::allowHost(ipAddress))
        return false;

    JsonWriter writer;
    writer.beginObject();
    writer.key("success");
    writer.value(true);
    writer.endObject();
    result = writer.result();
    return true;
}


static bool
rpcAllowSubnet (const string & body, string & result)
{
    JsonReader reader(body);

    string subnetIpAddress;
    if (!reader.getString("subnetIpAddress", subnetIpAddress))
        return false;

    ulong validatedIp = 0;
    if (!NetUtils::stringIpToLong(subnetIpAddress, validatedIp))
        return false;

    if (!DtcpStackInterface::allowSubnet(subnetIpAddress))
        return false;

    JsonWriter writer;
    writer.beginObject();
    writer.key("success");
    writer.value(true);
    writer.endObject();
    result = writer.result();
    return true;
}


static bool
rpcListExcludedHosts (const string & body, string & result)
{
    DtcpStackInterface::t_IpAddressList hostList;
    if (!DtcpStackInterface::listExcludedHosts(hostList))
        return false;

    JsonWriter writer;
    writer.beginObject();
    writer.key("hosts");
    writer.beginArray();

    for (const auto& host : hostList)
    {
        writer.value(host);
    }

    writer.endArray();
    writer.endObject();
    result = writer.result();
    return true;
}


static bool
rpcListExcludedSubnets (const string & body, string & result)
{
    DtcpStackInterface::t_SubnetAddressList subnetList;
    if (!DtcpStackInterface::listExcludedSubnets(subnetList))
        return false;

    JsonWriter writer;
    writer.beginObject();
    writer.key("subnets");
    writer.beginArray();

    for (const auto& subnet : subnetList)
    {
        writer.beginObject();
        writer.key("ipAddress");
        writer.value(subnet->ipAddress);
        writer.key("netMask");
        writer.value(subnet->netMask);
        writer.endObject();
    }

    writer.endArray();
    writer.endObject();
    result = writer.result();
    return true;
}



// Group methods
// ---

static bool
rpcCreateGroup (const string & body, string & result)
{
    JsonReader reader(body);

    string name;
    if (!reader.getString("name", name))
        return false;

    string description;
    reader.getString("description", description);

    ulong groupId = 0;
    if (!AlpineStackInterface::createGroup(name, description, groupId))
        return false;

    JsonWriter writer;
    writer.beginObject();
    writer.key("groupId");
    writer.value(groupId);
    writer.endObject();
    result = writer.result();
    return true;
}


static bool
rpcDeleteGroup (const string & body, string & result)
{
    JsonReader reader(body);

    ulong groupId = 0;
    if (!reader.getUlong("groupId", groupId))
        return false;

    if (!AlpineStackInterface::deleteGroup(groupId))
        return false;

    JsonWriter writer;
    writer.beginObject();
    writer.key("success");
    writer.value(true);
    writer.endObject();
    result = writer.result();
    return true;
}


static bool
rpcListGroups (const string & body, string & result)
{
    AlpineStackInterface::t_IdList groupIds;
    if (!AlpineStackInterface::listGroups(groupIds))
        return false;

    JsonWriter writer;
    writer.beginObject();
    writer.key("groupIds");
    writer.beginArray();

    for (const auto& id : groupIds)
    {
        writer.value(id);
    }

    writer.endArray();
    writer.endObject();
    result = writer.result();
    return true;
}


static bool
rpcGetGroupInfo (const string & body, string & result)
{
    JsonReader reader(body);

    ulong groupId = 0;
    if (!reader.getUlong("groupId", groupId))
        return false;

    AlpineStackInterface::t_GroupInfo info;
    if (!AlpineStackInterface::getGroupInfo(groupId, info))
        return false;

    JsonWriter writer;
    writer.beginObject();
    writer.key("groupId");
    writer.value(info.groupId);
    writer.key("groupName");
    writer.value(info.groupName);
    writer.key("description");
    writer.value(info.description);
    writer.key("numPeers");
    writer.value(info.numPeers);
    writer.key("totalQueries");
    writer.value(info.totalQueries);
    writer.key("totalResponses");
    writer.value(info.totalResponses);
    writer.endObject();
    result = writer.result();
    return true;
}


static bool
rpcGetDefaultGroupInfo (const string & body, string & result)
{
    AlpineStackInterface::t_GroupInfo info;
    if (!AlpineStackInterface::getDefaultGroupInfo(info))
        return false;

    JsonWriter writer;
    writer.beginObject();
    writer.key("groupId");
    writer.value(info.groupId);
    writer.key("groupName");
    writer.value(info.groupName);
    writer.key("description");
    writer.value(info.description);
    writer.key("numPeers");
    writer.value(info.numPeers);
    writer.key("totalQueries");
    writer.value(info.totalQueries);
    writer.key("totalResponses");
    writer.value(info.totalResponses);
    writer.endObject();
    result = writer.result();
    return true;
}


static bool
rpcGetGroupPeerList (const string & body, string & result)
{
    JsonReader reader(body);

    ulong groupId = 0;
    if (!reader.getUlong("groupId", groupId))
        return false;

    AlpineStackInterface::t_IdList peerIds;
    if (!AlpineStackInterface::getGroupPeerList(groupId, peerIds))
        return false;

    JsonWriter writer;
    writer.beginObject();
    writer.key("peerIds");
    writer.beginArray();

    for (const auto& id : peerIds)
    {
        writer.value(id);
    }

    writer.endArray();
    writer.endObject();
    result = writer.result();
    return true;
}


static bool
rpcAddPeerToGroup (const string & body, string & result)
{
    JsonReader reader(body);

    ulong groupId = 0;
    if (!reader.getUlong("groupId", groupId))
        return false;

    ulong peerId = 0;
    if (!reader.getUlong("peerId", peerId))
        return false;

    if (!AlpineStackInterface::addPeerToGroup(groupId, peerId))
        return false;

    JsonWriter writer;
    writer.beginObject();
    writer.key("success");
    writer.value(true);
    writer.endObject();
    result = writer.result();
    return true;
}


static bool
rpcRemovePeerFromGroup (const string & body, string & result)
{
    JsonReader reader(body);

    ulong groupId = 0;
    if (!reader.getUlong("groupId", groupId))
        return false;

    ulong peerId = 0;
    if (!reader.getUlong("peerId", peerId))
        return false;

    if (!AlpineStackInterface::removePeerFromGroup(groupId, peerId))
        return false;

    JsonWriter writer;
    writer.beginObject();
    writer.key("success");
    writer.value(true);
    writer.endObject();
    result = writer.result();
    return true;
}



// Module methods
// ---

static bool
rpcRegisterModule (const string & body, string & result)
{
    JsonReader reader(body);

    string libraryPath;
    if (!reader.getString("libraryPath", libraryPath))
        return false;

    // Reject paths containing directory traversal
    if (libraryPath.find("..") != string::npos)
        return false;

    // If module directory is configured, require path starts with it
    string allowedDir;
    if (Configuration::getValue("Module Directory", allowedDir) && !allowedDir.empty()) {
        char resolvedPath[PATH_MAX];
        char resolvedAllowed[PATH_MAX];
        if (!realpath(libraryPath.c_str(), resolvedPath) ||
            !realpath(allowedDir.c_str(), resolvedAllowed))
            return false;
        if (string(resolvedPath).find(resolvedAllowed) != 0)
            return false;
    }

    string bootstrapSymbol;
    if (!reader.getString("bootstrapSymbol", bootstrapSymbol))
        return false;

    ulong moduleId = 0;
    if (!AlpineStackInterface::registerModule(libraryPath, bootstrapSymbol, moduleId))
        return false;

    JsonWriter writer;
    writer.beginObject();
    writer.key("moduleId");
    writer.value(moduleId);
    writer.endObject();
    result = writer.result();
    return true;
}


static bool
rpcUnregisterModule (const string & body, string & result)
{
    JsonReader reader(body);

    ulong moduleId = 0;
    if (!reader.getUlong("moduleId", moduleId))
        return false;

    if (!AlpineStackInterface::unregisterModule(moduleId))
        return false;

    JsonWriter writer;
    writer.beginObject();
    writer.key("success");
    writer.value(true);
    writer.endObject();
    result = writer.result();
    return true;
}


static bool
rpcLoadModule (const string & body, string & result)
{
    JsonReader reader(body);

    ulong moduleId = 0;
    if (!reader.getUlong("moduleId", moduleId))
        return false;

    if (!AlpineStackInterface::loadModule(moduleId))
        return false;

    JsonWriter writer;
    writer.beginObject();
    writer.key("success");
    writer.value(true);
    writer.endObject();
    result = writer.result();
    return true;
}


static bool
rpcUnloadModule (const string & body, string & result)
{
    JsonReader reader(body);

    ulong moduleId = 0;
    if (!reader.getUlong("moduleId", moduleId))
        return false;

    if (!AlpineStackInterface::unloadModule(moduleId))
        return false;

    JsonWriter writer;
    writer.beginObject();
    writer.key("success");
    writer.value(true);
    writer.endObject();
    result = writer.result();
    return true;
}


static bool
rpcListActiveModules (const string & body, string & result)
{
    AlpineStackInterface::t_IdList moduleIds;
    if (!AlpineStackInterface::listActiveModules(moduleIds))
        return false;

    JsonWriter writer;
    writer.beginObject();
    writer.key("moduleIds");
    writer.beginArray();

    for (const auto& id : moduleIds)
    {
        writer.value(id);
    }

    writer.endArray();
    writer.endObject();
    result = writer.result();
    return true;
}


static bool
rpcListAllModules (const string & body, string & result)
{
    AlpineStackInterface::t_IdList moduleIds;
    if (!AlpineStackInterface::listAllModules(moduleIds))
        return false;

    JsonWriter writer;
    writer.beginObject();
    writer.key("moduleIds");
    writer.beginArray();

    for (const auto& id : moduleIds)
    {
        writer.value(id);
    }

    writer.endArray();
    writer.endObject();
    result = writer.result();
    return true;
}


static bool
rpcGetModuleInfo (const string & body, string & result)
{
    JsonReader reader(body);

    ulong moduleId = 0;
    if (!reader.getUlong("moduleId", moduleId))
        return false;

    AlpineStackInterface::t_ModuleInfo info;
    if (!AlpineStackInterface::getModuleInfo(moduleId, info))
        return false;

    JsonWriter writer;
    writer.beginObject();
    writer.key("moduleId");
    writer.value(info.moduleId);
    writer.key("moduleName");
    writer.value(info.moduleName);
    writer.key("description");
    writer.value(info.description);
    writer.key("version");
    writer.value(info.version);
    writer.key("libraryPath");
    writer.value(info.libraryPath);
    writer.key("bootstrapSymbol");
    writer.value(info.bootstrapSymbol);
    writer.key("activeTime");
    writer.value(info.activeTime);
    writer.endObject();
    result = writer.result();
    return true;
}



// Status methods
// ---

static bool
rpcGetStatus (const string & body, string & result)
{
    JsonWriter writer;
    writer.beginObject();
    writer.key("status");
    writer.value("running"s);
    writer.key("version");
    writer.value("devel-00019"s);
    writer.endObject();
    result = writer.result();
    return true;
}



// ---------------------------------------------------------------------------
//  Method dispatch table
// ---------------------------------------------------------------------------

using t_RpcMethod = bool(*)(const string &, string &);

using t_MethodTable = std::unordered_map<string,
                 t_RpcMethod,
                 OptHash<string>,
                 equal_to<string> >;

static t_MethodTable  methodTable_s;
static bool           tableInitialized_s = false;


static void
initMethodTable ()
{
    if (tableInitialized_s)
        return;

    // Query
    methodTable_s.emplace("startQuery",        rpcStartQuery);
    methodTable_s.emplace("queryInProgress",   rpcQueryInProgress);
    methodTable_s.emplace("getQueryStatus",    rpcGetQueryStatus);
    methodTable_s.emplace("pauseQuery",        rpcPauseQuery);
    methodTable_s.emplace("resumeQuery",       rpcResumeQuery);
    methodTable_s.emplace("cancelQuery",       rpcCancelQuery);
    methodTable_s.emplace("getQueryResults",   rpcGetQueryResults);

    // Peer
    methodTable_s.emplace("getAllPeers",       rpcGetAllPeers);
    methodTable_s.emplace("getPeer",           rpcGetPeer);
    methodTable_s.emplace("addPeer",           rpcAddPeer);
    methodTable_s.emplace("getPeerId",         rpcGetPeerId);
    methodTable_s.emplace("activatePeer",      rpcActivatePeer);
    methodTable_s.emplace("deactivatePeer",    rpcDeactivatePeer);
    methodTable_s.emplace("pingPeer",          rpcPingPeer);

    // Network filter
    methodTable_s.emplace("excludeHost",           rpcExcludeHost);
    methodTable_s.emplace("excludeSubnet",         rpcExcludeSubnet);
    methodTable_s.emplace("allowHost",             rpcAllowHost);
    methodTable_s.emplace("allowSubnet",           rpcAllowSubnet);
    methodTable_s.emplace("listExcludedHosts",     rpcListExcludedHosts);
    methodTable_s.emplace("listExcludedSubnets",   rpcListExcludedSubnets);

    // Group
    methodTable_s.emplace("createGroup",           rpcCreateGroup);
    methodTable_s.emplace("deleteGroup",           rpcDeleteGroup);
    methodTable_s.emplace("listGroups",            rpcListGroups);
    methodTable_s.emplace("getGroupInfo",          rpcGetGroupInfo);
    methodTable_s.emplace("getDefaultGroupInfo",   rpcGetDefaultGroupInfo);
    methodTable_s.emplace("getGroupPeerList",      rpcGetGroupPeerList);
    methodTable_s.emplace("addPeerToGroup",        rpcAddPeerToGroup);
    methodTable_s.emplace("removePeerFromGroup",   rpcRemovePeerFromGroup);

    // Module — registerModule gated by config
    {
        string moduleRegistrationEnabled;
        if (Configuration::getValue("Module Registration Enabled", moduleRegistrationEnabled) &&
            (moduleRegistrationEnabled == "true" || moduleRegistrationEnabled == "1"))
        {
            methodTable_s.emplace("registerModule", rpcRegisterModule);
        }
    }
    methodTable_s.emplace("unregisterModule",  rpcUnregisterModule);
    methodTable_s.emplace("loadModule",        rpcLoadModule);
    methodTable_s.emplace("unloadModule",      rpcUnloadModule);
    methodTable_s.emplace("listActiveModules", rpcListActiveModules);
    methodTable_s.emplace("listAllModules",    rpcListAllModules);
    methodTable_s.emplace("getModuleInfo",     rpcGetModuleInfo);

    // Status
    methodTable_s.emplace("getStatus",         rpcGetStatus);

    tableInitialized_s = true;
}



// ---------------------------------------------------------------------------
//  Public interface
// ---------------------------------------------------------------------------

void
JsonRpcHandler::registerRoutes (HttpRouter & router)
{
    initMethodTable();
    router.addRoute("POST", "/rpc", handleRpc);
}



HttpResponse
JsonRpcHandler::handleRpc (const HttpRequest & request,
                           const std::unordered_map<string, string> & params)
{
    if (request.body.empty())
    {
        string err = jsonRpcError(-32700, "Parse error", "null");
        return HttpResponse::ok(err);
    }

    JsonReader reader(request.body);

    // Validate jsonrpc version
    string version;
    reader.getString("jsonrpc", version);
    if (version != "2.0")
    {
        string err = jsonRpcError(-32600, "Invalid Request", "null");
        return HttpResponse::ok(err);
    }

    // Extract method
    string method;
    if (!reader.getString("method", method))
    {
        string err = jsonRpcError(-32600, "Invalid Request", "null");
        return HttpResponse::ok(err);
    }

    // Extract id — try as number first, fall back to string
    string idStr;
    ulong idNum = 0;
    if (reader.getUlong("id", idNum))
        idStr = std::to_string(idNum);
    else if (reader.getString("id", idStr))
        idStr = "\""s + idStr + "\"";
    else
        idStr = "null";

    // Look up method in dispatch table
    auto iter = methodTable_s.find(method);
    if (iter == methodTable_s.end())
    {
        string err = jsonRpcError(-32601, "Method not found", idStr);
        return HttpResponse::ok(err);
    }

    // Dispatch — pass the full body so handler can read params via JsonReader
    string result;
    if (!iter->second(request.body, result))
    {
        string err = jsonRpcError(-32603, "Internal error", idStr);
        return HttpResponse::ok(err);
    }

    string response = jsonRpcResult(result, idStr);
    return HttpResponse::ok(response);
}
