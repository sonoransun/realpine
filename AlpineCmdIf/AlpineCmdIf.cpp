/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <algorithm>
#include <iostream>
#include <stdlib.h>
#include <time.h>

using std::cerr;
using std::cout;
using std::endl;

#include <Log.h>
#include <NetUtils.h>
#include <OptHash.h>
#include <StringUtils.h>

#include <ApplCore.h>
#include <Configuration.h>

#include <JsonReader.h>
#include <JsonRpcClient.h>
#include <JsonWriter.h>

#include <AlpineConfig.h>
#include <OutputFormatter.h>
#include <ShellCompletion.h>
#include <TermColor.h>

#include <linenoise.h>


static constexpr const char * ALPINE_CLI_VERSION = "devel-00019";


// Types for command dispatch table
//
using t_Handler = bool (*)();

struct t_CommandEntry
{
    t_Handler handler;
    string description;
    string detailedHelp;
};

using t_DispatchTable = std::unordered_map<string, t_CommandEntry, OptHash<string>, equal_to<string>>;


// Global members
//
static t_DispatchTable * dispatchTable_s = nullptr;
static bool verbose_s = false;
static bool jsonOutput_s = false;
static bool quietOutput_s = false;
static string formatMode_s = "json"s;
static bool colorEnabled_s = false;


bool buildDispatchTable();

bool commandExists(const string & command);

bool handleCommand(const string & command);

void printUsage();

void printCommandList();

void printCommandHelp(const string & command);

void printVersion();

vector<pair<string, string>> getDispatchEntries();

int runInteractive(const string & serverAddress, ushort serverPort);

void alpineCompletionCallback(const char * buf, linenoiseCompletions * lc);

static string historyFilePath();


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
main(int argc, char * argv[])
{
    bool status;

    status = buildDispatchTable();

    if (!status) {
        Log::Error("Building dispatch table failed!  Exiting.");
        return 1;
    }

    // Check for --help / -h, --version / -v, and --completions before
    // configuration init
    //
    for (int i = 1; i < argc; ++i) {
        string arg(argv[i]);
        if (arg == "--help" || arg == "-h") {
            printUsage();
            return 0;
        }
        if (arg == "--version" || arg == "-v") {
            printVersion();
            return 0;
        }
        if (arg == "--completions" && i + 1 < argc) {
            string shell(argv[i + 1]);
            auto entries = getDispatchEntries();
            if (shell == "bash") {
                cout << ShellCompletion::generateBash(entries);
            } else if (shell == "zsh") {
                cout << ShellCompletion::generateZsh(entries);
            } else {
                cerr << "Error: Unknown shell '"s + shell + "'. Use 'bash' or 'zsh'." << endl;
                return 2;
            }
            return 0;
        }
    }


    // Initialize application core
    //
    status = ApplCore::initialize(argc, argv);

    if (!status) {
        Log::Error("Unable to initialize application core!  Exiting.");
        return 1;
    }

    // Initialize configuration
    //
    ConfigData::t_ConfigElementList * configElements;

    AlpineConfig::createConfigElements();
    AlpineConfig::getConfigElements(configElements);

    status = Configuration::initialize(argc, argv, *configElements, AlpineConfig::configFile_s);

    if (!status) {
        Log::Error("Error initializing configuration!  Exiting.");
        return 1;
    }


    // Check for --json, --quiet, --format, and --color flags
    //
    for (int i = 1; i < argc; ++i) {
        string arg(argv[i]);
        if (arg == "--json")
            jsonOutput_s = true;
        if (arg == "--quiet")
            quietOutput_s = true;
        if (arg == "--format" && i + 1 < argc) {
            formatMode_s = string(argv[i + 1]);
            if (formatMode_s == "json"s)
                jsonOutput_s = true;
            ++i;
        }
        if (arg == "--color")
            colorEnabled_s = true;
    }

    // Auto-detect color when not explicitly set
    if (!colorEnabled_s)
        colorEnabled_s = TermColor::isTerminal();


    // Load configuration settings
    //
    string serverAddress;
    status = Configuration::getValue("Server Address", serverAddress);

    if (!status) {
        if (jsonOutput_s) {
            cerr << "{\"error\":\"No Server Address value\",\"code\":1}" << endl;
        } else {
            cerr << "Error: No Server Address value.  Exiting." << endl;
        }
        return 1;
    }

    string serverPortStr;
    status = Configuration::getValue("Server Port", serverPortStr);

    if (!status) {
        if (jsonOutput_s) {
            cerr << "{\"error\":\"No Server Port value\",\"code\":1}" << endl;
        } else {
            cerr << "Error: No Server Port value.  Exiting." << endl;
        }
        return 1;
    }

    ushort serverPort = static_cast<ushort>(atoi(serverPortStr.c_str()));

    string command;
    status = Configuration::getValue("Command", command);

    if (!status) {
        if (jsonOutput_s) {
            cerr << "{\"error\":\"No command value\",\"code\":2}" << endl;
        } else {
            cerr << "Error: No command value.  Exiting." << endl;
        }
        return 2;
    }

    string verboseValue;
    status = Configuration::getValue("Verbose", verboseValue);

    if (status) {
        verbose_s = true;
        verboseValue = "Verbose mode ON";
    } else {
        verboseValue = "Verbose mode OFF";
    }

    // Handle "help" command
    //
    if (command == "help") {
        // Check if there's additional arg for per-command help
        string helpArg;
        if (Configuration::getValue("Query String", helpArg) && !helpArg.empty()) {
            printCommandHelp(helpArg);
        } else {
            printCommandList();
        }
        return 0;
    }


    if (!quietOutput_s) {
        Log::Info("Starting ALPINE Command Interface-"s + "\nServer: "s + serverAddress + ":" + serverPortStr +
                  "\nCommand: "s + command + "\n"s + verboseValue + "\n");
    }


    // Initialize JSON-RPC client
    //
    status = JsonRpcClient::initialize(serverAddress, serverPort);

    if (!status) {
        if (jsonOutput_s) {
            cerr << "{\"error\":\"Initializing JsonRpcClient failed\",\"code\":1}" << endl;
        } else {
            cerr << "Error: Initializing JsonRpcClient failed.  Exiting." << endl;
        }
        Log::Error("Initializing JsonRpcClient failed.  Exiting.");
        return 1;
    }


    // Interactive REPL mode
    //
    if (command == "interactive"s) {
        return runInteractive(serverAddress, serverPort);
    }


    // Perform requested command...
    //
    status = commandExists(command);

    if (!status) {
        if (jsonOutput_s) {
            cerr << "{\"error\":\"Invalid command: "s + command + "\",\"code\":2}" << endl;
        } else {
            cerr << "Error: Invalid command '"s + command + "'.  Exiting." << endl;
        }
        Log::Error("Invalid command!  Exiting.");
        return 2;
    }

    status = handleCommand(command);

    if (!status) {
        return 3;
    }


    Log::Info("Command finished.  Exiting.");

    return 0;
}


