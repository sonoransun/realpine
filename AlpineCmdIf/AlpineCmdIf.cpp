/// Copyright (C) 2026 sonoransun — see LICENCE.txt



#include <time.h>
#include <stdlib.h>
#include <iostream>

using std::cout; using std::endl;

#include <Log.h>
#include <StringUtils.h>
#include <NetUtils.h>
#include <OptHash.h>

#include <ApplCore.h>
#include <Configuration.h>

#include <JsonRpcClient.h>
#include <JsonReader.h>
#include <JsonWriter.h>

#include <AlpineConfig.h>



// Types for command dispatch table
//
using t_Handler = bool(*)();

using t_DispatchTable = std::unordered_map<string,
                 t_Handler,
                 OptHash<string>,
                 equal_to<string> >;

using t_DispatchTablePair = std::pair<string, t_Handler>;


// Global members
//
static t_DispatchTable *  dispatchTable_s = nullptr;
static bool  verbose_s = false;


bool buildDispatchTable ();

bool commandExists (const string &  command);

bool handleCommand (const string &  command);




// Application main entry point
// ----------------------------
//
// Return codes:
//   0 - Request Successful
//   1 - Initialization Failure
//   2 - Invalid Operation / Command
//   3 - Request Failed
//
int
main (int argc, char *argv[])
{
    bool status;

    status = buildDispatchTable ();

    if (!status) {
        Log::Error ("Building dispatch table failed!  Exiting.");
        return 1;
    }


    // Initialize application core
    //
    status = ApplCore::initialize (argc, argv);

    if (!status) {
        Log::Error ("Unable to initialize application core!  Exiting.");
        return 1;
    }

    // Initialize configuration
    //
    ConfigData::t_ConfigElementList * configElements;

    AlpineConfig::createConfigElements ();
    AlpineConfig::getConfigElements (configElements);

    status = Configuration::initialize (argc,
                                        argv,
                                        *configElements,
                                        AlpineConfig::configFile_s);

    if (!status) {
        Log::Error ("Error initializing configuration!  Exiting.");
        return 1;
    }


    // Load configuration settings
    //
    string  serverAddress;
    status = Configuration::getValue ("Server Address", serverAddress);

    if (!status) {
        Log::Error ("No Server Address value!  Exiting.");
        return 1;
    }

    string  serverPortStr;
    status = Configuration::getValue ("Server Port", serverPortStr);

    if (!status) {
        Log::Error ("No Server Port value!  Exiting.");
        return 1;
    }

    ushort serverPort = static_cast<ushort>(atoi(serverPortStr.c_str()));

    string command;
    status = Configuration::getValue ("Command", command);

    if (!status) {
        Log::Error ("No command value!  Exiting.");
        return 1;
    }

    string verboseValue;
    status = Configuration::getValue ("Verbose", verboseValue);

    if (status) {
        // User specified verbose mode
        verbose_s = true;
        verboseValue = "Verbose mode ON";
    }
    else {
        verboseValue = "Verbose mode OFF";
    }



    Log::Info ("Starting ALPINE Command Interface-"s +
               "\nServer: "s + serverAddress + ":" + serverPortStr +
               "\nCommand: "s +  command +
               "\n"s + verboseValue +
               "\n");



    // Initialize JSON-RPC client
    //
    status = JsonRpcClient::initialize (serverAddress, serverPort);

    if (!status) {
        string msg("Initializing JsonRpcClient failed.  Exiting.");
        Log::Error (msg);

        if (verbose_s) {
            cout << msg << endl;
        }

        return 1;
    }

    // Perform requested command...
    //
    status = commandExists (command);

    if (!status) {
        string  msg("Invalid command!  Exiting.");
        Log::Error (msg);

        if (verbose_s) {
            cout << msg << endl;
        }

        return 2;
    }

    status = handleCommand (command);

    if (!status) {
        return 3;
    }


    Log::Info ("Command finished.  Exiting.");

    return 0;
}



// Methods for each supported command type
//
bool  performAddDtcpPeer ();
bool  performGetDtcpPeerId ();
bool  performGetDtcpPeerStatus ();
bool  performActivateDtcpPeer ();
bool  performDeactivateDtcpPeer ();
bool  performPingDtcpPeer ();
bool  performExcludeHost ();
bool  performExcludeSubnet ();
bool  performAllowHost ();
bool  performAllowSubnet ();
bool  performListExcludedHosts ();
bool  performListExcludedSubnets ();
bool  performGetUserGroupList ();
bool  performCreateUserGroup ();
bool  performDestroyUserGroup ();
bool  performGetPeerUserGroupList ();
bool  performAddPeerToGroup ();
bool  performRemovePeerFromGroup ();
bool  performGetExtendedPeerList ();
bool  performStartQuery ();
bool  performGetQueryStatus ();
bool  performPauseQuery ();
bool  performResumeQuery ();
bool  performCancelQuery ();
bool  performGetQueryResults ();
bool  performRegisterModule ();
bool  performUnregisterModule ();
bool  performLoadModule ();
bool  performUnloadModule ();
bool  performListActiveModules ();
bool  performListAllModules ();
bool  performGetModuleInfo ();
bool  performGetStatus ();


