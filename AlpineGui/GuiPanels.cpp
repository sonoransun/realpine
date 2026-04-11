/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AsyncRpcClient.h>
#include <GuiHelpers.h>
#include <GuiPanels.h>
#include <JsonReader.h>
#include <JsonWriter.h>
#include <cstdlib>
#include <cstring>
#include <future>
#include <imgui.h>


// Helper: launch an async RPC call if none is in progress
static void
launchCall(
    AppState & state, AsyncRpcClient & client, const string & method, const string & params, const string & target)
{
    if (state.callInProgress)
        return;

    if (!client.isConnected()) {
        addLog(state, LogLevel::Error, "Not connected");
        return;
    }

    state.callInProgress = true;
    state.callTarget = target;
    state.pendingResult.clear();

    state.pendingCall = std::async(std::launch::async, [&client, method, params, &state]() -> bool {
        return client.call(method, params, state.pendingResult);
    });

    addLog(state, LogLevel::Info, "Calling "s + method + "...");
}


// ============================================================================
//  Connection Bar
// ============================================================================

void
drawConnectionBar(AppState & state, AsyncRpcClient & client)
{
    ImGui::BeginChild("ConnectionBar", ImVec2(0, 44), true);

    ImGui::SetNextItemWidth(180);
    ImGui::InputText("Host", state.host, sizeof(state.host));

    ImGui::SameLine();
    ImGui::SetNextItemWidth(80);
    ImGui::InputInt("Port", &state.port, 0, 0);

    ImGui::SameLine();
    bool disabled = state.callInProgress;

    if (state.connected) {
        if (disabled)
            ImGui::BeginDisabled();
        if (ImGui::Button("Disconnect")) {
            client.disconnect();
            state.connected = false;
            state.statusMessage = "Disconnected";
            addLog(state, LogLevel::Info, "Disconnected");
        }
        if (disabled)
            ImGui::EndDisabled();
    } else {
        if (disabled)
            ImGui::BeginDisabled();
        if (ImGui::Button("Connect")) {
            ushort p = static_cast<ushort>(state.port);
            if (client.connect(state.host, p)) {
                state.connected = true;
                state.statusMessage = "Connected to "s + state.host + ":" + std::to_string(state.port);
                addLog(state, LogLevel::Info, state.statusMessage);
            } else {
                state.statusMessage = "Connection failed";
                addLog(
                    state, LogLevel::Error, "Failed to connect to "s + state.host + ":" + std::to_string(state.port));
            }
        }
        if (disabled)
            ImGui::EndDisabled();
    }

    ImGui::SameLine();
    ImGui::Text("|");
    ImGui::SameLine();

    // Status indicator
    if (state.connected) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.9f, 0.2f, 1.0f));
        ImGui::Text("  Connected");
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.2f, 0.2f, 1.0f));
        ImGui::Text("  Disconnected");
    }
    ImGui::PopStyleColor();

    if (state.callInProgress) {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.3f, 1.0f), "  [calling...]");
    }

    ImGui::EndChild();
}


// ============================================================================
//  Status Tab
// ============================================================================

static void
drawStatusTab(AppState & state, AsyncRpcClient & client)
{
    if (ImGui::Button("Refresh Status"))
        launchCall(state, client, "getStatus", "{}", "status");

    ImGui::Separator();

    if (!state.statusResult.empty()) {
        JsonReader reader(state.statusResult);
        string serverStatus, version;
        reader.getString("status", serverStatus);
        reader.getString("version", version);

        ImGui::Text("Server Status: %s", serverStatus.c_str());
        ImGui::Text("Version:       %s", version.c_str());

        ImGui::Separator();
        ImGui::TextWrapped("Raw: %s", state.statusResult.c_str());
    }
}


// ============================================================================
//  Peers Tab
// ============================================================================

