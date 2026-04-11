/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <ReadWriteSem.h>
#include <vector>


class IpFilter
{
  public:
    static bool initialize(const string & allowlistFile, const string & blocklistFile);

    // Check if an IP address is allowed.
    // Returns true if: (allowlist empty OR ip in allowlist) AND ip NOT in blocklist.
    [[nodiscard]] static bool isAllowed(const string & ip);

    // Dynamic management (thread-safe)
    static bool allowIp(const string & cidr);
    static bool blockIp(const string & cidr);
    static bool removeAllow(const string & cidr);
    static bool removeBlock(const string & cidr);

    // List current rules
    static vector<string> getAllowlist();
    static vector<string> getBlocklist();

    // Reload from files
    static bool reload();


  private:
    struct t_CidrEntry
    {
        uint32_t network;
        uint32_t mask;
        string original;  // original CIDR string for display
    };

    static bool parseCidr(const string & cidr, t_CidrEntry & entry);
    static bool matchesCidr(const string & ip, const t_CidrEntry & entry);
    static bool loadFile(const string & filePath, vector<t_CidrEntry> & entries);

    static vector<t_CidrEntry> allowlist_s;
    static vector<t_CidrEntry> blocklist_s;
    static ReadWriteSem filterLock_s;
    static string allowlistFile_s;
    static string blocklistFile_s;
};