// Methods for each supported command type
//
bool performAddDtcpPeer();
bool performGetDtcpPeerId();
bool performGetDtcpPeerStatus();
bool performActivateDtcpPeer();
bool performDeactivateDtcpPeer();
bool performPingDtcpPeer();
bool performExcludeHost();
bool performExcludeSubnet();
bool performAllowHost();
bool performAllowSubnet();
bool performListExcludedHosts();
bool performListExcludedSubnets();
bool performGetUserGroupList();
bool performCreateUserGroup();
bool performDestroyUserGroup();
bool performGetPeerUserGroupList();
bool performAddPeerToGroup();
bool performRemovePeerFromGroup();
bool performGetExtendedPeerList();
bool performStartQuery();
bool performGetQueryStatus();
bool performPauseQuery();
bool performResumeQuery();
bool performCancelQuery();
bool performGetQueryResults();
bool performRegisterModule();
bool performUnregisterModule();
bool performLoadModule();
bool performUnloadModule();
bool performListActiveModules();
bool performListAllModules();
bool performGetModuleInfo();
bool performGetStatus();


// Initialize dispatch table
//
bool
buildDispatchTable()
{
    dispatchTable_s = new t_DispatchTable;

    // Peer commands
    dispatchTable_s->emplace("addDtcpPeer",
                             t_CommandEntry{&performAddDtcpPeer,
                                            "Add a DTCP peer by IP and port"s,
                                            "Usage: --command addDtcpPeer --ipAddress <ip> --port <port>\n"
                                            "Registers a new peer at the given address."s});
    dispatchTable_s->emplace("getDtcpPeerId",
                             t_CommandEntry{&performGetDtcpPeerId,
                                            "Get the peer ID for a given address"s,
                                            "Usage: --command getDtcpPeerId --ipAddress <ip> --port <port>\n"
                                            "Returns the numeric peer ID."s});
    dispatchTable_s->emplace("getDtcpPeerStatus",
                             t_CommandEntry{&performGetDtcpPeerStatus,
                                            "Get status details for a peer"s,
                                            "Usage: --command getDtcpPeerStatus --peerId <id>\n"
                                            "Returns IP, port, bandwidth, and timing information."s});
    dispatchTable_s->emplace("activateDtcpPeer",
                             t_CommandEntry{&performActivateDtcpPeer,
                                            "Activate a peer connection"s,
                                            "Usage: --command activateDtcpPeer --peerId <id>\n"
                                            "Enables communication with the specified peer."s});
    dispatchTable_s->emplace("deactivateDtcpPeer",
                             t_CommandEntry{&performDeactivateDtcpPeer,
                                            "Deactivate a peer connection"s,
                                            "Usage: --command deactivateDtcpPeer --peerId <id>\n"
                                            "Disables communication with the specified peer."s});
    dispatchTable_s->emplace("pingDtcpPeer",
                             t_CommandEntry{&performPingDtcpPeer,
                                            "Ping a peer to check connectivity"s,
                                            "Usage: --command pingDtcpPeer --peerId <id>\n"
                                            "Sends a ping request to the specified peer."s});

    // Network filter commands
    dispatchTable_s->emplace("excludeHost",
                             t_CommandEntry{&performExcludeHost,
                                            "Exclude a host by IP address"s,
                                            "Usage: --command excludeHost --ipAddress <ip>\n"
                                            "Blocks a host from peer communication."s});
    dispatchTable_s->emplace(
        "excludeSubnet",
        t_CommandEntry{&performExcludeSubnet,
                       "Exclude a subnet"s,
                       "Usage: --command excludeSubnet --subnetIpAddress <ip> --subnetMask <mask>\n"
                       "Blocks an entire subnet from peer communication."s});
    dispatchTable_s->emplace("allowHost",
                             t_CommandEntry{&performAllowHost,
                                            "Allow a previously excluded host"s,
                                            "Usage: --command allowHost --ipAddress <ip>\n"
                                            "Removes a host from the exclusion list."s});
    dispatchTable_s->emplace("allowSubnet",
                             t_CommandEntry{&performAllowSubnet,
                                            "Allow a previously excluded subnet"s,
                                            "Usage: --command allowSubnet --subnetIpAddress <ip>\n"
                                            "Removes a subnet from the exclusion list."s});
    dispatchTable_s->emplace("listExcludedHosts",
                             t_CommandEntry{&performListExcludedHosts,
                                            "List all excluded hosts"s,
                                            "Usage: --command listExcludedHosts\n"
                                            "Displays all currently excluded host IP addresses."s});
    dispatchTable_s->emplace("listExcludedSubnets",
                             t_CommandEntry{&performListExcludedSubnets,
                                            "List all excluded subnets"s,
                                            "Usage: --command listExcludedSubnets\n"
                                            "Displays all currently excluded subnets."s});

    // Group commands
    dispatchTable_s->emplace("getUserGroupList",
                             t_CommandEntry{&performGetUserGroupList,
                                            "List all user groups"s,
                                            "Usage: --command getUserGroupList\n"
                                            "Returns the list of all defined user groups."s});
    dispatchTable_s->emplace(
        "createUserGroup",
        t_CommandEntry{&performCreateUserGroup,
                       "Create a new user group"s,
                       "Usage: --command createUserGroup --groupName <name> [--description <desc>]\n"
                       "Creates a group and returns its ID."s});
    dispatchTable_s->emplace("destroyUserGroup",
                             t_CommandEntry{&performDestroyUserGroup,
                                            "Delete a user group"s,
                                            "Usage: --command destroyUserGroup --groupId <id>\n"
                                            "Permanently removes the specified group."s});
    dispatchTable_s->emplace("getPeerUserGroupList",
                             t_CommandEntry{&performGetPeerUserGroupList,
                                            "List peers in a group"s,
                                            "Usage: --command getPeerUserGroupList --groupId <id>\n"
                                            "Returns peers belonging to the specified group."s});
    dispatchTable_s->emplace("addPeerToGroup",
                             t_CommandEntry{&performAddPeerToGroup,
                                            "Add a peer to a group"s,
                                            "Usage: --command addPeerToGroup --groupId <id> --peerId <id>\n"
                                            "Adds the specified peer to the group."s});
    dispatchTable_s->emplace("removePeerFromGroup",
                             t_CommandEntry{&performRemovePeerFromGroup,
                                            "Remove a peer from a group"s,
                                            "Usage: --command removePeerFromGroup --groupId <id> --peerId <id>\n"
                                            "Removes the specified peer from the group."s});
    dispatchTable_s->emplace("getExtendedPeerList",
                             t_CommandEntry{&performGetExtendedPeerList,
                                            "Get extended list of all peers"s,
                                            "Usage: --command getExtendedPeerList\n"
                                            "Returns detailed information for all known peers."s});

    // Query commands
    dispatchTable_s->emplace("beginQuery",
                             t_CommandEntry{&performStartQuery,
                                            "Start a new resource query"s,
                                            "Usage: --command beginQuery --queryString <query> [--groupName <group>]\n"
                                            "       [--autoHaltLimit <n>] [--peerDescriptionLimit <n>]\n"
                                            "Starts a distributed query and returns the query ID."s});
    dispatchTable_s->emplace("getQueryStatus",
                             t_CommandEntry{&performGetQueryStatus,
                                            "Get status of a running query"s,
                                            "Usage: --command getQueryStatus --queryId <id>\n"
                                            "Returns peer counts and hit totals."s});
    dispatchTable_s->emplace("pauseQuery",
                             t_CommandEntry{&performPauseQuery,
                                            "Pause a running query"s,
                                            "Usage: --command pauseQuery --queryId <id>\n"
                                            "Temporarily halts query propagation."s});
    dispatchTable_s->emplace("resumeQuery",
                             t_CommandEntry{&performResumeQuery,
                                            "Resume a paused query"s,
                                            "Usage: --command resumeQuery --queryId <id>\n"
                                            "Resumes a previously paused query."s});
    dispatchTable_s->emplace("cancelQuery",
                             t_CommandEntry{&performCancelQuery,
                                            "Cancel a query"s,
                                            "Usage: --command cancelQuery --queryId <id>\n"
                                            "Permanently stops the query."s});
    dispatchTable_s->emplace("getQueryResults",
                             t_CommandEntry{&performGetQueryResults,
                                            "Get results of a query"s,
                                            "Usage: --command getQueryResults --queryId <id>\n"
                                            "Returns discovered resources from all responding peers."s});

    // Module commands
    dispatchTable_s->emplace(
        "registerModule",
        t_CommandEntry{&performRegisterModule,
                       "Register a new module"s,
                       "Usage: --command registerModule --moduleLibPath <path> --moduleSymbol <sym>\n"
                       "Registers a shared library module and returns its ID."s});
    dispatchTable_s->emplace("unregisterModule",
                             t_CommandEntry{&performUnregisterModule,
                                            "Unregister a module"s,
                                            "Usage: --command unregisterModule --moduleId <id>\n"
                                            "Removes the module registration."s});
    dispatchTable_s->emplace("loadModule",
                             t_CommandEntry{&performLoadModule,
                                            "Load a registered module"s,
                                            "Usage: --command loadModule --moduleId <id>\n"
                                            "Loads the module into the running server."s});
    dispatchTable_s->emplace("unloadModule",
                             t_CommandEntry{&performUnloadModule,
                                            "Unload an active module"s,
                                            "Usage: --command unloadModule --moduleId <id>\n"
                                            "Unloads the module from the running server."s});
    dispatchTable_s->emplace("listActiveModules",
                             t_CommandEntry{&performListActiveModules,
                                            "List currently loaded modules"s,
                                            "Usage: --command listActiveModules\n"
                                            "Returns all modules that are currently active."s});
    dispatchTable_s->emplace("listAllModules",
                             t_CommandEntry{&performListAllModules,
                                            "List all registered modules"s,
                                            "Usage: --command listAllModules\n"
                                            "Returns all registered modules (active and inactive)."s});
    dispatchTable_s->emplace("getModuleInfo",
                             t_CommandEntry{&performGetModuleInfo,
                                            "Get detailed module information"s,
                                            "Usage: --command getModuleInfo --moduleId <id>\n"
                                            "Returns name, version, library path, and active time."s});

    // Status commands
    dispatchTable_s->emplace("getStatus",
                             t_CommandEntry{&performGetStatus,
                                            "Get server status and version"s,
                                            "Usage: --command getStatus\n"
                                            "Returns the server running status and version string."s});

    // Interactive mode (handled specially in main, not via handler)
    dispatchTable_s->emplace("interactive",
                             t_CommandEntry{nullptr,
                                            "Enter interactive REPL with readline and history"s,
                                            "Usage: --command interactive\n"
                                            "Opens an interactive shell with tab completion, command\n"
                                            "history, and line editing.  History is saved to\n"
                                            "~/.alpine_history between sessions."s});


    return true;
}


