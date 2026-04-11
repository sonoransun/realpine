/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <InterfaceEnumerator.h>
#include <Log.h>

#include <cstring>

#ifdef ALPINE_PLATFORM_WINDOWS

#include <iphlpapi.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "iphlpapi.lib")

#else

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <sys/socket.h>

#endif


static bool
isVpnInterfaceName(const string & name)
{
    return name.starts_with("tun") || name.starts_with("tap") || name.starts_with("wg") || name.starts_with("utun") ||
           name.starts_with("ppp");
}


#ifdef ALPINE_PLATFORM_WINDOWS

bool
InterfaceEnumerator::enumerate(std::vector<InterfaceInfo> & interfaces, AddressFamily family)
{
    interfaces.clear();

    // Use AF_UNSPEC to get both IPv4 and IPv6 addresses
    ULONG requestFamily = AF_UNSPEC;
    if (family == AddressFamily::IPv4)
        requestFamily = AF_INET;
    else if (family == AddressFamily::IPv6)
        requestFamily = AF_INET6;

    ULONG bufferSize = 15000;
    PIP_ADAPTER_ADDRESSES addresses = nullptr;
    ULONG result = 0;

    // Retry with larger buffer if needed
    for (int attempt = 0; attempt < 3; ++attempt) {
        addresses = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(new byte[bufferSize]);

        result = GetAdaptersAddresses(requestFamily, GAP_FLAG_SKIP_DNS_SERVER, nullptr, addresses, &bufferSize);
        if (result == ERROR_SUCCESS)
            break;

        delete[] reinterpret_cast<byte *>(addresses);
        addresses = nullptr;

        if (result != ERROR_BUFFER_OVERFLOW)
            break;
    }

    if (result != ERROR_SUCCESS || !addresses) {
        Log::Error("InterfaceEnumerator: GetAdaptersAddresses failed.");
        return false;
    }

    for (auto adapter = addresses; adapter; adapter = adapter->Next) {
        for (auto unicast = adapter->FirstUnicastAddress; unicast; unicast = unicast->Next) {

            auto addrFamily = unicast->Address.lpSockaddr->sa_family;

            if (addrFamily == AF_INET) {
                auto * sockaddr = reinterpret_cast<sockaddr_in *>(unicast->Address.lpSockaddr);

                char addrBuf[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &sockaddr->sin_addr, addrBuf, sizeof(addrBuf));

                InterfaceInfo info;
                char nameBuf[256] = {};
                WideCharToMultiByte(CP_UTF8, 0, adapter->FriendlyName, -1, nameBuf, sizeof(nameBuf), nullptr, nullptr);
                info.name = nameBuf;
                info.ipAddress = addrBuf;
                info.flags = 0;
                info.family = AddressFamily::IPv4;
                info.isUp = (adapter->OperStatus == IfOperStatusUp);
                info.isLoopback = (adapter->IfType == IF_TYPE_SOFTWARE_LOOPBACK);
                info.isVpn = isVpnInterfaceName(info.name) || (adapter->IfType == IF_TYPE_TUNNEL) ||
                             (adapter->IfType == IF_TYPE_PPP);

                // Compute netmask from prefix length
                if (unicast->OnLinkPrefixLength <= 32) {
                    uint32_t mask = 0;
                    if (unicast->OnLinkPrefixLength > 0)
                        mask = htonl(~((1u << (32 - unicast->OnLinkPrefixLength)) - 1));
                    char maskBuf[INET_ADDRSTRLEN];
                    struct in_addr maskAddr;
                    maskAddr.s_addr = mask;
                    inet_ntop(AF_INET, &maskAddr, maskBuf, sizeof(maskBuf));
                    info.netmask = maskBuf;
                }

                interfaces.push_back(info);
            } else if (addrFamily == AF_INET6) {
                auto * sockaddr6 = reinterpret_cast<sockaddr_in6 *>(unicast->Address.lpSockaddr);

                char addrBuf[INET6_ADDRSTRLEN];
                inet_ntop(AF_INET6, &sockaddr6->sin6_addr, addrBuf, sizeof(addrBuf));

                InterfaceInfo info;
                char nameBuf[256] = {};
                WideCharToMultiByte(CP_UTF8, 0, adapter->FriendlyName, -1, nameBuf, sizeof(nameBuf), nullptr, nullptr);
                info.name = nameBuf;
                info.ipAddress = addrBuf;
                info.flags = 0;
                info.family = AddressFamily::IPv6;
                info.isUp = (adapter->OperStatus == IfOperStatusUp);
                info.isLoopback = (adapter->IfType == IF_TYPE_SOFTWARE_LOOPBACK);
                info.isVpn = isVpnInterfaceName(info.name) || (adapter->IfType == IF_TYPE_TUNNEL) ||
                             (adapter->IfType == IF_TYPE_PPP);

                // Prefix length as netmask string (e.g. "64")
                info.netmask = std::to_string(unicast->OnLinkPrefixLength);

                interfaces.push_back(info);
            }
        }
    }

    delete[] reinterpret_cast<byte *>(addresses);
    return true;
}