// Initialize dispatch table
//
bool
buildDispatchTable ()
{
    dispatchTable_s = new t_DispatchTable;

    dispatchTable_s->emplace ("addDtcpPeer", &performAddDtcpPeer);
    dispatchTable_s->emplace ("getDtcpPeerId", &performGetDtcpPeerId);
    dispatchTable_s->emplace ("getDtcpPeerStatus", &performGetDtcpPeerStatus);
    dispatchTable_s->emplace ("activateDtcpPeer", &performActivateDtcpPeer);
    dispatchTable_s->emplace ("deactivateDtcpPeer", &performDeactivateDtcpPeer);
    dispatchTable_s->emplace ("pingDtcpPeer", &performPingDtcpPeer);
    dispatchTable_s->emplace ("excludeHost", &performExcludeHost);
    dispatchTable_s->emplace ("excludeSubnet", &performExcludeSubnet);
    dispatchTable_s->emplace ("allowHost", &performAllowHost);
    dispatchTable_s->emplace ("allowSubnet", &performAllowSubnet);
    dispatchTable_s->emplace ("listExcludedHosts", &performListExcludedHosts);
    dispatchTable_s->emplace ("listExcludedSubnets", &performListExcludedSubnets);
    dispatchTable_s->emplace ("getUserGroupList", &performGetUserGroupList);
    dispatchTable_s->emplace ("createUserGroup", &performCreateUserGroup);
    dispatchTable_s->emplace ("destroyUserGroup", &performDestroyUserGroup);
    dispatchTable_s->emplace ("getPeerUserGroupList", &performGetPeerUserGroupList);
    dispatchTable_s->emplace ("addPeerToGroup", &performAddPeerToGroup);
    dispatchTable_s->emplace ("removePeerFromGroup", &performRemovePeerFromGroup);
    dispatchTable_s->emplace ("getExtendedPeerList", &performGetExtendedPeerList);
    dispatchTable_s->emplace ("beginQuery", &performStartQuery);
    dispatchTable_s->emplace ("getQueryStatus", &performGetQueryStatus);
    dispatchTable_s->emplace ("pauseQuery", &performPauseQuery);
    dispatchTable_s->emplace ("resumeQuery", &performResumeQuery);
    dispatchTable_s->emplace ("cancelQuery", &performCancelQuery);
    dispatchTable_s->emplace ("getQueryResults", &performGetQueryResults);
    dispatchTable_s->emplace ("registerModule", &performRegisterModule);
    dispatchTable_s->emplace ("unregisterModule", &performUnregisterModule);
    dispatchTable_s->emplace ("loadModule", &performLoadModule);
    dispatchTable_s->emplace ("unloadModule", &performUnloadModule);
    dispatchTable_s->emplace ("listActiveModules", &performListActiveModules);
    dispatchTable_s->emplace ("listAllModules", &performListAllModules);
    dispatchTable_s->emplace ("getModuleInfo", &performGetModuleInfo);
    dispatchTable_s->emplace ("getStatus", &performGetStatus);


    return true;
}



bool
commandExists (const string &  command)
{
    return dispatchTable_s->find (command) != dispatchTable_s->end ();
}



// Dispatch correct command
//
bool
handleCommand (const string &  command)
{
    t_Handler  handler;

    auto iter = dispatchTable_s->find (command);

    if (iter == dispatchTable_s->end ()) {
        Log::Error ("Invalid command option: "s + command + " provided!");
        return false;
    }
    handler = (*iter).second;

    bool status;
    status = (*handler)();

    if (!status) {
        if (verbose_s) {
            cout << "Request failed!  Check application log file.\n";
        }

        return false;
    }
    return true;
}



// ---------------------------------------------------------------------------
//  Peer commands
// ---------------------------------------------------------------------------

bool
performAddDtcpPeer ()
{
    Log::Debug ("performAddDtcpPeer invoked.");

    string  ipAddress;
    string  portStr;

    bool status;
    status = Configuration::getValue ("IP Address", ipAddress);

    if (!status) {
        Log::Error ("No IP Address value given!");
        return false;
    }
    status = Configuration::getValue ("Port", portStr);

    if (!status) {
        Log::Error ("No Port value given!");
        return false;
    }

    JsonWriter writer;
    writer.beginObject();
    writer.key("ipAddress");
    writer.value(ipAddress);
    writer.key("port");
    writer.value(strtoul(portStr.c_str(), 0, 0));
    writer.endObject();

    string result;
    status = JsonRpcClient::call("addPeer", writer.result(), result);

    if (!status) {
        Log::Error ("addPeer RPC call failed!");
        return false;
    }
    if (verbose_s) {
        cout << "Add Request successful.\n";
    }

    return true;
}