bool
commandExists(const string & command)
{
    return dispatchTable_s->contains(command);
}


// Dispatch correct command
//
bool
handleCommand(const string & command)
{
    auto iter = dispatchTable_s->find(command);

    if (iter == dispatchTable_s->end()) {
        Log::Error("Invalid command option: "s + command + " provided!");
        return false;
    }
    auto handler = iter->second.handler;

    if (!handler) {
        Log::Error("Command '"s + command + "' is not dispatchable.");
        return false;
    }

    bool status = (*handler)();

    if (!status) {
        if (jsonOutput_s) {
            cerr << "{\"error\":\"Command '"s + command + "' failed\",\"code\":3}" << endl;
        } else if (!quietOutput_s) {
            cerr << "Error: Command '"s + command + "' failed.  Check application log file." << endl;
        }
        return false;
    }
    return true;
}


void
printVersion()
{
    cout << "AlpineCmdIf version "s << ALPINE_CLI_VERSION << endl;
}


void
printUsage()
{
    cout << "AlpineCmdIf - Alpine P2P Command Line Interface\n"
         << "Version: "s << ALPINE_CLI_VERSION << "\n\n"
         << "Usage:\n"
         << "  AlpineCmdIf --serverAddress <addr> --serverPort <port> --command <cmd> [options]\n\n"
         << "Flags:\n"
         << "  -h, --help              Show this help message\n"
         << "  -v, --version           Show version\n"
         << "  --verbose               Enable verbose output\n"
         << "  --json                  Output results as JSON\n"
         << "  --quiet                 Suppress non-essential output\n"
         << "  --format <fmt>          Output format: json (default), table, csv, yaml\n"
         << "  --color                 Enable colored output (auto-detected for terminals)\n"
         << "  --completions <shell>   Generate shell completions (bash or zsh)\n\n"
         << "Use '--command help' to list all available commands.\n"
         << "Use '--command help --queryString <command>' for detailed help on a command.\n"
         << "Use '--command interactive' for an interactive shell with tab completion and history.\n";
}


