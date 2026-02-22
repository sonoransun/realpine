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


#pragma once

#include <string>
#include <vector>
#include <future>
#include <optional>
#include <chrono>
#include <cstring>


enum class LogLevel { Info, Warn, Error };

struct LogEntry
{
    std::string   timestamp;
    LogLevel      level;
    std::string   message;
};


struct AppState
{
    // Connection
    char         host[256]     = "127.0.0.1";
    int          port          = 8600;
    bool         connected     = false;
    std::string  statusMessage = "Not connected";

    // Async RPC
    std::optional<std::future<bool>>  pendingCall;
    std::string   pendingResult;
    bool          callInProgress = false;
    std::string   callTarget;       // which tab/action initiated the call

    // Tab selection
    int  activeTab = 0;

    // Log
    std::vector<LogEntry>  logEntries;

    // --- Status tab ---
    std::string  statusResult;

    // --- Peers tab ---
    char         peerIp[256]   = "";
    int          peerPort      = 5000;
    char         peerId[64]    = "";
    std::string  peersResult;
    std::string  peerInfoResult;

    // --- Queries tab ---
    char         queryString[512]  = "";
    char         queryGroup[256]   = "";
    int          queryAutoHalt     = 0;
    int          queryPeerDescMax  = 0;
    char         queryId[64]       = "";
    std::string  queryResult;
    std::string  queryStatusResult;

    // --- Groups tab ---
    char         groupName[256]    = "";
    char         groupDesc[512]    = "";
    char         groupId[64]       = "";
    char         groupPeerId[64]   = "";
    std::string  groupsResult;
    std::string  groupInfoResult;

    // --- Modules tab ---
    char         moduleLibPath[512]  = "";
    char         moduleSymbol[256]   = "";
    char         moduleId[64]        = "";
    std::string  modulesResult;
    std::string  moduleInfoResult;

    // --- Network tab ---
    char         netIpAddress[256]       = "";
    char         netSubnetIp[256]        = "";
    char         netSubnetMask[256]      = "";
    std::string  excludedHostsResult;
    std::string  excludedSubnetsResult;

    // --- Raw RPC tab ---
    char         rawMethod[128]    = "";
    char         rawParams[4096]   = "{}";
    std::string  rawResult;
};