bool
performGetDtcpPeerId ()
{
    Log::Debug ("performGetDtcpPeerId invoked.");

    string  ipAddress;
    string  portStr;

    bool status;
    status = Configuration::getValue ("IP Address", ipAddress);

    if (!status) {
        Log::Error ("No IP Address value given!");
        return false;
    }
    status = Configuration::getValue ("Port", portStr);

    if (!status) {
        Log::Error ("No Port value given!");
        return false;
    }

    JsonWriter writer;
    writer.beginObject();
    writer.key("ipAddress");
    writer.value(ipAddress);
    writer.key("port");
    writer.value(strtoul(portStr.c_str(), 0, 0));
    writer.endObject();

    string result;
    status = JsonRpcClient::call("getPeerId", writer.result(), result);

    if (!status) {
        Log::Error ("getPeerId RPC call failed!");
        return false;
    }

    JsonReader reader(result);
    ulong peerId = 0;
    reader.getUlong("peerId", peerId);

    if (verbose_s) {
        cout << "Request successful.\n";
        cout << "Peer ID: " << peerId << endl;
    }
    else {
        cout << peerId << endl;
    }

    return true;
}



bool
performGetDtcpPeerStatus ()
{
    Log::Debug ("performGetDtcpPeerStatus invoked.");

    string  peerIdString;
    ulong   peerId;

    bool status;
    status = Configuration::getValue ("Peer ID", peerIdString);

    if (!status) {
        Log::Error ("No Peer ID value given!");
        return false;
    }
    peerId = strtoul (peerIdString.c_str(), 0, 0);

    JsonWriter writer;
    writer.beginObject();
    writer.key("peerId");
    writer.value(peerId);
    writer.endObject();

    string result;
    status = JsonRpcClient::call("getPeer", writer.result(), result);

    if (!status) {
        Log::Error ("getPeer RPC call failed!");
        return false;
    }

    JsonReader reader(result);
    string ipAddr;
    ulong port = 0, lastRecv = 0, lastSend = 0, avgBw = 0, peakBw = 0;
    reader.getString("ipAddress", ipAddr);
    reader.getUlong("port", port);
    reader.getUlong("lastRecvTime", lastRecv);
    reader.getUlong("lastSendTime", lastSend);
    reader.getUlong("avgBandwidth", avgBw);
    reader.getUlong("peakBandwidth", peakBw);

    if (verbose_s) {
        cout << "Request successful.\n";
        cout << "- Dtcp Peer Status:\n";
        cout << "IpAddress: " << ipAddr << endl;
        cout << "Port: " << port << endl;
        cout << "Last Receive Time: " << lastRecv << endl;
        cout << "Last Send Time: " << lastSend << endl;
        cout << "Average Bandwidth: " << avgBw << " Kbps" << endl;
        cout << "Peak Bandwidth: " << peakBw << " Kbps" << endl;
        cout << "---\n\n";
    }
    else {
        cout << ipAddr << " ";
        cout << port << " ";
        cout << lastRecv << " ";
        cout << lastSend << " ";
        cout << avgBw << " ";
        cout << peakBw << " ";
        cout << endl;
    }

    return true;
}



bool
performActivateDtcpPeer ()
{
    Log::Debug ("performActivateDtcpPeer invoked.");

    string  peerIdString;
    ulong   peerId;

    bool status;
    status = Configuration::getValue ("Peer ID", peerIdString);

    if (!status) {
        Log::Error ("No Peer ID value given!");
        return false;
    }
    peerId = strtoul (peerIdString.c_str(), 0, 0);

    JsonWriter writer;
    writer.beginObject();
    writer.key("peerId");
    writer.value(peerId);
    writer.endObject();

    string result;
    status = JsonRpcClient::call("activatePeer", writer.result(), result);

    if (!status) {
        Log::Error ("activatePeer RPC call failed!");
        return false;
    }
    if (verbose_s) {
        cout << "Activation successful.\n";
    }

    return true;
}



bool
performDeactivateDtcpPeer ()
{
    Log::Debug ("performDeactivateDtcpPeer invoked.");

    string  peerIdString;
    ulong   peerId;

    bool status;
    status = Configuration::getValue ("Peer ID", peerIdString);

    if (!status) {
        Log::Error ("No Peer ID value given!");
        return false;
    }
    peerId = strtoul (peerIdString.c_str(), 0, 0);

    JsonWriter writer;
    writer.beginObject();
    writer.key("peerId");
    writer.value(peerId);
    writer.endObject();

    string result;
    status = JsonRpcClient::call("deactivatePeer", writer.result(), result);

    if (!status) {
        Log::Error ("deactivatePeer RPC call failed!");
        return false;
    }
    if (verbose_s) {
        cout << "Deactivation successful.\n";
    }

    return true;
}



bool
performPingDtcpPeer ()
{
    Log::Debug ("performPingDtcpPeer invoked.");

    string  peerIdString;
    ulong   peerId;

    bool status;
    status = Configuration::getValue ("Peer ID", peerIdString);

    if (!status) {
        Log::Error ("No Peer ID value given!");
        return false;
    }
    peerId = strtoul (peerIdString.c_str(), 0, 0);

    JsonWriter writer;
    writer.beginObject();
    writer.key("peerId");
    writer.value(peerId);
    writer.endObject();

    string result;
    status = JsonRpcClient::call("pingPeer", writer.result(), result);

    if (!status) {
        Log::Error ("pingPeer RPC call failed!");
        return false;
    }
    if (verbose_s) {
        cout << "Ping request successful.\n";
    }

    return true;
}