void
printCommandList()
{
    // Collect and sort command names for stable output
    vector<string> names;
    names.reserve(dispatchTable_s->size());
    for (const auto & [name, entry] : *dispatchTable_s)
        names.push_back(name);
    std::sort(names.begin(), names.end());

    if (jsonOutput_s) {
        JsonWriter writer;
        writer.beginObject();
        writer.key("commands");
        writer.beginArray();
        for (const auto & name : names) {
            writer.beginObject();
            writer.key("name");
            writer.value(name);
            writer.key("description");
            writer.value(dispatchTable_s->at(name).description);
            writer.endObject();
        }
        writer.endArray();
        writer.endObject();
        cout << writer.result() << endl;
        return;
    }

    cout << "Available commands:\n\n";

    // Find longest command name for alignment
    ulong maxLen = 0;
    for (const auto & name : names) {
        if (name.length() > maxLen)
            maxLen = name.length();
    }

    for (const auto & name : names) {
        cout << "  " << name;
        for (ulong i = name.length(); i < maxLen + 2; ++i)
            cout << ' ';
        cout << dispatchTable_s->at(name).description << "\n";
    }

    cout << "\nUse '--command help --queryString <command>' for detailed help.\n";
}


void
printCommandHelp(const string & command)
{
    auto iter = dispatchTable_s->find(command);
    if (iter == dispatchTable_s->end()) {
        if (jsonOutput_s) {
            cerr << "{\"error\":\"Unknown command: "s + command + "\",\"code\":2}" << endl;
        } else {
            cerr << "Error: Unknown command '"s + command + "'." << endl;
        }
        return;
    }

    if (jsonOutput_s) {
        JsonWriter writer;
        writer.beginObject();
        writer.key("command");
        writer.value(command);
        writer.key("description");
        writer.value(iter->second.description);
        writer.key("help");
        writer.value(iter->second.detailedHelp);
        writer.endObject();
        cout << writer.result() << endl;
        return;
    }

    cout << command << " - " << iter->second.description << "\n\n" << iter->second.detailedHelp << endl;
}


