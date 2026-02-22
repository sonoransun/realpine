/// Copyright (C) 2026 sonoransun — see LICENCE.txt


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



    // Register additional wireless transports
    //
    static bool  registerAdditionalUdpTransport (DtcpBaseUdpTransport * transport,
                                                  DtcpBaseConnMux *      connMux);


  private:

    static DtcpBaseUdpTransport *  udpTransport_s;          // primary (unicast)
    static DtcpBaseConnMux *       connMux_s;               // primary (unicast)
    static ulong                   currTransportId_s;
    static ReadWriteSem            dataLock_s;

    // Additional wireless transports
    //
    static std::vector<DtcpBaseUdpTransport *>  additionalTransports_s;
    static std::vector<DtcpBaseConnMux *>       additionalMuxes_s;

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