// ---------------------------------------------------------------------------
//  Network filter commands
// ---------------------------------------------------------------------------

bool
performExcludeHost ()
{
    Log::Debug ("performExcludeHost invoked.");

    bool status;
    string ipAddress;

    status = Configuration::getValue ("IP Address", ipAddress);

    if (!status) {
        Log::Error ("No IP Address value given!");
        return false;
    }

    JsonWriter writer;
    writer.beginObject();
    writer.key("ipAddress");
    writer.value(ipAddress);
    writer.endObject();

    string result;
    status = JsonRpcClient::call("excludeHost", writer.result(), result);

    if (!status) {
        Log::Error ("excludeHost RPC call failed!");
        return false;
    }
    if (verbose_s) {
        cout << "Exclude host request successful.\n";
    }

    return true;
}



bool
performExcludeSubnet ()
{
    Log::Debug ("performExcludeSubnet invoked.");

    bool status;
    string subnetIpAddress;
    string subnetMask;

    status = Configuration::getValue ("Subnet IP Address", subnetIpAddress);

    if (!status) {
        Log::Error ("No Subnet IP Address value given!");
        return false;
    }
    status = Configuration::getValue ("Subnet Mask", subnetMask);

    if (!status) {
        Log::Error ("No Subnet Mask value given!");
        return false;
    }

    JsonWriter writer;
    writer.beginObject();
    writer.key("subnetIpAddress");
    writer.value(subnetIpAddress);
    writer.key("subnetMask");
    writer.value(subnetMask);
    writer.endObject();

    string result;
    status = JsonRpcClient::call("excludeSubnet", writer.result(), result);

    if (!status) {
        Log::Error ("excludeSubnet RPC call failed!");
        return false;
    }
    if (verbose_s) {
        cout << "Exclude subnet request successful.\n";
    }

    return true;
}



bool
performAllowHost ()
{
    Log::Debug ("performAllowHost invoked.");

    bool status;
    string ipAddress;

    status = Configuration::getValue ("IP Address", ipAddress);

    if (!status) {
        Log::Error ("No IP Address value given!");
        return false;
    }

    JsonWriter writer;
    writer.beginObject();
    writer.key("ipAddress");
    writer.value(ipAddress);
    writer.endObject();

    string result;
    status = JsonRpcClient::call("allowHost", writer.result(), result);

    if (!status) {
        Log::Error ("allowHost RPC call failed!");
        return false;
    }
    if (verbose_s) {
        cout << "Allow host request successful.\n";
    }

    return true;
}



bool
performAllowSubnet ()
{
    Log::Debug ("performAllowSubnet invoked.");

    bool status;
    string subnetIpAddress;

    status = Configuration::getValue ("Subnet IP Address", subnetIpAddress);

    if (!status) {
        Log::Error ("No Subnet IP Address value given!");
        return false;
    }

    JsonWriter writer;
    writer.beginObject();
    writer.key("subnetIpAddress");
    writer.value(subnetIpAddress);
    writer.endObject();

    string result;
    status = JsonRpcClient::call("allowSubnet", writer.result(), result);

    if (!status) {
        Log::Error ("allowSubnet RPC call failed!");
        return false;
    }
    if (verbose_s) {
        cout << "Allow subnet request successful.\n";
    }

    return true;
}



bool
performListExcludedHosts ()
{
    Log::Debug ("performListExcludedHosts invoked.");

    string result;
    bool status = JsonRpcClient::call("listExcludedHosts", "{}", result);

    if (!status) {
        Log::Error ("listExcludedHosts RPC call failed!");
        return false;
    }

    // Parse the hosts array from result
    // Result is: {"hosts":["ip1","ip2",...]}
    // Simple extraction: find each quoted string inside the array
    if (verbose_s) {
        cout << "Request successful.\n";
        cout << "- Excluded hosts:\n";
    }

    // Find the array content
    ulong arrStart = result.find('[');
    ulong arrEnd = result.rfind(']');
    if (arrStart != string::npos && arrEnd != string::npos && arrEnd > arrStart)
    {
        string arrContent = result.substr(arrStart + 1, arrEnd - arrStart - 1);
        ulong pos = 0;
        while (pos < arrContent.length())
        {
            ulong qStart = arrContent.find('"', pos);
            if (qStart == string::npos)
                break;
            ulong qEnd = arrContent.find('"', qStart + 1);
            if (qEnd == string::npos)
                break;
            string host = arrContent.substr(qStart + 1, qEnd - qStart - 1);
            if (verbose_s)
                cout << "   " << host << endl;
            else
                cout << host << endl;
            pos = qEnd + 1;
        }
    }

    if (verbose_s)
        cout << "---\n\n";

    return true;
}



