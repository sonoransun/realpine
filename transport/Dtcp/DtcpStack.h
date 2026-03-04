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



    [[nodiscard]] static bool  initialize ();


    static uint  numConnTransports ();

    [[nodiscard]] static bool  exists (ulong   ipAddress,
                         ushort  port);

    [[nodiscard]] static bool  exists (ulong  transportId);

    [[nodiscard]] static bool  getAllConnTransports (t_ConnTransportList & transportList);

    [[nodiscard]] static bool  locateTransport (ulong                    transportId,
                                  DtcpBaseConnTransport *& transport);

    [[nodiscard]] static bool  locateTransport (ulong                    ipAddress,
                                  ushort                   port,
                                  DtcpBaseConnTransport *& transport);

    [[nodiscard]] static bool  createTransport  (ulong                    ipAddress,
                                   ushort                   port,
                                   DtcpBaseConnTransport *& transport);

    [[nodiscard]] static bool  hostIsExcluded (ulong  ipAddress);

    [[nodiscard]] static bool  subnetIsExcluded (ulong  subnetIpAddress);

    [[nodiscard]] static bool  excludeHost (ulong ipAddress);

    [[nodiscard]] static bool  excludeSubnet (ulong  subnetIpAddress,
                                ulong  subnetMask);

    [[nodiscard]] static bool  allowHost (ulong ipAddress);

    [[nodiscard]] static bool  allowSubnet (ulong  subnetIpAddress);

    [[nodiscard]] static bool  getAllExcludedHosts (t_IpAddressList & ipAddressList);

    [[nodiscard]] static bool  getAllExcludedSubnets (t_SubnetAddressList & subnetList);




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

