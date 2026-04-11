/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <IpFilter.h>
#include <Log.h>
#include <Platform.h>
#include <ReadLock.h>
#include <WriteLock.h>
#include <format>
#include <fstream>
#include <sstream>

#ifdef ALPINE_PLATFORM_POSIX
#include <arpa/inet.h>
#else
#include <winsock2.h>
#include <ws2tcpip.h>
#endif


vector<IpFilter::t_CidrEntry> IpFilter::allowlist_s;
vector<IpFilter::t_CidrEntry> IpFilter::blocklist_s;
ReadWriteSem IpFilter::filterLock_s;
string IpFilter::allowlistFile_s;
string IpFilter::blocklistFile_s;


bool
IpFilter::initialize(const string & allowlistFile, const string & blocklistFile)
{
    allowlistFile_s = allowlistFile;
    blocklistFile_s = blocklistFile;

    WriteLock lock(filterLock_s);

    if (!allowlistFile.empty()) {
        if (!loadFile(allowlistFile, allowlist_s)) {
            Log::Error("IpFilter: failed to load allowlist from {}", allowlistFile);
        } else {
            Log::Info("IpFilter: loaded {} allowlist entries", allowlist_s.size());
        }
    }

    if (!blocklistFile.empty()) {
        if (!loadFile(blocklistFile, blocklist_s)) {
            Log::Error("IpFilter: failed to load blocklist from {}", blocklistFile);
        } else {
            Log::Info("IpFilter: loaded {} blocklist entries", blocklist_s.size());
        }
    }

    return true;
}


bool
IpFilter::isAllowed(const string & ip)
{
    ReadLock lock(filterLock_s);

    // Check blocklist first — blocked IPs are always rejected
    for (const auto & entry : blocklist_s) {
        if (matchesCidr(ip, entry)) {
            return false;
        }
    }

    // If allowlist is empty, allow all (that aren't blocked)
    if (allowlist_s.empty()) {
        return true;
    }

    // If allowlist is non-empty, IP must match at least one entry
    for (const auto & entry : allowlist_s) {
        if (matchesCidr(ip, entry)) {
            return true;
        }
    }

    return false;
}


bool
IpFilter::allowIp(const string & cidr)
{
    t_CidrEntry entry;
    if (!parseCidr(cidr, entry)) {
        return false;
    }

    WriteLock lock(filterLock_s);
    allowlist_s.push_back(std::move(entry));
    return true;
}


bool
IpFilter::blockIp(const string & cidr)
{
    t_CidrEntry entry;
    if (!parseCidr(cidr, entry)) {
        return false;
    }

    WriteLock lock(filterLock_s);
    blocklist_s.push_back(std::move(entry));
    return true;
}


bool
IpFilter::removeAllow(const string & cidr)
{
    WriteLock lock(filterLock_s);
    auto it = std::remove_if(
        allowlist_s.begin(), allowlist_s.end(), [&cidr](const t_CidrEntry & e) { return e.original == cidr; });
    if (it == allowlist_s.end()) {
        return false;
    }
    allowlist_s.erase(it, allowlist_s.end());
    return true;
}


bool
IpFilter::removeBlock(const string & cidr)
{
    WriteLock lock(filterLock_s);
    auto it = std::remove_if(
        blocklist_s.begin(), blocklist_s.end(), [&cidr](const t_CidrEntry & e) { return e.original == cidr; });
    if (it == blocklist_s.end()) {
        return false;
    }
    blocklist_s.erase(it, blocklist_s.end());
    return true;
}


vector<string>
IpFilter::getAllowlist()
{
    ReadLock lock(filterLock_s);
    vector<string> result;
    result.reserve(allowlist_s.size());
    for (const auto & e : allowlist_s) {
        result.push_back(e.original);
    }
    return result;
}


vector<string>
IpFilter::getBlocklist()
{
    ReadLock lock(filterLock_s);
    vector<string> result;
    result.reserve(blocklist_s.size());
    for (const auto & e : blocklist_s) {
        result.push_back(e.original);
    }
    return result;
}


bool
IpFilter::reload()
{
    WriteLock lock(filterLock_s);
    allowlist_s.clear();
    blocklist_s.clear();

    bool ok = true;
    if (!allowlistFile_s.empty()) {
        ok = loadFile(allowlistFile_s, allowlist_s) && ok;
    }
    if (!blocklistFile_s.empty()) {
        ok = loadFile(blocklistFile_s, blocklist_s) && ok;
    }
    return ok;
}


bool
IpFilter::parseCidr(const string & cidr, t_CidrEntry & entry)
{
    entry.original = cidr;

    auto slash = cidr.find('/');
    string ipStr;
    int prefixLen = 32;  // default: single host

    if (slash != string::npos) {
        ipStr = cidr.substr(0, slash);
        try {
            prefixLen = std::stoi(cidr.substr(slash + 1));
        } catch (...) {
            return false;
        }
        if (prefixLen < 0 || prefixLen > 32) {
            return false;
        }
    } else {
        ipStr = cidr;
    }

    struct in_addr addr
    {};
    if (inet_pton(AF_INET, ipStr.c_str(), &addr) != 1) {
        return false;
    }

    entry.network = ntohl(addr.s_addr);
    entry.mask = (prefixLen == 0) ? 0 : (~uint32_t{0} << (32 - prefixLen));
    entry.network &= entry.mask;

    return true;
}


bool
IpFilter::matchesCidr(const string & ip, const t_CidrEntry & entry)
{
    struct in_addr addr
    {};
    if (inet_pton(AF_INET, ip.c_str(), &addr) != 1) {
        return false;
    }

    uint32_t ipHost = ntohl(addr.s_addr);
    return (ipHost & entry.mask) == entry.network;
}


bool
IpFilter::loadFile(const string & filePath, vector<t_CidrEntry> & entries)
{
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return false;
    }

    string line;
    while (std::getline(file, line)) {
        // Trim whitespace
        auto start = line.find_first_not_of(" \t\r\n");
        if (start == string::npos) {
            continue;
        }
        auto end = line.find_last_not_of(" \t\r\n");
        line = line.substr(start, end - start + 1);

        // Skip comments and empty lines
        if (line.empty() || line.front() == '#') {
            continue;
        }

        t_CidrEntry entry;
        if (parseCidr(line, entry)) {
            entries.push_back(std::move(entry));
        } else {
            Log::Error("IpFilter: invalid CIDR entry: {}", line);
        }
    }

    return true;
}