bool
performListExcludedSubnets ()
{
    Log::Debug ("performListExcludedSubnets invoked.");

    string result;
    bool status = JsonRpcClient::call("listExcludedSubnets", "{}", result);

    if (!status) {
        Log::Error ("listExcludedSubnets RPC call failed!");
        return false;
    }

    // Result contains {"subnets":[{"ipAddress":"...","netMask":"..."},...]]}
    // Parse using JsonReader on each object within the array
    if (verbose_s) {
        cout << "Request successful.\n";
        cout << "- Excluded subnets:\n";
    }

    // Simple parsing — find each {"ipAddress":"...","netMask":"..."} pair
    ulong searchPos = 0;
    while (searchPos < result.length())
    {
        ulong objStart = result.find('{', searchPos);
        if (objStart == string::npos)
            break;
        ulong objEnd = result.find('}', objStart);
        if (objEnd == string::npos)
            break;

        string objStr = result.substr(objStart, objEnd - objStart + 1);
        JsonReader objReader(objStr);

        string ipAddr;
        string netMask;
        if (objReader.getString("ipAddress", ipAddr) &&
            objReader.getString("netMask", netMask))
        {
            if (verbose_s) {
                cout << "   Subnet Address: " << ipAddr
                     << " \tSubnet Mask: " << netMask << endl;
            }
            else {
                cout << ipAddr << "/" << netMask << endl;
            }
        }

        searchPos = objEnd + 1;
    }

    if (verbose_s)
        cout << "---\n\n";

    return true;
}



// ---------------------------------------------------------------------------
//  Group commands
// ---------------------------------------------------------------------------

bool
performGetUserGroupList ()
{
    Log::Debug ("performGetUserGroupList invoked.");

    string result;
    bool status = JsonRpcClient::call("listGroups", "{}", result);

    if (!status) {
        Log::Error ("listGroups RPC call failed!");
        return false;
    }

    if (verbose_s) {
        cout << "Request successful.\n";
        cout << "Result: " << result << endl;
    }
    else {
        cout << result << endl;
    }

    return true;
}



bool
performCreateUserGroup ()
{
    Log::Debug ("performCreateUserGroup invoked.");

    string groupName;
    string description;

    bool status = Configuration::getValue ("Group Name", groupName);
    if (!status) {
        Log::Error ("No Group Name value given!");
        return false;
    }

    Configuration::getValue ("Description", description);

    JsonWriter writer;
    writer.beginObject();
    writer.key("name");
    writer.value(groupName);
    writer.key("description");
    writer.value(description);
    writer.endObject();

    string result;
    status = JsonRpcClient::call("createGroup", writer.result(), result);

    if (!status) {
        Log::Error ("createGroup RPC call failed!");
        return false;
    }

    JsonReader reader(result);
    ulong groupId = 0;
    reader.getUlong("groupId", groupId);

    if (verbose_s) {
        cout << "Request successful.\n";
        cout << "Group ID: " << groupId << endl;
    }
    else {
        cout << groupId << endl;
    }

    return true;
}



bool
performDestroyUserGroup ()
{
    Log::Debug ("performDestroyUserGroup invoked.");

    string groupIdStr;
    bool status = Configuration::getValue ("Group ID", groupIdStr);
    if (!status) {
        Log::Error ("No Group ID value given!");
        return false;
    }

    ulong groupId = strtoul(groupIdStr.c_str(), 0, 0);

    JsonWriter writer;
    writer.beginObject();
    writer.key("groupId");
    writer.value(groupId);
    writer.endObject();

    string result;
    status = JsonRpcClient::call("deleteGroup", writer.result(), result);

    if (!status) {
        Log::Error ("deleteGroup RPC call failed!");
        return false;
    }
    if (verbose_s) {
        cout << "Destroy group successful.\n";
    }

    return true;
}



bool
performGetPeerUserGroupList ()
{
    Log::Debug ("performGetPeerUserGroupList invoked.");

    string groupIdStr;
    bool status = Configuration::getValue ("Group ID", groupIdStr);
    if (!status) {
        Log::Error ("No Group ID value given!");
        return false;
    }

    ulong groupId = strtoul(groupIdStr.c_str(), 0, 0);

    JsonWriter writer;
    writer.beginObject();
    writer.key("groupId");
    writer.value(groupId);
    writer.endObject();

    string result;
    status = JsonRpcClient::call("getGroupPeerList", writer.result(), result);

    if (!status) {
        Log::Error ("getGroupPeerList RPC call failed!");
        return false;
    }

    if (verbose_s) {
        cout << "Request successful.\n";
        cout << "Result: " << result << endl;
    }
    else {
        cout << result << endl;
    }

    return true;
}



bool
performAddPeerToGroup ()
{
    Log::Debug ("performAddPeerToGroup invoked.");

    string groupIdStr;
    string peerIdStr;

    bool status = Configuration::getValue ("Group ID", groupIdStr);
    if (!status) {
        Log::Error ("No Group ID value given!");
        return false;
    }

    status = Configuration::getValue ("Peer ID", peerIdStr);
    if (!status) {
        Log::Error ("No Peer ID value given!");
        return false;
    }

    ulong groupId = strtoul(groupIdStr.c_str(), 0, 0);
    ulong peerId = strtoul(peerIdStr.c_str(), 0, 0);

    JsonWriter writer;
    writer.beginObject();
    writer.key("groupId");
    writer.value(groupId);
    writer.key("peerId");
    writer.value(peerId);
    writer.endObject();

    string result;
    status = JsonRpcClient::call("addPeerToGroup", writer.result(), result);

    if (!status) {
        Log::Error ("addPeerToGroup RPC call failed!");
        return false;
    }
    if (verbose_s) {
        cout << "Add peer to group successful.\n";
    }

    return true;
}



