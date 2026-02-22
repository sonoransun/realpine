/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <OptHash.h>
#include <ReadWriteSem.h>
#include <set>
#include <vector>
#include <functional>


class DtcpFilter
{
  public:

    DtcpFilter () = default;
    ~DtcpFilter () = default;


    static bool  initialize ();

    static bool  validAddress (ulong  ipAddress);


    // IP Address bans
    //
    static bool  addIpAddressBan (ulong  ipAddress);

    static bool  removeIpAddressBan (ulong  ipAddress);

    static bool  numIpAddressFiltered (ulong & count);

    using t_IpAddressList = vector<ulong>;

    static bool  getFilteredList (t_IpAddressList & list);


    // IP Network bans
    //
    static bool  addNetworkBan (ulong  network,
                                ulong  mask);

    static bool  removeNetworkBan (ulong  network,
                                   ulong  mask);

    static bool  numNetworkFiltered (ulong & count);

    struct t_NetworkMaskPair {
        ulong  network;
        ulong  mask;
    };

    using t_NetworkMaskList = vector<t_NetworkMaskPair>;

    static bool  getFilteredList (t_NetworkMaskList & list);




    using t_IpAddressSet = std::unordered_set< ulong,
                       OptHash<ulong>,
                       equal_to<ulong> >;

    using t_NetworkMaskSet = std::set< ulonglong,
                  less<ulonglong> >;


  private:

    static t_IpAddressSet *     ipAddressSet_s;
    static t_NetworkMaskSet *   networkMaskSet_s;
    static bool                 initialized_s;
    static ReadWriteSem         dataLock_s;


    // Helper methods
    //
    static void  packNetworkMask (ulong        network,
                                  ulong        mask,
                                  ulonglong &  packedNetMask);

    static void  unpackNetworkMask (const ulonglong &  packedNetMask,
                                    ulong &      network,
                                    ulong &      mask);

};

