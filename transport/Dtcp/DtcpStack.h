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
#include <ReadWriteSem.h>
#include <vector>
#include <memory>
#include <OptHash.h>


class DtcpBaseUdpTransport;
class DtcpBaseConnMux;
class DtcpBaseConnTransport;


class DtcpStack
{
  public:

    DtcpStack () = default;
    ~DtcpStack () = default;



    // public types
    //
    using t_ConnTransportList = vector<DtcpBaseConnTransport *>;

    using t_IpAddressList = vector<ulong>;

    struct t_SubnetAddress {
        ulong  ipAddress;
        ulong  netMask;
    };

    using t_SubnetAddressList = vector<t_SubnetAddress *>;



    static bool  initialize ();


    static uint  numConnTransports ();

    static bool  exists (ulong   ipAddress,
                         ushort  port);

    static bool  exists (ulong  transportId);

    static bool  getAllConnTransports (t_ConnTransportList & transportList);

    static bool  locateTransport (ulong                    transportId,
                                  DtcpBaseConnTransport *& transport);

    static bool  locateTransport (ulong                    ipAddress,
                                  ushort                   port,
                                  DtcpBaseConnTransport *& transport);

    static bool  createTransport  (ulong                    ipAddress,
                                   ushort                   port,
                                   DtcpBaseConnTransport *& transport);

    static bool  hostIsExcluded (ulong  ipAddress);

    static bool  subnetIsExcluded (ulong  subnetIpAddress);

    static bool  excludeHost (ulong ipAddress);

    static bool  excludeSubnet (ulong  subnetIpAddress,
                                ulong  subnetMask);

    static bool  allowHost (ulong ipAddress);

    static bool  allowSubnet (ulong  subnetIpAddress);

    static bool  getAllExcludedHosts (t_IpAddressList & ipAddressList);

    static bool  getAllExcludedSubnets (t_SubnetAddressList & subnetList);




    // member types
    //
    using t_IpAddressSet = std::unordered_set < ulong,
                       OptHash<ulong>,
                       equal_to<ulong> >;


    using t_SubnetAddressIndex = std::unordered_map < ulong,
                       t_SubnetAddress *,
                       OptHash<ulong>,
                       equal_to<ulong> >;


    using t_ConnTransportIndex = std::unordered_map < ulong,
                       DtcpBaseConnTransport *,
                       OptHash<ulong>,
                       equal_to<ulong> >;

    using t_ConnTransportIndexPair = std::pair <ulong, DtcpBaseConnTransport *>;



  private:

    static DtcpBaseUdpTransport *  udpTransport_s;
    static DtcpBaseConnMux *       connMux_s;
    static ulong                   currTransportId_s;
    static ReadWriteSem            dataLock_s;

    static std::unique_ptr<t_IpAddressSet>        ipAddressFilter_s;
    static std::unique_ptr<t_SubnetAddressIndex>  subnetFilter_s;
    static ReadWriteSem            filterLock_s;

    static std::unique_ptr<t_ConnTransportIndex>  transportIndex_s;
    static ReadWriteSem            transportIndexLock_s;



    static bool  getNextTransportId (ulong & transportId);

    static bool  registerUdpTransport (DtcpBaseUdpTransport * udpTransport,
                                       DtcpBaseConnMux *      connMux);

    static bool  registerConnTransport (DtcpBaseConnTransport * transport);

    static bool  deactivateConnTransport (DtcpBaseConnTransport * transport);



    friend class DtcpBaseConnMux;
    friend class DtcpBaseUdpTransport;

};