bool
performRemovePeerFromGroup ()
{
    Log::Debug ("performRemovePeerFromGroup invoked.");

    string groupIdStr;
    string peerIdStr;

    bool status = Configuration::getValue ("Group ID", groupIdStr);
    if (!status) {
        Log::Error ("No Group ID value given!");
        return false;
    }

    status = Configuration::getValue ("Peer ID", peerIdStr);
    if (!status) {
        Log::Error ("No Peer ID value given!");
        return false;
    }

    ulong groupId = strtoul(groupIdStr.c_str(), 0, 0);
    ulong peerId = strtoul(peerIdStr.c_str(), 0, 0);

    JsonWriter writer;
    writer.beginObject();
    writer.key("groupId");
    writer.value(groupId);
    writer.key("peerId");
    writer.value(peerId);
    writer.endObject();

    string result;
    status = JsonRpcClient::call("removePeerFromGroup", writer.result(), result);

    if (!status) {
        Log::Error ("removePeerFromGroup RPC call failed!");
        return false;
    }
    if (verbose_s) {
        cout << "Remove peer from group successful.\n";
    }

    return true;
}



bool
performGetExtendedPeerList ()
{
    Log::Debug ("performGetExtendedPeerList invoked.");

    string result;
    bool status = JsonRpcClient::call("getAllPeers", "{}", result);

    if (!status) {
        Log::Error ("getAllPeers RPC call failed!");
        return false;
    }

    if (verbose_s) {
        cout << "Request successful.\n";
        cout << "Result: " << result << endl;
    }
    else {
        cout << result << endl;
    }

    return true;
}



// ---------------------------------------------------------------------------
//  Query commands
// ---------------------------------------------------------------------------

bool
performStartQuery ()
{
    Log::Debug ("performStartQuery invoked.");

    string queryString;
    bool status = Configuration::getValue ("Query String", queryString);
    if (!status) {
        Log::Error ("No Query String value given!");
        return false;
    }

    JsonWriter writer;
    writer.beginObject();
    writer.key("queryString");
    writer.value(queryString);

    string groupName;
    if (Configuration::getValue ("Group Name", groupName)) {
        writer.key("groupName");
        writer.value(groupName);
    }

    string autoHaltStr;
    if (Configuration::getValue ("Auto Halt Limit", autoHaltStr)) {
        writer.key("autoHaltLimit");
        writer.value(strtoul(autoHaltStr.c_str(), 0, 0));
    }

    string peerDescStr;
    if (Configuration::getValue ("Peer Description Limit", peerDescStr)) {
        writer.key("peerDescMax");
        writer.value(strtoul(peerDescStr.c_str(), 0, 0));
    }

    writer.endObject();

    string result;
    status = JsonRpcClient::call("startQuery", writer.result(), result);

    if (!status) {
        Log::Error ("startQuery RPC call failed!");
        return false;
    }

    JsonReader reader(result);
    ulong queryId = 0;
    reader.getUlong("queryId", queryId);

    if (verbose_s) {
        cout << "Query start successful.\n";
        cout << "Query ID: " << queryId << endl;
    }
    else {
        cout << queryId << endl;
    }

    return true;
}



bool
performGetQueryStatus ()
{
    Log::Debug ("performGetQueryStatus invoked.");

    string queryIdStr;
    bool status = Configuration::getValue ("Query ID", queryIdStr);
    if (!status) {
        Log::Error ("No Query ID value given!");
        return false;
    }

    ulong queryId = strtoul(queryIdStr.c_str(), 0, 0);

    JsonWriter writer;
    writer.beginObject();
    writer.key("queryId");
    writer.value(queryId);
    writer.endObject();

    string result;
    status = JsonRpcClient::call("getQueryStatus", writer.result(), result);

    if (!status) {
        Log::Error ("getQueryStatus RPC call failed!");
        return false;
    }

    JsonReader reader(result);
    ulong totalPeers = 0, peersQueried = 0, numResponses = 0, totalHits = 0;
    reader.getUlong("totalPeers", totalPeers);
    reader.getUlong("peersQueried", peersQueried);
    reader.getUlong("numPeerResponses", numResponses);
    reader.getUlong("totalHits", totalHits);

    if (verbose_s) {
        cout << "Request successful.\n";
        cout << "- Query Status:\n";
        cout << "Total Peers: " << totalPeers << endl;
        cout << "Peers Queried: " << peersQueried << endl;
        cout << "Peer Responses: " << numResponses << endl;
        cout << "Total Hits: " << totalHits << endl;
        cout << "---\n\n";
    }
    else {
        cout << totalPeers << " ";
        cout << peersQueried << " ";
        cout << numResponses << " ";
        cout << totalHits << endl;
    }

    return true;
}



