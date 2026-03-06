/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <Platform.h>
#include <vector>


class InterfaceEnumerator
{
  public:

    enum class AddressFamily : uint8_t {
        IPv4,
        IPv6,
        Any
    };

    struct InterfaceInfo {
        string          name;
        string          ipAddress;
        string          netmask;
        uint            flags;
        AddressFamily   family;
        bool            isUp;
        bool            isVpn;
        bool            isLoopback;
    };


    static bool enumerate (std::vector<InterfaceInfo> & interfaces,
                           AddressFamily family = AddressFamily::Any);

    static bool findByName (const string & name, InterfaceInfo & info,
                            AddressFamily family = AddressFamily::IPv4);

    static bool findVpnInterfaces (std::vector<InterfaceInfo> & vpnInterfaces);

    static bool getInterfaceAddress (const string & interfaceName,
                                     string & ipAddress,
                                     AddressFamily family = AddressFamily::IPv4);
};