#else  // POSIX

bool
InterfaceEnumerator::enumerate(std::vector<InterfaceInfo> & interfaces, AddressFamily family)
{
    interfaces.clear();

    struct ifaddrs * ifAddrList = nullptr;

    if (getifaddrs(&ifAddrList) == -1) {
        Log::Error("InterfaceEnumerator: getifaddrs() failed.");
        return false;
    }

    for (auto * ifa = ifAddrList; ifa; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr)
            continue;

        auto addrFamily = ifa->ifa_addr->sa_family;

        if (addrFamily == AF_INET && family != AddressFamily::IPv6) {

            auto * sockaddr = reinterpret_cast<struct sockaddr_in *>(ifa->ifa_addr);

            char addrBuf[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &sockaddr->sin_addr, addrBuf, sizeof(addrBuf));

            InterfaceInfo info;
            info.name = ifa->ifa_name;
            info.ipAddress = addrBuf;
            info.flags = ifa->ifa_flags;
            info.family = AddressFamily::IPv4;
            info.isUp = (ifa->ifa_flags & IFF_UP) != 0;
            info.isLoopback = (ifa->ifa_flags & IFF_LOOPBACK) != 0;
            info.isVpn = isVpnInterfaceName(info.name) || ((ifa->ifa_flags & IFF_POINTOPOINT) != 0 && !info.isLoopback);

            if (ifa->ifa_netmask) {
                auto * maskAddr = reinterpret_cast<struct sockaddr_in *>(ifa->ifa_netmask);
                char maskBuf[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &maskAddr->sin_addr, maskBuf, sizeof(maskBuf));
                info.netmask = maskBuf;
            }

            interfaces.push_back(info);
        } else if (addrFamily == AF_INET6 && family != AddressFamily::IPv4) {

            auto * sockaddr6 = reinterpret_cast<struct sockaddr_in6 *>(ifa->ifa_addr);

            char addrBuf[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, &sockaddr6->sin6_addr, addrBuf, sizeof(addrBuf));

            InterfaceInfo info;
            info.name = ifa->ifa_name;
            info.ipAddress = addrBuf;
            info.flags = ifa->ifa_flags;
            info.family = AddressFamily::IPv6;
            info.isUp = (ifa->ifa_flags & IFF_UP) != 0;
            info.isLoopback = (ifa->ifa_flags & IFF_LOOPBACK) != 0;
            info.isVpn = isVpnInterfaceName(info.name) || ((ifa->ifa_flags & IFF_POINTOPOINT) != 0 && !info.isLoopback);

            // For IPv6, store prefix length from netmask
            if (ifa->ifa_netmask) {
                auto * mask6 = reinterpret_cast<struct sockaddr_in6 *>(ifa->ifa_netmask);
                int prefixLen = 0;
                for (int i = 0; i < 16; ++i) {
                    uint8_t b = mask6->sin6_addr.s6_addr[i];
                    while (b) {
                        prefixLen += (b & 1);
                        b >>= 1;
                    }
                }
                info.netmask = std::to_string(prefixLen);
            }

            interfaces.push_back(info);
        }
    }

    freeifaddrs(ifAddrList);
    return true;
}

#endif


bool
InterfaceEnumerator::findByName(const string & name, InterfaceInfo & info, AddressFamily family)
{
    std::vector<InterfaceInfo> interfaces;

    if (!enumerate(interfaces, family))
        return false;

    for (const auto & iface : interfaces) {
        if (iface.name == name) {
            info = iface;
            return true;
        }
    }

    return false;
}


bool
InterfaceEnumerator::findVpnInterfaces(std::vector<InterfaceInfo> & vpnInterfaces)
{
    vpnInterfaces.clear();

    std::vector<InterfaceInfo> interfaces;

    if (!enumerate(interfaces))
        return false;

    for (const auto & iface : interfaces) {
        if (iface.isVpn && iface.isUp && !iface.isLoopback)
            vpnInterfaces.push_back(iface);
    }

    return true;
}


bool
InterfaceEnumerator::getInterfaceAddress(const string & interfaceName, string & ipAddress, AddressFamily family)
{
    InterfaceInfo info;

    if (!findByName(interfaceName, info, family))
        return false;

    ipAddress = info.ipAddress;
    return true;
}