vector<pair<string, string>>
getDispatchEntries()
{
    vector<pair<string, string>> entries;
    if (!dispatchTable_s)
        return entries;

    entries.reserve(dispatchTable_s->size());
    for (const auto & [name, entry] : *dispatchTable_s)
        entries.emplace_back(name, entry.description);

    return entries;
}


// ---------------------------------------------------------------------------
//  Peer commands
// ---------------------------------------------------------------------------

bool
performAddDtcpPeer()
{
    Log::Debug("performAddDtcpPeer invoked.");

    string ipAddress;
    string portStr;

    bool status;
    status = Configuration::getValue("IP Address", ipAddress);

    if (!status) {
        Log::Error("No IP Address value given!");
        return false;
    }
    status = Configuration::getValue("Port", portStr);

    if (!status) {
        Log::Error("No Port value given!");
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
        Log::Error("addPeer RPC call failed!");
        return false;
    }
    if (verbose_s) {
        cout << "Add Request successful.\n";
    }

    return true;
}


bool
performGetDtcpPeerId()
{
    Log::Debug("performGetDtcpPeerId invoked.");

    string ipAddress;
    string portStr;

    bool status;
    status = Configuration::getValue("IP Address", ipAddress);

    if (!status) {
        Log::Error("No IP Address value given!");
        return false;
    }
    status = Configuration::getValue("Port", portStr);

    if (!status) {
        Log::Error("No Port value given!");
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
        Log::Error("getPeerId RPC call failed!");
        return false;
    }

    JsonReader reader(result);
    ulong peerId = 0;
    reader.getUlong("peerId", peerId);

    if (verbose_s) {
        cout << "Request successful.\n";
        cout << "Peer ID: " << peerId << endl;
    } else {
        cout << peerId << endl;
    }

    return true;
}