bool
performPauseQuery ()
{
    Log::Debug ("performPauseQuery invoked.");

    string queryIdStr;
    bool status = Configuration::getValue ("Query ID", queryIdStr);
    if (!status) {
        Log::Error ("No Query ID value given!");
        return false;
    }

    ulong queryId = strtoul(queryIdStr.c_str(), 0, 0);

    JsonWriter writer;
    writer.beginObject();
    writer.key("queryId");
    writer.value(queryId);
    writer.endObject();

    string result;
    status = JsonRpcClient::call("pauseQuery", writer.result(), result);

    if (!status) {
        Log::Error ("pauseQuery RPC call failed!");
        return false;
    }
    if (verbose_s) {
        cout << "Query pause successful.\n";
    }

    return true;
}



bool
performResumeQuery ()
{
    Log::Debug ("performResumeQuery invoked.");

    string queryIdStr;
    bool status = Configuration::getValue ("Query ID", queryIdStr);
    if (!status) {
        Log::Error ("No Query ID value given!");
        return false;
    }

    ulong queryId = strtoul(queryIdStr.c_str(), 0, 0);

    JsonWriter writer;
    writer.beginObject();
    writer.key("queryId");
    writer.value(queryId);
    writer.endObject();

    string result;
    status = JsonRpcClient::call("resumeQuery", writer.result(), result);

    if (!status) {
        Log::Error ("resumeQuery RPC call failed!");
        return false;
    }
    if (verbose_s) {
        cout << "Query resume successful.\n";
    }

    return true;
}



bool
performCancelQuery ()
{
    Log::Debug ("performCancelQuery invoked.");

    string queryIdStr;
    bool status = Configuration::getValue ("Query ID", queryIdStr);
    if (!status) {
        Log::Error ("No Query ID value given!");
        return false;
    }

    ulong queryId = strtoul(queryIdStr.c_str(), 0, 0);

    JsonWriter writer;
    writer.beginObject();
    writer.key("queryId");
    writer.value(queryId);
    writer.endObject();

    string result;
    status = JsonRpcClient::call("cancelQuery", writer.result(), result);

    if (!status) {
        Log::Error ("cancelQuery RPC call failed!");
        return false;
    }
    if (verbose_s) {
        cout << "Query cancel successful.\n";
    }

    return true;
}



bool
performGetQueryResults ()
{
    Log::Debug ("performGetQueryResults invoked.");

    string queryIdStr;
    bool status = Configuration::getValue ("Query ID", queryIdStr);
    if (!status) {
        Log::Error ("No Query ID value given!");
        return false;
    }

    ulong queryId = strtoul(queryIdStr.c_str(), 0, 0);

    JsonWriter writer;
    writer.beginObject();
    writer.key("queryId");
    writer.value(queryId);
    writer.endObject();

    string result;
    status = JsonRpcClient::call("getQueryResults", writer.result(), result);

    if (!status) {
        Log::Error ("getQueryResults RPC call failed!");
        return false;
    }

    if (verbose_s) {
        cout << "Request successful.\n";
        cout << "Result: " << result << endl;
    }
    else {
        cout << result << endl;
    }

    return true;
}



// ---------------------------------------------------------------------------
//  Module commands
// ---------------------------------------------------------------------------

bool
performRegisterModule ()
{
    Log::Debug ("performRegisterModule invoked.");

    string libPath;
    string symbol;

    bool status = Configuration::getValue ("Module Lib Path", libPath);
    if (!status) {
        Log::Error ("No Module Lib Path value given!");
        return false;
    }

    status = Configuration::getValue ("Module Symbol", symbol);
    if (!status) {
        Log::Error ("No Module Symbol value given!");
        return false;
    }

    JsonWriter writer;
    writer.beginObject();
    writer.key("libraryPath");
    writer.value(libPath);
    writer.key("bootstrapSymbol");
    writer.value(symbol);
    writer.endObject();

    string result;
    status = JsonRpcClient::call("registerModule", writer.result(), result);

    if (!status) {
        Log::Error ("registerModule RPC call failed!");
        return false;
    }

    JsonReader reader(result);
    ulong moduleId = 0;
    reader.getUlong("moduleId", moduleId);

    if (verbose_s) {
        cout << "Request successful.\n";
        cout << "Module ID: " << moduleId << endl;
    }
    else {
        cout << moduleId << endl;
    }

    return true;
}



bool
performUnregisterModule ()
{
    Log::Debug ("performUnregisterModule invoked.");

    string moduleIdStr;
    bool status = Configuration::getValue ("Module ID", moduleIdStr);
    if (!status) {
        Log::Error ("No Module ID value given!");
        return false;
    }

    ulong moduleId = strtoul(moduleIdStr.c_str(), 0, 0);

    JsonWriter writer;
    writer.beginObject();
    writer.key("moduleId");
    writer.value(moduleId);
    writer.endObject();

    string result;
    status = JsonRpcClient::call("unregisterModule", writer.result(), result);

    if (!status) {
        Log::Error ("unregisterModule RPC call failed!");
        return false;
    }
    if (verbose_s) {
        cout << "Request successful.\n";
    }

    return true;
}



