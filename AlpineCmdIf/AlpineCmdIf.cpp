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



#include <time.h>
#include <stdlib.h>

#include <Log.h>
#include <StringUtils.h>
#include <NetUtils.h>
#include <OptHash.h>

#include <ApplCore.h>
#include <Configuration.h>

#include <CorbaServant.h>
#include <DtcpPeerMgmtIntf.h>

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
    string  interfaceContext;
    status = Configuration::getValue ("Interface Context", interfaceContext);

    if (!status) {
        Log::Error ("No Interface Context value!  Exiting.");
        return 1;
    }

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
               "\nInterface Context: "s + interfaceContext +
               "\nCommand: "s +  command +
               "\n"s + verboseValue +
               "\n");



    // start CORBA interface
    //
    status = CorbaServant::initialize (argc, argv);

    if (!status) {
        string msg("Initializing CorbaServant failed.  Exiting.");
        Log::Error (msg);

        if (verbose_s) {
            cout << msg << endl;
        }

        return 1;
    }
    status = CorbaServant::intializeAlpineClientInterface (interfaceContext);

    if (!status) {
        string  msg("Initialization of client CORBA interface failed.  Exiting.");
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
bool  performNatDiscovery ();
bool  performNatDiscoveryQuery (); // required?
bool  performSetDataSendingLimit ();
bool  performGetDataSendingLimit ();
bool  performSetStackThreadLimit ();
bool  performGetStackThreadLimit ();
bool  performSetReceiveBufferLimit ();
bool  performGetReceiveBufferLimit ();
bool  performSetSendBufferLimit ();
bool  performGetSendBufferLimit ();
bool  performGetBufferStats ();
bool  performSetTotalTransferLimit ();
bool  performGetTotalTransferLimit ();
bool  performSetPeerTransferLimit ();
bool  performGetPeerTransferLimit ();
bool  performGetTransferStats ();
bool  performGetUserGroupList ();
bool  performCreateUserGroup ();
bool  performDestroyUserGroup ();
bool  performGetPeerUserGroupList ();
bool  performAddPeerToGroup ();
bool  performRemovePeerFromGroup ();
bool  performGetExtendedPeerList ();
bool  performGetPeerInformation ();
bool  performUpdatePeerInformation ();
bool  performSetDefaultQueryOptions ();
bool  performGetDefaultQueryOptions ();
bool  performStartQuery ();
bool  performGetQueryStatus ();
bool  performPauseQuery ();
bool  performResumeQuery ();
bool  performCancelQuery ();
bool  performGetQueryResults ();
bool  performRegisterModule ();
bool  performUnregisterModule ();
bool  performSetModuleConfiguration ();
bool  performGetModuleConfiguration ();
bool  performLoadModule ();
bool  performUnloadModule ();
bool  performListActiveModules ();
bool  performListAllModules ();


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
    dispatchTable_s->emplace ("natDiscovery", &performNatDiscovery);
    dispatchTable_s->emplace ("natDiscoveryQuery", &performNatDiscoveryQuery);
    dispatchTable_s->emplace ("setDataSendingLimit", &performSetDataSendingLimit);
    dispatchTable_s->emplace ("getDataSendingLimit", &performGetDataSendingLimit);
    dispatchTable_s->emplace ("setStackThreadLimit", &performSetStackThreadLimit);
    dispatchTable_s->emplace ("getStackThreadLimit", &performGetStackThreadLimit);
    dispatchTable_s->emplace ("setReceiveBufferLimit", &performSetReceiveBufferLimit);
    dispatchTable_s->emplace ("getReceiveBufferLimit", &performGetReceiveBufferLimit);
    dispatchTable_s->emplace ("setSendBufferLimit", &performSetSendBufferLimit);
    dispatchTable_s->emplace ("getSendBufferLimit", &performGetSendBufferLimit);
    dispatchTable_s->emplace ("getBufferStats", &performGetBufferStats);
    dispatchTable_s->emplace ("setTotalTransferLimit", &performSetTotalTransferLimit);
    dispatchTable_s->emplace ("getTotalTransferLimit", &performGetTotalTransferLimit);
    dispatchTable_s->emplace ("setPeerTransferLimit", &performSetPeerTransferLimit);
    dispatchTable_s->emplace ("getPeerTransferLimit", &performGetPeerTransferLimit);
    dispatchTable_s->emplace ("getTransferStats", &performGetTransferStats);
    dispatchTable_s->emplace ("getUserGroupList", &performGetUserGroupList);
    dispatchTable_s->emplace ("createUserGroup", &performCreateUserGroup);
    dispatchTable_s->emplace ("destroyUserGroup", &performDestroyUserGroup);
    dispatchTable_s->emplace ("getPeerUserGroupList", &performGetPeerUserGroupList);
    dispatchTable_s->emplace ("addPeerToGroup", &performAddPeerToGroup);
    dispatchTable_s->emplace ("removePeerFromGroup", &performRemovePeerFromGroup);
    dispatchTable_s->emplace ("getExtendedPeerList", &performGetExtendedPeerList);
    dispatchTable_s->emplace ("getPeerInformation", &performGetPeerInformation);
    dispatchTable_s->emplace ("updatePeerInformation", &performUpdatePeerInformation);
    dispatchTable_s->emplace ("setDefaultQueryOptions", &performSetDefaultQueryOptions);
    dispatchTable_s->emplace ("getDefaultQueryOptions", &performGetDefaultQueryOptions);
    dispatchTable_s->emplace ("beginQuery", &performStartQuery);
    dispatchTable_s->emplace ("getQueryStatus", &performGetQueryStatus);
    dispatchTable_s->emplace ("pauseQuery", &performPauseQuery);
    dispatchTable_s->emplace ("resumeQuery", &performResumeQuery);
    dispatchTable_s->emplace ("cancelQuery", &performCancelQuery);
    dispatchTable_s->emplace ("getQueryResults", &performGetQueryResults);
    dispatchTable_s->emplace ("registerModule", &performRegisterModule);
    dispatchTable_s->emplace ("unregisterModule", &performUnregisterModule);
    dispatchTable_s->emplace ("setModuleConfiguration", &performSetModuleConfiguration);
    dispatchTable_s->emplace ("getModuleConfiguration", &performGetModuleConfiguration);
    dispatchTable_s->emplace ("loadModule", &performLoadModule);
    dispatchTable_s->emplace ("unloadModule", &performUnloadModule);
    dispatchTable_s->emplace ("listActiveModules", &performListActiveModules);
    dispatchTable_s->emplace ("listAllModules", &performListAllModules);


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



bool  
performAddDtcpPeer ()
{
    Log::Debug ("performAddDtcpPeer invoked.");

    string  ipAddress;
    string  portStr;
    ushort  port;

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
    port = atoi (portStr.c_str());
    port = htons(port);


    status = DtcpPeerMgmtIntf::addDtcpPeer (ipAddress, port);

    if (!status) {
        Log::Error ("DtcpPeerMgmtIntf::addDtcpPeer call failed!");
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
    ushort  port;

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
    port = atoi (portStr.c_str());
    port = htons(port);

    ulong  peerId;
    status = DtcpPeerMgmtIntf::getDtcpPeerId (ipAddress,
                                              port,
                                              peerId);

    if (!status) {
        Log::Error ("DtcpPeerMgmtIntf::getDtcpPeerId call failed!");
        return false;
    }
    // Display result of operation
    //
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


    DtcpPeerMgmtIntf::t_DtcpPeerStatus  dtcpStatus;
    status = DtcpPeerMgmtIntf::getDtcpPeerStatus (peerId, dtcpStatus);

    if (!status) {
        Log::Error ("DtcpPeerMgmtIntf::getDtcpPeerStatus call failed!");
        return false;
    }
    // Display result of operation
    //
    if (verbose_s) {
        cout << "Request successful.\n";
        cout << "- Dtcp Peer Status:\n";
        cout << "IpAddress: " << dtcpStatus.ipAddress << endl;
        cout << "Port: " << dtcpStatus.port << endl;
        cout << "Last Receive Time: " << ctime ((time_t *)&dtcpStatus.lastRecvTime) << endl;
        cout << "Last Send Time: " << ctime ((time_t *)&dtcpStatus.lastSendTime) << endl;
        cout << "Average Bandwidth: " << dtcpStatus.avgBandwidth << " Kbps" << endl;
        cout << "Peak Bandwidth: " << dtcpStatus.peakBandwidth << " Kbps" << endl;
        cout << "---\n\n";
    }
    else {
        cout << dtcpStatus.ipAddress << " ";
        cout << dtcpStatus.port << " ";
        cout << dtcpStatus.lastRecvTime << " ";
        cout << dtcpStatus.lastSendTime << " ";
        cout << dtcpStatus.avgBandwidth << " ";
        cout << dtcpStatus.peakBandwidth << " ";
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


    status = DtcpPeerMgmtIntf::activateDtcpPeer (peerId);

    if (!status) {
        Log::Error ("DtcpPeerMgmtIntf::activateDtcpPeer call failed!");
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


    status = DtcpPeerMgmtIntf::deactivateDtcpPeer (peerId);

    if (!status) {
        Log::Error ("DtcpPeerMgmtIntf::deactivateDtcpPeer call failed!");
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


    status = DtcpPeerMgmtIntf::pingDtcpPeer (peerId);

    if (!status) {
        Log::Error ("DtcpPeerMgmtIntf::pingDtcpPeer call failed!");
        return false;
    }
    if (verbose_s) {
        cout << "Ping request successful.\n";
    }

    return true;
}



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
    status = DtcpPeerMgmtIntf::excludeHost (ipAddress);

    if (!status) {
        Log::Error ("DtcpPeerMgmtIntf::excludeHost call failed!");
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
    status = DtcpPeerMgmtIntf::excludeSubnet (subnetIpAddress, subnetMask);
    
    if (!status) {
        Log::Error ("DtcpPeerMgmtIntf::excludeSubnet call failed!");
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
    status = DtcpPeerMgmtIntf::allowHost (ipAddress);

    if (!status) {
        Log::Error ("DtcpPeerMgmtIntf::allowHost call failed!");
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
    status = DtcpPeerMgmtIntf::allowSubnet (subnetIpAddress);
    
    if (!status) {
        Log::Error ("DtcpPeerMgmtIntf::allowSubnet call failed!");
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

    bool status;
    DtcpPeerMgmtIntf::t_IpAddressList  hostList;

    status = DtcpPeerMgmtIntf::listExcludedHosts (hostList);

    if (!status) {
        Log::Error ("DtcpPeerMgmtIntf::listExcludedHosts call failed!");
        return false;
    }
    // Display result of operation
    //
    if (verbose_s) {
        cout << "Request successful.\n";
        cout << "- Excluded hosts:\n";

        for (auto& host : hostList) {
            cout << "   " << host << endl;
        }

        cout << "---\n\n";
    }
    else {
        for (auto& host : hostList) {
            cout << host << endl;
        }
        return;
    }


    return true;
}



bool  
performListExcludedSubnets ()
{
    Log::Debug ("performListExcludedSubnets invoked.");

    bool status;
    DtcpPeerMgmtIntf::t_SubnetAddressList  subnetList;

    status = DtcpPeerMgmtIntf::listExcludedSubnets (subnetList);

    if (!status) {
        Log::Error ("DtcpPeerMgmtIntf::listExcludedSubnets call failed!");
        return false;
    }
    // Display result of operation
    //
    if (verbose_s) {
        cout << "Request successful.\n";
        cout << "- Excluded subnets:\n";

        for (auto& subnet : subnetList) {
            cout << "   Subnet Address: " << subnet.ipAddress
                 << " \tSubnet Mask: " << subnet.netMask << endl;
        }

        cout << "---\n\n";
    }
    else {
        for (auto& subnet : subnetList) {
            cout << subnet.ipAddress << "/" << subnet.netMask << endl;
        }
        return;
    }

    return true;
}



bool  
performNatDiscovery ()
{
    Log::Debug ("performNatDiscovery invoked.");


    if (verbose_s) {
        cout << "NAT discovery request successful.\n";
    }

    return true;
}



bool  
performNatDiscoveryQuery ()
{
    Log::Debug ("performNatDiscoveryQuery invoked.");


    // Display result of operation
    //
    if (verbose_s) {
        cout << "Request successful.\n";
    }
    else {
        cout << endl;
    }

    return true;
}



bool  
performSetDataSendingLimit ()
{
    Log::Debug ("performSetDataSendingLimit invoked.");


    if (verbose_s) {
        cout << "Request successful.\n";
    }

    return true;
}



bool  
performGetDataSendingLimit ()
{
    Log::Debug ("performGetDataSendingLimit invoked.");


    // Display result of operation
    //
    if (verbose_s) {
        cout << "Request successful.\n";
    }
    else {
        cout << endl;
    }

    return true;
}



bool  
performSetStackThreadLimit ()
{
    Log::Debug ("performSetStackThreadLimit invoked.");


    if (verbose_s) {
        cout << "Request successful.\n";
    }

    return true;
}



bool  
performGetStackThreadLimit ()
{
    Log::Debug ("performGetStackThreadLimit invoked.");


    // Display result of operation
    //
    if (verbose_s) {
        cout << "Request successful.\n";
    }
    else {
        cout << endl;
    }

    return true;
}



bool  
performSetReceiveBufferLimit ()
{
    Log::Debug ("performSetReceiveBufferLimit invoked.");


    if (verbose_s) {
        cout << "Request successful.\n";
    }

    return true;
}



bool  
performGetReceiveBufferLimit ()
{
    Log::Debug ("performGetReceiveBufferLimit invoked.");


    // Display result of operation
    //
    if (verbose_s) {
        cout << "Request successful.\n";
    }
    else {
        cout << endl;
    }

    return true;
}



bool  
performSetSendBufferLimit ()
{
    Log::Debug ("performSetSendBufferLimit invoked.");


    if (verbose_s) {
        cout << "Request successful.\n";
    }

    return true;
}



bool  
performGetSendBufferLimit ()
{
    Log::Debug ("performGetSendBufferLimit invoked.");


    // Display result of operation
    //
    if (verbose_s) {
        cout << "Request successful.\n";
    }
    else {
        cout << endl;
    }

    return true;
}



bool  
performGetBufferStats ()
{
    Log::Debug ("performGetBufferStats invoked.");


    // Display result of operation
    //
    if (verbose_s) {
        cout << "Request successful.\n";
    }
    else {
        cout << endl;
    }

    return true;
}



bool  
performSetTotalTransferLimit ()
{
    Log::Debug ("performSetTotalTransferLimit invoked.");


    if (verbose_s) {
        cout << "Request successful.\n";
    }

    return true;
}



bool  
performGetTotalTransferLimit ()
{
    Log::Debug ("performGetTotalTransferLimit invoked.");


    // Display result of operation
    //
    if (verbose_s) {
        cout << "Request successful.\n";
    }
    else {
        cout << endl;
    }

    return true;
}



bool  
performSetPeerTransferLimit ()
{
    Log::Debug ("performSetPeerTransferLimit invoked.");


    if (verbose_s) {
        cout << "Request successful.\n";
    }

    return true;
}



bool  
performGetPeerTransferLimit ()
{
    Log::Debug ("performGetPeerTransferLimit invoked.");


    // Display result of operation
    //
    if (verbose_s) {
        cout << "Request successful.\n";
    }
    else {
        cout << endl;
    }

    return true;
}



bool  
performGetTransferStats ()
{
    Log::Debug ("performGetTransferStats invoked.");


    // Display result of operation
    //
    if (verbose_s) {
        cout << "Request successful.\n";
    }
    else {
        cout << endl;
    }

    return true;
}



bool  
performGetUserGroupList ()
{
    Log::Debug ("performGetUserGroupList invoked.");


    // Display result of operation
    //
    if (verbose_s) {
        cout << "Request successful.\n";
    }
    else {
        cout << endl;
    }

    return true;
}



bool  
performCreateUserGroup ()
{
    Log::Debug ("performCreateUserGroup invoked.");


    // Display result of operation
    //
    if (verbose_s) {
        cout << "Request successful.\n";
    }
    else {
        cout << endl;
    }

    return true;
}



bool  
performDestroyUserGroup ()
{
    Log::Debug ("performDestroyUserGroup invoked.");


    if (verbose_s) {
        cout << "Destroy group successful.\n";
    }

    return true;
}



bool  
performGetPeerUserGroupList ()
{
    Log::Debug ("performGetPeerUserGroupList invoked.");


    // Display result of operation
    //
    if (verbose_s) {
        cout << "Request successful.\n";
    }
    else {
        cout << endl;
    }

    return true;
}



bool  
performAddPeerToGroup ()
{
    Log::Debug ("performAddPeerToGroup invoked.");


    if (verbose_s) {
        cout << "Add peer to group successful.\n";
    }

    return true;
}



bool  
performRemovePeerFromGroup ()
{
    Log::Debug ("performRemovePeerFromGroup invoked.");


    if (verbose_s) {
        cout << "Remove peer from group successful.\n";
    }

    return true;
}



bool  
performGetExtendedPeerList ()
{
    Log::Debug ("performGetExtendedPeerList invoked.");


    // Display result of operation
    //
    if (verbose_s) {
        cout << "Request successful.\n";
    }
    else {
        cout << endl;
    }

    return true;
}



bool  
performGetPeerInformation ()
{
    Log::Debug ("performGetPeerInformation invoked.");


    // Display result of operation
    //
    if (verbose_s) {
        cout << "Request successful.\n";
    }
    else {
        cout << endl;
    }

    return true;
}



bool  
performUpdatePeerInformation ()
{
    Log::Debug ("performUpdatePeerInformation invoked.");


    if (verbose_s) {
        cout << "Peer update successful.\n";
    }

    return true;
}



bool
performSetDefaultQueryOptions ()
{
    Log::Debug ("performSetDefaultQueryOptions invoked.");


    if (verbose_s) {
        cout << "Set default query options successful.\n";
    }

    return true;
}



bool
performGetDefaultQueryOptions ()
{
    Log::Debug ("performGetDefaultQueryOptions invoked.");


    // Display result of operation
    //
    if (verbose_s) {
        cout << "Request successful.\n";
    }
    else {
        cout << endl;
    }

    return true;
}



bool  
performStartQuery ()
{
    Log::Debug ("performStartQuery invoked.");


    if (verbose_s) {
        cout << "Query start successful.\n";
    }

    return true;
}



bool  
performGetQueryStatus ()
{
    Log::Debug ("performGetQueryStatus invoked.");


    // Display result of operation
    //
    if (verbose_s) {
        cout << "Request successful.\n";
    }
    else {
        cout << endl;
    }

    return true;
}



bool  
performPauseQuery ()
{
    Log::Debug ("performPauseQuery invoked.");


    if (verbose_s) {
        cout << "Query pause successful.\n";
    }

    return true;
}



bool  
performResumeQuery ()
{
    Log::Debug ("performResumeQuery invoked.");


    if (verbose_s) {
        cout << "Query resume successful.\n";
    }

    return true;
}



bool  
performCancelQuery ()
{
    Log::Debug ("performCancelQuery invoked.");


    if (verbose_s) {
        cout << "Query cancel successful.\n";
    }

    return true;
}



bool  
performGetQueryResults ()
{
    Log::Debug ("performGetQueryResults invoked.");


    // Display result of operation
    //
    if (verbose_s) {
        cout << "Request successful.\n";
    }
    else {
        cout << endl;
    }

    return true;
}



bool  
performRegisterModule ()
{
    Log::Debug ("performRegisterModule invoked.");


    if (verbose_s) {
        cout << "Request successful.\n";
    }

    return true;
}



bool  
performUnregisterModule ()
{
    Log::Debug ("performUnregisterModule invoked.");


    if (verbose_s) {
        cout << "Request successful.\n";
    }

    return true;
}



bool  
performSetModuleConfiguration ()
{
    Log::Debug ("performSetModuleConfiguration invoked.");


    if (verbose_s) {
        cout << "Request successful.\n";
    }

    return true;
}



bool  
performGetModuleConfiguration ()
{
    Log::Debug ("performGetModuleConfiguration invoked.");


    // Display result of operation
    //
    if (verbose_s) {
        cout << "Request successful.\n";
    }
    else {
        cout << endl;
    }

    return true;
}



bool  
performLoadModule ()
{
    Log::Debug ("performLoadModule invoked.");


    if (verbose_s) {
        cout << "Request successful.\n";
    }

    return true;
}



bool  
performUnloadModule ()
{
    Log::Debug ("performUnloadModule invoked.");


    if (verbose_s) {
        cout << "Request successful.\n";
    }

    return true;
}



bool  
performListActiveModules ()
{
    Log::Debug ("performListActiveModules invoked.");


    // Display result of operation
    //
    if (verbose_s) {
        cout << "Request successful.\n";
    }
    else {
        cout << endl;
    }

    return true;
}



bool  
performListAllModules ()
{
    Log::Debug ("performListAllModules invoked.");


    // Display result of operation
    //
    if (verbose_s) {
        cout << "Request successful.\n";
    }
    else {
        cout << endl;
    }

    return true;
}