bool
performGetDtcpPeerStatus()
{
    Log::Debug("performGetDtcpPeerStatus invoked.");

    string peerIdString;
    ulong peerId;

    bool status;
    status = Configuration::getValue("Peer ID", peerIdString);

    if (!status) {
        Log::Error("No Peer ID value given!");
        return false;
    }
    peerId = strtoul(peerIdString.c_str(), 0, 0);

    JsonWriter writer;
    writer.beginObject();
    writer.key("peerId");
    writer.value(peerId);
    writer.endObject();

    string result;
    status = JsonRpcClient::call("getPeer", writer.result(), result);

    if (!status) {
        Log::Error("getPeer RPC call failed!");
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
    } else {
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
performActivateDtcpPeer()
{
    Log::Debug("performActivateDtcpPeer invoked.");

    string peerIdString;
    ulong peerId;

    bool status;
    status = Configuration::getValue("Peer ID", peerIdString);

    if (!status) {
        Log::Error("No Peer ID value given!");
        return false;
    }
    peerId = strtoul(peerIdString.c_str(), 0, 0);

    JsonWriter writer;
    writer.beginObject();
    writer.key("peerId");
    writer.value(peerId);
    writer.endObject();

    string result;
    status = JsonRpcClient::call("activatePeer", writer.result(), result);

    if (!status) {
        Log::Error("activatePeer RPC call failed!");
        return false;
    }
    if (verbose_s) {
        cout << "Activation successful.\n";
    }

    return true;
}


bool
performDeactivateDtcpPeer()
{
    Log::Debug("performDeactivateDtcpPeer invoked.");

    string peerIdString;
    ulong peerId;

    bool status;
    status = Configuration::getValue("Peer ID", peerIdString);

    if (!status) {
        Log::Error("No Peer ID value given!");
        return false;
    }
    peerId = strtoul(peerIdString.c_str(), 0, 0);

    JsonWriter writer;
    writer.beginObject();
    writer.key("peerId");
    writer.value(peerId);
    writer.endObject();

    string result;
    status = JsonRpcClient::call("deactivatePeer", writer.result(), result);

    if (!status) {
        Log::Error("deactivatePeer RPC call failed!");
        return false;
    }
    if (verbose_s) {
        cout << "Deactivation successful.\n";
    }

    return true;
}


bool
performPingDtcpPeer()
{
    Log::Debug("performPingDtcpPeer invoked.");

    string peerIdString;
    ulong peerId;

    bool status;
    status = Configuration::getValue("Peer ID", peerIdString);

    if (!status) {
        Log::Error("No Peer ID value given!");
        return false;
    }
    peerId = strtoul(peerIdString.c_str(), 0, 0);

    JsonWriter writer;
    writer.beginObject();
    writer.key("peerId");
    writer.value(peerId);
    writer.endObject();

    string result;
    status = JsonRpcClient::call("pingPeer", writer.result(), result);

    if (!status) {
        Log::Error("pingPeer RPC call failed!");
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
performExcludeHost()
{
    Log::Debug("performExcludeHost invoked.");

    bool status;
    string ipAddress;

    status = Configuration::getValue("IP Address", ipAddress);

    if (!status) {
        Log::Error("No IP Address value given!");
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
        Log::Error("excludeHost RPC call failed!");
        return false;
    }
    if (verbose_s) {
        cout << "Exclude host request successful.\n";
    }

    return true;
}


bool
performExcludeSubnet()
{
    Log::Debug("performExcludeSubnet invoked.");

    bool status;
    string subnetIpAddress;
    string subnetMask;

    status = Configuration::getValue("Subnet IP Address", subnetIpAddress);

    if (!status) {
        Log::Error("No Subnet IP Address value given!");
        return false;
    }
    status = Configuration::getValue("Subnet Mask", subnetMask);

    if (!status) {
        Log::Error("No Subnet Mask value given!");
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
        Log::Error("excludeSubnet RPC call failed!");
        return false;
    }
    if (verbose_s) {
        cout << "Exclude subnet request successful.\n";
    }

    return true;
}


bool
performAllowHost()
{
    Log::Debug("performAllowHost invoked.");

    bool status;
    string ipAddress;

    status = Configuration::getValue("IP Address", ipAddress);

    if (!status) {
        Log::Error("No IP Address value given!");
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
        Log::Error("allowHost RPC call failed!");
        return false;
    }
    if (verbose_s) {
        cout << "Allow host request successful.\n";
    }

    return true;
}


bool
performAllowSubnet()
{
    Log::Debug("performAllowSubnet invoked.");

    bool status;
    string subnetIpAddress;

    status = Configuration::getValue("Subnet IP Address", subnetIpAddress);

    if (!status) {
        Log::Error("No Subnet IP Address value given!");
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
        Log::Error("allowSubnet RPC call failed!");
        return false;
    }
    if (verbose_s) {
        cout << "Allow subnet request successful.\n";
    }

    return true;
}


bool
performListExcludedHosts()
{
    Log::Debug("performListExcludedHosts invoked.");

    string result;
    bool status = JsonRpcClient::call("listExcludedHosts", "{}", result);

    if (!status) {
        Log::Error("listExcludedHosts RPC call failed!");
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
    if (arrStart != string::npos && arrEnd != string::npos && arrEnd > arrStart) {
        string arrContent = result.substr(arrStart + 1, arrEnd - arrStart - 1);
        ulong pos = 0;
        while (pos < arrContent.length()) {
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
performListExcludedSubnets()
{
    Log::Debug("performListExcludedSubnets invoked.");

    string result;
    bool status = JsonRpcClient::call("listExcludedSubnets", "{}", result);

    if (!status) {
        Log::Error("listExcludedSubnets RPC call failed!");
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
    while (searchPos < result.length()) {
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
        if (objReader.getString("ipAddress", ipAddr) && objReader.getString("netMask", netMask)) {
            if (verbose_s) {
                cout << "   Subnet Address: " << ipAddr << " \tSubnet Mask: " << netMask << endl;
            } else {
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
performGetUserGroupList()
{
    Log::Debug("performGetUserGroupList invoked.");

    string result;
    bool status = JsonRpcClient::call("listGroups", "{}", result);

    if (!status) {
        Log::Error("listGroups RPC call failed!");
        return false;
    }

    if (verbose_s) {
        cout << "Request successful.\n";
        cout << "Result: " << result << endl;
    } else {
        cout << result << endl;
    }

    return true;
}


bool
performCreateUserGroup()
{
    Log::Debug("performCreateUserGroup invoked.");

    string groupName;
    string description;

    bool status = Configuration::getValue("Group Name", groupName);
    if (!status) {
        Log::Error("No Group Name value given!");
        return false;
    }

    Configuration::getValue("Description", description);

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
        Log::Error("createGroup RPC call failed!");
        return false;
    }

    JsonReader reader(result);
    ulong groupId = 0;
    reader.getUlong("groupId", groupId);

    if (verbose_s) {
        cout << "Request successful.\n";
        cout << "Group ID: " << groupId << endl;
    } else {
        cout << groupId << endl;
    }

    return true;
}


bool
performDestroyUserGroup()
{
    Log::Debug("performDestroyUserGroup invoked.");

    string groupIdStr;
    bool status = Configuration::getValue("Group ID", groupIdStr);
    if (!status) {
        Log::Error("No Group ID value given!");
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
        Log::Error("deleteGroup RPC call failed!");
        return false;
    }
    if (verbose_s) {
        cout << "Destroy group successful.\n";
    }

    return true;
}


bool
performGetPeerUserGroupList()
{
    Log::Debug("performGetPeerUserGroupList invoked.");

    string groupIdStr;
    bool status = Configuration::getValue("Group ID", groupIdStr);
    if (!status) {
        Log::Error("No Group ID value given!");
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
        Log::Error("getGroupPeerList RPC call failed!");
        return false;
    }

    if (verbose_s) {
        cout << "Request successful.\n";
        cout << "Result: " << result << endl;
    } else {
        cout << result << endl;
    }

    return true;
}


bool
performAddPeerToGroup()
{
    Log::Debug("performAddPeerToGroup invoked.");

    string groupIdStr;
    string peerIdStr;

    bool status = Configuration::getValue("Group ID", groupIdStr);
    if (!status) {
        Log::Error("No Group ID value given!");
        return false;
    }

    status = Configuration::getValue("Peer ID", peerIdStr);
    if (!status) {
        Log::Error("No Peer ID value given!");
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
        Log::Error("addPeerToGroup RPC call failed!");
        return false;
    }
    if (verbose_s) {
        cout << "Add peer to group successful.\n";
    }

    return true;
}


bool
performRemovePeerFromGroup()
{
    Log::Debug("performRemovePeerFromGroup invoked.");

    string groupIdStr;
    string peerIdStr;

    bool status = Configuration::getValue("Group ID", groupIdStr);
    if (!status) {
        Log::Error("No Group ID value given!");
        return false;
    }

    status = Configuration::getValue("Peer ID", peerIdStr);
    if (!status) {
        Log::Error("No Peer ID value given!");
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
        Log::Error("removePeerFromGroup RPC call failed!");
        return false;
    }
    if (verbose_s) {
        cout << "Remove peer from group successful.\n";
    }

    return true;
}


bool
performGetExtendedPeerList()
{
    Log::Debug("performGetExtendedPeerList invoked.");

    string result;
    bool status = JsonRpcClient::call("getAllPeers", "{}", result);

    if (!status) {
        Log::Error("getAllPeers RPC call failed!");
        return false;
    }

    if (verbose_s) {
        cout << "Request successful.\n";
        cout << "Result: " << result << endl;
    } else {
        cout << result << endl;
    }

    return true;
}


// ---------------------------------------------------------------------------
//  Query commands
// ---------------------------------------------------------------------------

bool
performStartQuery()
{
    Log::Debug("performStartQuery invoked.");

    string queryString;
    bool status = Configuration::getValue("Query String", queryString);
    if (!status) {
        Log::Error("No Query String value given!");
        return false;
    }

    JsonWriter writer;
    writer.beginObject();
    writer.key("queryString");
    writer.value(queryString);

    string groupName;
    if (Configuration::getValue("Group Name", groupName)) {
        writer.key("groupName");
        writer.value(groupName);
    }

    string autoHaltStr;
    if (Configuration::getValue("Auto Halt Limit", autoHaltStr)) {
        writer.key("autoHaltLimit");
        writer.value(strtoul(autoHaltStr.c_str(), 0, 0));
    }

    string peerDescStr;
    if (Configuration::getValue("Peer Description Limit", peerDescStr)) {
        writer.key("peerDescMax");
        writer.value(strtoul(peerDescStr.c_str(), 0, 0));
    }

    writer.endObject();

    string result;
    status = JsonRpcClient::call("startQuery", writer.result(), result);

    if (!status) {
        Log::Error("startQuery RPC call failed!");
        return false;
    }

    JsonReader reader(result);
    ulong queryId = 0;
    reader.getUlong("queryId", queryId);

    if (verbose_s) {
        cout << "Query start successful.\n";
        cout << "Query ID: " << queryId << endl;
    } else {
        cout << queryId << endl;
    }

    return true;
}


bool
performGetQueryStatus()
{
    Log::Debug("performGetQueryStatus invoked.");

    string queryIdStr;
    bool status = Configuration::getValue("Query ID", queryIdStr);
    if (!status) {
        Log::Error("No Query ID value given!");
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
        Log::Error("getQueryStatus RPC call failed!");
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
    } else {
        cout << totalPeers << " ";
        cout << peersQueried << " ";
        cout << numResponses << " ";
        cout << totalHits << endl;
    }

    return true;
}


bool
performPauseQuery()
{
    Log::Debug("performPauseQuery invoked.");

    string queryIdStr;
    bool status = Configuration::getValue("Query ID", queryIdStr);
    if (!status) {
        Log::Error("No Query ID value given!");
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
        Log::Error("pauseQuery RPC call failed!");
        return false;
    }
    if (verbose_s) {
        cout << "Query pause successful.\n";
    }

    return true;
}


bool
performResumeQuery()
{
    Log::Debug("performResumeQuery invoked.");

    string queryIdStr;
    bool status = Configuration::getValue("Query ID", queryIdStr);
    if (!status) {
        Log::Error("No Query ID value given!");
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
        Log::Error("resumeQuery RPC call failed!");
        return false;
    }
    if (verbose_s) {
        cout << "Query resume successful.\n";
    }

    return true;
}


bool
performCancelQuery()
{
    Log::Debug("performCancelQuery invoked.");

    string queryIdStr;
    bool status = Configuration::getValue("Query ID", queryIdStr);
    if (!status) {
        Log::Error("No Query ID value given!");
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
        Log::Error("cancelQuery RPC call failed!");
        return false;
    }
    if (verbose_s) {
        cout << "Query cancel successful.\n";
    }

    return true;
}


bool
performGetQueryResults()
{
    Log::Debug("performGetQueryResults invoked.");

    string queryIdStr;
    bool status = Configuration::getValue("Query ID", queryIdStr);
    if (!status) {
        Log::Error("No Query ID value given!");
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
        Log::Error("getQueryResults RPC call failed!");
        return false;
    }

    if (verbose_s) {
        cout << "Request successful.\n";
        cout << "Result: " << result << endl;
    } else {
        cout << result << endl;
    }

    return true;
}


// ---------------------------------------------------------------------------
//  Module commands
// ---------------------------------------------------------------------------

bool
performRegisterModule()
{
    Log::Debug("performRegisterModule invoked.");

    string libPath;
    string symbol;

    bool status = Configuration::getValue("Module Lib Path", libPath);
    if (!status) {
        Log::Error("No Module Lib Path value given!");
        return false;
    }

    status = Configuration::getValue("Module Symbol", symbol);
    if (!status) {
        Log::Error("No Module Symbol value given!");
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
        Log::Error("registerModule RPC call failed!");
        return false;
    }

    JsonReader reader(result);
    ulong moduleId = 0;
    reader.getUlong("moduleId", moduleId);

    if (verbose_s) {
        cout << "Request successful.\n";
        cout << "Module ID: " << moduleId << endl;
    } else {
        cout << moduleId << endl;
    }

    return true;
}


bool
performUnregisterModule()
{
    Log::Debug("performUnregisterModule invoked.");

    string moduleIdStr;
    bool status = Configuration::getValue("Module ID", moduleIdStr);
    if (!status) {
        Log::Error("No Module ID value given!");
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
        Log::Error("unregisterModule RPC call failed!");
        return false;
    }
    if (verbose_s) {
        cout << "Request successful.\n";
    }

    return true;
}


bool
performLoadModule()
{
    Log::Debug("performLoadModule invoked.");

    string moduleIdStr;
    bool status = Configuration::getValue("Module ID", moduleIdStr);
    if (!status) {
        Log::Error("No Module ID value given!");
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
        Log::Error("loadModule RPC call failed!");
        return false;
    }
    if (verbose_s) {
        cout << "Request successful.\n";
    }

    return true;
}


bool
performUnloadModule()
{
    Log::Debug("performUnloadModule invoked.");

    string moduleIdStr;
    bool status = Configuration::getValue("Module ID", moduleIdStr);
    if (!status) {
        Log::Error("No Module ID value given!");
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
        Log::Error("unloadModule RPC call failed!");
        return false;
    }
    if (verbose_s) {
        cout << "Request successful.\n";
    }

    return true;
}


bool
performListActiveModules()
{
    Log::Debug("performListActiveModules invoked.");

    string result;
    bool status = JsonRpcClient::call("listActiveModules", "{}", result);

    if (!status) {
        Log::Error("listActiveModules RPC call failed!");
        return false;
    }

    if (verbose_s) {
        cout << "Request successful.\n";
        cout << "Result: " << result << endl;
    } else {
        cout << result << endl;
    }

    return true;
}


bool
performListAllModules()
{
    Log::Debug("performListAllModules invoked.");

    string result;
    bool status = JsonRpcClient::call("listAllModules", "{}", result);

    if (!status) {
        Log::Error("listAllModules RPC call failed!");
        return false;
    }

    if (verbose_s) {
        cout << "Request successful.\n";
        cout << "Result: " << result << endl;
    } else {
        cout << result << endl;
    }

    return true;
}


bool
performGetModuleInfo()
{
    Log::Debug("performGetModuleInfo invoked.");

    string moduleIdStr;
    bool status = Configuration::getValue("Module ID", moduleIdStr);
    if (!status) {
        Log::Error("No Module ID value given!");
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
        Log::Error("getModuleInfo RPC call failed!");
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
    } else {
        cout << modId << " " << modName << " " << modVer << endl;
    }

    return true;
}


// ---------------------------------------------------------------------------
//  Status commands
// ---------------------------------------------------------------------------

bool
performGetStatus()
{
    Log::Debug("performGetStatus invoked.");

    string result;
    bool status = JsonRpcClient::call("getStatus", "{}", result);

    if (!status) {
        Log::Error("getStatus RPC call failed!");
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
    } else {
        cout << serverStatus << " " << version << endl;
    }

    return true;
}


// ---------------------------------------------------------------------------
//  Interactive REPL mode
// ---------------------------------------------------------------------------


/// Resolve the history file path (~/.alpine_history).
///
static string
historyFilePath()
{
    string path;

    const char * home = getenv("HOME");
    if (home) {
        path = string(home) + "/.alpine_history"s;
    } else {
        path = ".alpine_history"s;
    }
    return path;
}


/// Linenoise tab-completion callback.
/// Matches partially-typed input against the dispatch table keys.
///
void
alpineCompletionCallback(const char * buf, linenoiseCompletions * lc)
{
    if (!dispatchTable_s || !buf)
        return;

    string prefix(buf);

    for (const auto & [name, entry] : *dispatchTable_s) {
        if (name.starts_with(prefix)) {
            linenoiseAddCompletion(lc, name.c_str());
        }
    }

    // REPL built-in commands
    static const char * builtins[] = {"help", "exit", "quit", nullptr};
    for (const char ** cmd = builtins; *cmd; ++cmd) {
        if (string(*cmd).starts_with(prefix)) {
            linenoiseAddCompletion(lc, *cmd);
        }
    }
}


/// Enter the interactive REPL loop.
///
/// Returns the process exit code (0 on clean exit).
///
int
runInteractive(const string & serverAddress, ushort serverPort)
{
    string histFile = historyFilePath();

    // Configure linenoise
    linenoiseSetMultiLine(0);
    linenoiseHistorySetMaxLen(500);
    linenoiseHistoryLoad(histFile.c_str());
    linenoiseSetCompletionCallback(alpineCompletionCallback);

    if (!quietOutput_s) {
        cout << "Alpine interactive shell (server "s << serverAddress << ":"s << serverPort << ")\n"s
             << "Type a command name, 'help' for a list, or Ctrl-D to exit.\n"s;
    }

    while (true) {
        char * line = linenoise("alpine> ");

        if (!line) {
            // EOF / Ctrl-D
            break;
        }

        string input(line);
        linenoiseFree(line);

        // Trim leading/trailing whitespace
        while (!input.empty() && input.front() == ' ')
            input.erase(input.begin());
        while (!input.empty() && input.back() == ' ')
            input.pop_back();

        if (input.empty())
            continue;

        // Add to history and persist
        linenoiseHistoryAdd(input.c_str());
        linenoiseHistorySave(histFile.c_str());

        // Built-in REPL commands
        if (input == "exit"s || input == "quit"s) {
            break;
        }

        if (input == "help"s) {
            printCommandList();
            continue;
        }

        if (input.starts_with("help "s)) {
            string helpCmd = input.substr(5);
            while (!helpCmd.empty() && helpCmd.front() == ' ')
                helpCmd.erase(helpCmd.begin());
            if (!helpCmd.empty()) {
                printCommandHelp(helpCmd);
            } else {
                printCommandList();
            }
            continue;
        }

        // Dispatch the command
        if (!commandExists(input)) {
            cerr << "Unknown command: "s << input << ".  Type 'help' for a list." << endl;
            continue;
        }

        handleCommand(input);
    }

    if (!quietOutput_s) {
        cout << "\nExiting interactive shell." << endl;
    }

    return 0;
}