bool
performLoadModule ()
{
    Log::Debug ("performLoadModule invoked.");

    string moduleIdStr;
    bool status = Configuration::getValue ("Module ID", moduleIdStr);
    if (!status) {
        Log::Error ("No Module ID value given!");
        return false;
    }

    ulong moduleId = strtoul(moduleIdStr.c_str(), 0, 0);

    JsonWriter writer;
    writer.beginObject();
    writer.key("moduleId");
    writer.value(moduleId);
    writer.endObject();

    string result;
    status = JsonRpcClient::call("loadModule", writer.result(), result);

    if (!status) {
        Log::Error ("loadModule RPC call failed!");
        return false;
    }
    if (verbose_s) {
        cout << "Request successful.\n";
    }

    return true;
}



bool
performUnloadModule ()
{
    Log::Debug ("performUnloadModule invoked.");

    string moduleIdStr;
    bool status = Configuration::getValue ("Module ID", moduleIdStr);
    if (!status) {
        Log::Error ("No Module ID value given!");
        return false;
    }

    ulong moduleId = strtoul(moduleIdStr.c_str(), 0, 0);

    JsonWriter writer;
    writer.beginObject();
    writer.key("moduleId");
    writer.value(moduleId);
    writer.endObject();

    string result;
    status = JsonRpcClient::call("unloadModule", writer.result(), result);

    if (!status) {
        Log::Error ("unloadModule RPC call failed!");
        return false;
    }
    if (verbose_s) {
        cout << "Request successful.\n";
    }

    return true;
}



bool
performListActiveModules ()
{
    Log::Debug ("performListActiveModules invoked.");

    string result;
    bool status = JsonRpcClient::call("listActiveModules", "{}", result);

    if (!status) {
        Log::Error ("listActiveModules RPC call failed!");
        return false;
    }

    if (verbose_s) {
        cout << "Request successful.\n";
        cout << "Result: " << result << endl;
    }
    else {
        cout << result << endl;
    }

    return true;
}



bool
performListAllModules ()
{
    Log::Debug ("performListAllModules invoked.");

    string result;
    bool status = JsonRpcClient::call("listAllModules", "{}", result);

    if (!status) {
        Log::Error ("listAllModules RPC call failed!");
        return false;
    }

    if (verbose_s) {
        cout << "Request successful.\n";
        cout << "Result: " << result << endl;
    }
    else {
        cout << result << endl;
    }

    return true;
}



bool
performGetModuleInfo ()
{
    Log::Debug ("performGetModuleInfo invoked.");

    string moduleIdStr;
    bool status = Configuration::getValue ("Module ID", moduleIdStr);
    if (!status) {
        Log::Error ("No Module ID value given!");
        return false;
    }

    ulong moduleId = strtoul(moduleIdStr.c_str(), 0, 0);

    JsonWriter writer;
    writer.beginObject();
    writer.key("moduleId");
    writer.value(moduleId);
    writer.endObject();

    string result;
    status = JsonRpcClient::call("getModuleInfo", writer.result(), result);

    if (!status) {
        Log::Error ("getModuleInfo RPC call failed!");
        return false;
    }

    JsonReader reader(result);
    string modName, modDesc, modVer, modLib, modSym;
    ulong modId = 0, activeTime = 0;
    reader.getUlong("moduleId", modId);
    reader.getString("moduleName", modName);
    reader.getString("description", modDesc);
    reader.getString("version", modVer);
    reader.getString("libraryPath", modLib);
    reader.getString("bootstrapSymbol", modSym);
    reader.getUlong("activeTime", activeTime);

    if (verbose_s) {
        cout << "Request successful.\n";
        cout << "- Module Info:\n";
        cout << "Module ID: " << modId << endl;
        cout << "Name: " << modName << endl;
        cout << "Description: " << modDesc << endl;
        cout << "Version: " << modVer << endl;
        cout << "Library: " << modLib << endl;
        cout << "Symbol: " << modSym << endl;
        cout << "Active Time: " << activeTime << endl;
        cout << "---\n\n";
    }
    else {
        cout << modId << " " << modName << " " << modVer << endl;
    }

    return true;
}



// ---------------------------------------------------------------------------
//  Status commands
// ---------------------------------------------------------------------------

bool
performGetStatus ()
{
    Log::Debug ("performGetStatus invoked.");

    string result;
    bool status = JsonRpcClient::call("getStatus", "{}", result);

    if (!status) {
        Log::Error ("getStatus RPC call failed!");
        return false;
    }

    JsonReader reader(result);
    string serverStatus;
    string version;
    reader.getString("status", serverStatus);
    reader.getString("version", version);

    if (verbose_s) {
        cout << "Request successful.\n";
        cout << "- Server Status:\n";
        cout << "Status: " << serverStatus << endl;
        cout << "Version: " << version << endl;
        cout << "---\n\n";
    }
    else {
        cout << serverStatus << " " << version << endl;
    }

    return true;
}


