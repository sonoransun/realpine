/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once

#include <chrono>
#include <cstring>
#include <future>
#include <optional>
#include <string>
#include <vector>


enum class LogLevel { Info, Warn, Error };

struct LogEntry
{
    std::string timestamp;
    LogLevel level;
    std::string message;
};


struct AppState
{
    // Connection
    char host[256] = "127.0.0.1";
    int port = 8600;
    bool connected = false;
    std::string statusMessage = "Not connected";

    // Async RPC
    std::optional<std::future<bool>> pendingCall;
    std::string pendingResult;
    bool callInProgress = false;
    std::string callTarget;  // which tab/action initiated the call

    // Tab selection
    int activeTab = 0;

    // Log
    std::vector<LogEntry> logEntries;

    // --- Status tab ---
    std::string statusResult;

    // --- Peers tab ---
    char peerIp[256] = "";
    int peerPort = 5000;
    char peerId[64] = "";
    std::string peersResult;
    std::string peerInfoResult;

    // --- Queries tab ---
    char queryString[512] = "";
    char queryGroup[256] = "";
    int queryAutoHalt = 0;
    int queryPeerDescMax = 0;
    char queryId[64] = "";
    std::string queryResult;
    std::string queryStatusResult;

    // --- Groups tab ---
    char groupName[256] = "";
    char groupDesc[512] = "";
    char groupId[64] = "";
    char groupPeerId[64] = "";
    std::string groupsResult;
    std::string groupInfoResult;

    // --- Modules tab ---
    char moduleLibPath[512] = "";
    char moduleSymbol[256] = "";
    char moduleId[64] = "";
    std::string modulesResult;
    std::string moduleInfoResult;

    // --- Network tab ---
    char netIpAddress[256] = "";
    char netSubnetIp[256] = "";
    char netSubnetMask[256] = "";
    std::string excludedHostsResult;
    std::string excludedSubnetsResult;

    // --- Raw RPC tab ---
    char rawMethod[128] = "";
    char rawParams[4096] = "{}";
    std::string rawResult;
};
