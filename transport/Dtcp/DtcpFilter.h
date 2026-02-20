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