static void
drawPeersTab(AppState & state, AsyncRpcClient & client)
{
    // --- List all peers ---
    if (ImGui::Button("List All Peers"))
        launchCall(state, client, "getAllPeers", "{}", "peers");

    ImGui::SameLine();
    if (ImGui::Button("Get Peer Info") && strlen(state.peerId) > 0) {
        ulong id = strtoul(state.peerId, nullptr, 10);
        JsonWriter w;
        w.beginObject();
        w.key("peerId");
        w.value(id);
        w.endObject();
        launchCall(state, client, "getPeer", w.result(), "peerInfo");
    }

    ImGui::Separator();

    // --- Add Peer form ---
    ImGui::Text("Add Peer:");
    ImGui::SetNextItemWidth(160);
    ImGui::InputText("IP##peer", state.peerIp, sizeof(state.peerIp));
    ImGui::SameLine();
    ImGui::SetNextItemWidth(80);
    ImGui::InputInt("Port##peer", &state.peerPort, 0, 0);
    ImGui::SameLine();
    if (ImGui::Button("Add Peer")) {
        JsonWriter w;
        w.beginObject();
        w.key("ipAddress");
        w.value(string(state.peerIp));
        w.key("port");
        w.value(static_cast<ulong>(state.peerPort));
        w.endObject();
        launchCall(state, client, "addPeer", w.result(), "peers");
    }

    ImGui::Separator();

    // --- Peer actions ---
    ImGui::Text("Peer Actions:");
    ImGui::SetNextItemWidth(120);
    ImGui::InputText("Peer ID##act", state.peerId, sizeof(state.peerId));
    ImGui::SameLine();

    if (ImGui::Button("Get Peer ID")) {
        JsonWriter w;
        w.beginObject();
        w.key("ipAddress");
        w.value(string(state.peerIp));
        w.key("port");
        w.value(static_cast<ulong>(state.peerPort));
        w.endObject();
        launchCall(state, client, "getPeerId", w.result(), "peerInfo");
    }

    if (strlen(state.peerId) > 0) {
        ulong id = strtoul(state.peerId, nullptr, 10);

        ImGui::SameLine();
        if (ImGui::Button("Activate")) {
            JsonWriter w;
            w.beginObject();
            w.key("peerId");
            w.value(id);
            w.endObject();
            launchCall(state, client, "activatePeer", w.result(), "peerInfo");
        }

        ImGui::SameLine();
        if (ImGui::Button("Deactivate")) {
            JsonWriter w;
            w.beginObject();
            w.key("peerId");
            w.value(id);
            w.endObject();
            launchCall(state, client, "deactivatePeer", w.result(), "peerInfo");
        }

        ImGui::SameLine();
        if (ImGui::Button("Ping")) {
            JsonWriter w;
            w.beginObject();
            w.key("peerId");
            w.value(id);
            w.endObject();
            launchCall(state, client, "pingPeer", w.result(), "peerInfo");
        }
    }

    ImGui::Separator();

    // --- Peer list display ---
    if (!state.peersResult.empty()) {
        ImGui::Text("Peer List:");
        std::vector<string> items;
        parseJsonArray(state.peersResult, items);

        if (ImGui::BeginTable("PeersTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("Peer ID");
            ImGui::TableSetupColumn("IP Address");
            ImGui::TableSetupColumn("Port");
            ImGui::TableSetupColumn("Avg BW (Kbps)");
            ImGui::TableHeadersRow();

            for (const auto & item : items) {
                JsonReader r(item);
                ulong peerId = 0, port = 0, avgBw = 0;
                string ipAddr;
                r.getUlong("peerId", peerId);
                r.getString("ipAddress", ipAddr);
                r.getUlong("port", port);
                r.getUlong("avgBandwidth", avgBw);

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("%lu", peerId);
                ImGui::TableNextColumn();
                ImGui::Text("%s", ipAddr.c_str());
                ImGui::TableNextColumn();
                ImGui::Text("%lu", port);
                ImGui::TableNextColumn();
                ImGui::Text("%lu", avgBw);
            }

            ImGui::EndTable();
        }
    }

    // --- Peer info display ---
    if (!state.peerInfoResult.empty()) {
        ImGui::Separator();
        ImGui::Text("Peer Info:");
        ImGui::TextWrapped("%s", state.peerInfoResult.c_str());
    }
}


// ============================================================================
//  Queries Tab
// ============================================================================

static void
drawQueriesTab(AppState & state, AsyncRpcClient & client)
{
    // --- Start Query form ---
    ImGui::Text("Start Query:");
    ImGui::SetNextItemWidth(300);
    ImGui::InputText("Query String", state.queryString, sizeof(state.queryString));
    ImGui::SetNextItemWidth(180);
    ImGui::InputText("Group (optional)", state.queryGroup, sizeof(state.queryGroup));
    ImGui::SetNextItemWidth(100);
    ImGui::InputInt("Auto Halt Limit", &state.queryAutoHalt, 0, 0);
    ImGui::SetNextItemWidth(100);
    ImGui::InputInt("Peer Desc Max", &state.queryPeerDescMax, 0, 0);

    if (ImGui::Button("Start Query")) {
        JsonWriter w;
        w.beginObject();
        w.key("queryString");
        w.value(string(state.queryString));
        if (strlen(state.queryGroup) > 0) {
            w.key("groupName");
            w.value(string(state.queryGroup));
        }
        if (state.queryAutoHalt > 0) {
            w.key("autoHaltLimit");
            w.value(static_cast<ulong>(state.queryAutoHalt));
        }
        if (state.queryPeerDescMax > 0) {
            w.key("peerDescMax");
            w.value(static_cast<ulong>(state.queryPeerDescMax));
        }
        w.endObject();
        launchCall(state, client, "startQuery", w.result(), "query");
    }

    ImGui::Separator();

    // --- Query actions ---
    ImGui::Text("Query Actions:");
    ImGui::SetNextItemWidth(120);
    ImGui::InputText("Query ID", state.queryId, sizeof(state.queryId));

    if (strlen(state.queryId) > 0) {
        ulong qid = strtoul(state.queryId, nullptr, 10);

        ImGui::SameLine();
        if (ImGui::Button("Get Status##q")) {
            JsonWriter w;
            w.beginObject();
            w.key("queryId");
            w.value(qid);
            w.endObject();
            launchCall(state, client, "getQueryStatus", w.result(), "queryStatus");
        }

        ImGui::SameLine();
        if (ImGui::Button("Get Results")) {
            JsonWriter w;
            w.beginObject();
            w.key("queryId");
            w.value(qid);
            w.endObject();
            launchCall(state, client, "getQueryResults", w.result(), "query");
        }

        ImGui::SameLine();
        if (ImGui::Button("Pause")) {
            JsonWriter w;
            w.beginObject();
            w.key("queryId");
            w.value(qid);
            w.endObject();
            launchCall(state, client, "pauseQuery", w.result(), "queryStatus");
        }

        ImGui::SameLine();
        if (ImGui::Button("Resume")) {
            JsonWriter w;
            w.beginObject();
            w.key("queryId");
            w.value(qid);
            w.endObject();
            launchCall(state, client, "resumeQuery", w.result(), "queryStatus");
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            JsonWriter w;
            w.beginObject();
            w.key("queryId");
            w.value(qid);
            w.endObject();
            launchCall(state, client, "cancelQuery", w.result(), "queryStatus");
        }
    }

    ImGui::Separator();

    // --- Query status display ---
    if (!state.queryStatusResult.empty()) {
        ImGui::Text("Query Status:");
        JsonReader r(state.queryStatusResult);
        ulong totalPeers = 0, queried = 0, responses = 0, hits = 0;
        r.getUlong("totalPeers", totalPeers);
        r.getUlong("peersQueried", queried);
        r.getUlong("numPeerResponses", responses);
        r.getUlong("totalHits", hits);

        ImGui::Text("Total Peers: %lu", totalPeers);
        ImGui::Text("Peers Queried: %lu", queried);
        ImGui::Text("Peer Responses: %lu", responses);
        ImGui::Text("Total Hits: %lu", hits);
    }

    // --- Query result display ---
    if (!state.queryResult.empty()) {
        ImGui::Separator();
        ImGui::Text("Query Result:");
        ImGui::TextWrapped("%s", state.queryResult.c_str());
    }
}


// ============================================================================
//  Groups Tab
// ============================================================================

static void
drawGroupsTab(AppState & state, AsyncRpcClient & client)
{
    // --- List groups ---
    if (ImGui::Button("List Groups"))
        launchCall(state, client, "listGroups", "{}", "groups");

    ImGui::Separator();

    // --- Create group form ---
    ImGui::Text("Create Group:");
    ImGui::SetNextItemWidth(180);
    ImGui::InputText("Name##grp", state.groupName, sizeof(state.groupName));
    ImGui::SameLine();
    ImGui::SetNextItemWidth(250);
    ImGui::InputText("Description##grp", state.groupDesc, sizeof(state.groupDesc));
    ImGui::SameLine();
    if (ImGui::Button("Create")) {
        JsonWriter w;
        w.beginObject();
        w.key("name");
        w.value(string(state.groupName));
        w.key("description");
        w.value(string(state.groupDesc));
        w.endObject();
        launchCall(state, client, "createGroup", w.result(), "groups");
    }

    ImGui::Separator();

    // --- Group actions ---
    ImGui::Text("Group Actions:");
    ImGui::SetNextItemWidth(120);
    ImGui::InputText("Group ID", state.groupId, sizeof(state.groupId));

    if (strlen(state.groupId) > 0) {
        ulong gid = strtoul(state.groupId, nullptr, 10);

        ImGui::SameLine();
        if (ImGui::Button("Delete Group")) {
            JsonWriter w;
            w.beginObject();
            w.key("groupId");
            w.value(gid);
            w.endObject();
            launchCall(state, client, "deleteGroup", w.result(), "groups");
        }

        ImGui::SameLine();
        if (ImGui::Button("Get Peer List##grp")) {
            JsonWriter w;
            w.beginObject();
            w.key("groupId");
            w.value(gid);
            w.endObject();
            launchCall(state, client, "getGroupPeerList", w.result(), "groupInfo");
        }
    }

    ImGui::Separator();

    // --- Add/Remove peer from group ---
    ImGui::Text("Group Membership:");
    ImGui::SetNextItemWidth(120);
    ImGui::InputText("Peer ID##grp", state.groupPeerId, sizeof(state.groupPeerId));

    if (strlen(state.groupId) > 0 && strlen(state.groupPeerId) > 0) {
        ulong gid = strtoul(state.groupId, nullptr, 10);
        ulong pid = strtoul(state.groupPeerId, nullptr, 10);

        ImGui::SameLine();
        if (ImGui::Button("Add to Group")) {
            JsonWriter w;
            w.beginObject();
            w.key("groupId");
            w.value(gid);
            w.key("peerId");
            w.value(pid);
            w.endObject();
            launchCall(state, client, "addPeerToGroup", w.result(), "groupInfo");
        }

        ImGui::SameLine();
        if (ImGui::Button("Remove from Group")) {
            JsonWriter w;
            w.beginObject();
            w.key("groupId");
            w.value(gid);
            w.key("peerId");
            w.value(pid);
            w.endObject();
            launchCall(state, client, "removePeerFromGroup", w.result(), "groupInfo");
        }
    }

    ImGui::Separator();

    // --- Results display ---
    if (!state.groupsResult.empty()) {
        ImGui::Text("Groups:");
        ImGui::TextWrapped("%s", state.groupsResult.c_str());
    }

    if (!state.groupInfoResult.empty()) {
        ImGui::Separator();
        ImGui::Text("Group Info:");
        ImGui::TextWrapped("%s", state.groupInfoResult.c_str());
    }
}


// ============================================================================
//  Modules Tab
// ============================================================================

static void
drawModulesTab(AppState & state, AsyncRpcClient & client)
{
    // --- List modules ---
    if (ImGui::Button("List Active Modules"))
        launchCall(state, client, "listActiveModules", "{}", "modules");

    ImGui::SameLine();
    if (ImGui::Button("List All Modules"))
        launchCall(state, client, "listAllModules", "{}", "modules");

    ImGui::Separator();

    // --- Register module form ---
    ImGui::Text("Register Module:");
    ImGui::SetNextItemWidth(300);
    ImGui::InputText("Library Path", state.moduleLibPath, sizeof(state.moduleLibPath));
    ImGui::SetNextItemWidth(200);
    ImGui::InputText("Bootstrap Symbol", state.moduleSymbol, sizeof(state.moduleSymbol));

    if (ImGui::Button("Register")) {
        JsonWriter w;
        w.beginObject();
        w.key("libraryPath");
        w.value(string(state.moduleLibPath));
        w.key("bootstrapSymbol");
        w.value(string(state.moduleSymbol));
        w.endObject();
        launchCall(state, client, "registerModule", w.result(), "moduleInfo");
    }

    ImGui::Separator();

    // --- Module actions ---
    ImGui::Text("Module Actions:");
    ImGui::SetNextItemWidth(120);
    ImGui::InputText("Module ID", state.moduleId, sizeof(state.moduleId));

    if (strlen(state.moduleId) > 0) {
        ulong mid = strtoul(state.moduleId, nullptr, 10);

        ImGui::SameLine();
        if (ImGui::Button("Load")) {
            JsonWriter w;
            w.beginObject();
            w.key("moduleId");
            w.value(mid);
            w.endObject();
            launchCall(state, client, "loadModule", w.result(), "moduleInfo");
        }

        ImGui::SameLine();
        if (ImGui::Button("Unload")) {
            JsonWriter w;
            w.beginObject();
            w.key("moduleId");
            w.value(mid);
            w.endObject();
            launchCall(state, client, "unloadModule", w.result(), "moduleInfo");
        }

        ImGui::SameLine();
        if (ImGui::Button("Unregister")) {
            JsonWriter w;
            w.beginObject();
            w.key("moduleId");
            w.value(mid);
            w.endObject();
            launchCall(state, client, "unregisterModule", w.result(), "moduleInfo");
        }

        ImGui::SameLine();
        if (ImGui::Button("Get Info##mod")) {
            JsonWriter w;
            w.beginObject();
            w.key("moduleId");
            w.value(mid);
            w.endObject();
            launchCall(state, client, "getModuleInfo", w.result(), "moduleInfo");
        }
    }

    ImGui::Separator();

    // --- Module list display ---
    if (!state.modulesResult.empty()) {
        ImGui::Text("Modules:");
        std::vector<string> items;
        parseJsonArray(state.modulesResult, items);

        if (ImGui::BeginTable("ModulesTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("Module ID");
            ImGui::TableSetupColumn("Name");
            ImGui::TableSetupColumn("Version");
            ImGui::TableSetupColumn("Library");
            ImGui::TableHeadersRow();

            for (const auto & item : items) {
                JsonReader r(item);
                ulong modId = 0;
                string modName, modVer, modLib;
                r.getUlong("moduleId", modId);
                r.getString("moduleName", modName);
                r.getString("version", modVer);
                r.getString("libraryPath", modLib);

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("%lu", modId);
                ImGui::TableNextColumn();
                ImGui::Text("%s", modName.c_str());
                ImGui::TableNextColumn();
                ImGui::Text("%s", modVer.c_str());
                ImGui::TableNextColumn();
                ImGui::Text("%s", modLib.c_str());
            }

            ImGui::EndTable();
        }
    }

    // --- Module info display ---
    if (!state.moduleInfoResult.empty()) {
        ImGui::Separator();
        ImGui::Text("Module Info:");
        ImGui::TextWrapped("%s", state.moduleInfoResult.c_str());
    }
}


// ============================================================================
//  Network Tab
// ============================================================================

static void
drawNetworkTab(AppState & state, AsyncRpcClient & client)
{
    // --- List filters ---
    if (ImGui::Button("List Excluded Hosts"))
        launchCall(state, client, "listExcludedHosts", "{}", "excludedHosts");

    ImGui::SameLine();
    if (ImGui::Button("List Excluded Subnets"))
        launchCall(state, client, "listExcludedSubnets", "{}", "excludedSubnets");

    ImGui::Separator();

    // --- Host filter ---
    ImGui::Text("Host Filter:");
    ImGui::SetNextItemWidth(180);
    ImGui::InputText("IP Address##net", state.netIpAddress, sizeof(state.netIpAddress));

    if (strlen(state.netIpAddress) > 0) {
        ImGui::SameLine();
        if (ImGui::Button("Exclude Host")) {
            JsonWriter w;
            w.beginObject();
            w.key("ipAddress");
            w.value(string(state.netIpAddress));
            w.endObject();
            launchCall(state, client, "excludeHost", w.result(), "excludedHosts");
        }

        ImGui::SameLine();
        if (ImGui::Button("Allow Host")) {
            JsonWriter w;
            w.beginObject();
            w.key("ipAddress");
            w.value(string(state.netIpAddress));
            w.endObject();
            launchCall(state, client, "allowHost", w.result(), "excludedHosts");
        }
    }

    ImGui::Separator();

    // --- Subnet filter ---
    ImGui::Text("Subnet Filter:");
    ImGui::SetNextItemWidth(180);
    ImGui::InputText("Subnet IP##net", state.netSubnetIp, sizeof(state.netSubnetIp));
    ImGui::SameLine();
    ImGui::SetNextItemWidth(180);
    ImGui::InputText("Subnet Mask##net", state.netSubnetMask, sizeof(state.netSubnetMask));

    if (strlen(state.netSubnetIp) > 0) {
        ImGui::SameLine();
        if (ImGui::Button("Exclude Subnet")) {
            JsonWriter w;
            w.beginObject();
            w.key("subnetIpAddress");
            w.value(string(state.netSubnetIp));
            w.key("subnetMask");
            w.value(string(state.netSubnetMask));
            w.endObject();
            launchCall(state, client, "excludeSubnet", w.result(), "excludedSubnets");
        }

        ImGui::SameLine();
        if (ImGui::Button("Allow Subnet")) {
            JsonWriter w;
            w.beginObject();
            w.key("subnetIpAddress");
            w.value(string(state.netSubnetIp));
            w.endObject();
            launchCall(state, client, "allowSubnet", w.result(), "excludedSubnets");
        }
    }

    ImGui::Separator();

    // --- Excluded hosts display ---
    if (!state.excludedHostsResult.empty()) {
        ImGui::Text("Excluded Hosts:");
        ImGui::TextWrapped("%s", state.excludedHostsResult.c_str());
    }

    // --- Excluded subnets display ---
    if (!state.excludedSubnetsResult.empty()) {
        ImGui::Separator();
        ImGui::Text("Excluded Subnets:");
        ImGui::TextWrapped("%s", state.excludedSubnetsResult.c_str());
    }
}


// ============================================================================
//  Raw RPC Tab
// ============================================================================

static void
drawRawRpcTab(AppState & state, AsyncRpcClient & client)
{
    ImGui::Text("JSON-RPC Console");
    ImGui::Separator();

    ImGui::SetNextItemWidth(300);
    ImGui::InputText("Method", state.rawMethod, sizeof(state.rawMethod));

    ImGui::Text("Params (JSON):");
    ImGui::InputTextMultiline("##params", state.rawParams, sizeof(state.rawParams), ImVec2(-1.0f, 120.0f));

    if (ImGui::Button("Send") && strlen(state.rawMethod) > 0)
        launchCall(state, client, state.rawMethod, state.rawParams, "raw");

    ImGui::Separator();

    if (!state.rawResult.empty()) {
        ImGui::Text("Result:");
        string formatted = formatJson(state.rawResult);
        ImGui::InputTextMultiline(
            "##rawResult", formatted.data(), formatted.size() + 1, ImVec2(-1.0f, -1.0f), ImGuiInputTextFlags_ReadOnly);
    }
}


// ============================================================================
//  Main Tab Bar
// ============================================================================

void
drawMainTabs(AppState & state, AsyncRpcClient & client)
{
    if (ImGui::BeginTabBar("MainTabs")) {
        if (ImGui::BeginTabItem("Status")) {
            drawStatusTab(state, client);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Peers")) {
            drawPeersTab(state, client);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Queries")) {
            drawQueriesTab(state, client);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Groups")) {
            drawGroupsTab(state, client);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Modules")) {
            drawModulesTab(state, client);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Network")) {
            drawNetworkTab(state, client);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Raw RPC")) {
            drawRawRpcTab(state, client);
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}


// ============================================================================
//  Log Panel
// ============================================================================

void
drawLogPanel(AppState & state)
{
    ImGui::BeginChild("LogPanel", ImVec2(0, 0), true);

    if (ImGui::Button("Clear Log"))
        state.logEntries.clear();

    ImGui::SameLine();
    ImGui::Text("Log (%zu entries)", state.logEntries.size());

    ImGui::Separator();

    ImGui::BeginChild("LogScroll", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

    for (const auto & entry : state.logEntries) {
        ImVec4 color;
        switch (entry.level) {
        case LogLevel::Error:
            color = ImVec4(1.0f, 0.3f, 0.3f, 1.0f);
            break;
        case LogLevel::Warn:
            color = ImVec4(1.0f, 1.0f, 0.3f, 1.0f);
            break;
        case LogLevel::Info:
            color = ImVec4(0.8f, 0.8f, 0.8f, 1.0f);
            break;
        }

        ImGui::PushStyleColor(ImGuiCol_Text, color);
        ImGui::TextUnformatted(("[" + entry.timestamp + "] " + entry.message).c_str());
        ImGui::PopStyleColor();
    }

    // Auto-scroll to bottom
    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        ImGui::SetScrollHereY(1.0f);

    ImGui::EndChild();
    ImGui::EndChild();
}
