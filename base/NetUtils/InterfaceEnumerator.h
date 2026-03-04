/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <Platform.h>
#include <vector>


class InterfaceEnumerator
{
  public:

    struct InterfaceInfo {
        string  name;
        string  ipAddress;
        string  netmask;
        uint    flags;
        bool    isUp;
        bool    isVpn;
        bool    isLoopback;
    };


    static bool enumerate (std::vector<InterfaceInfo> & interfaces);

    static bool findByName (const string & name, InterfaceInfo & info);

    static bool findVpnInterfaces (std::vector<InterfaceInfo> & vpnInterfaces);

    static bool getInterfaceAddress (const string & interfaceName,
                                     string & ipAddress);
};
