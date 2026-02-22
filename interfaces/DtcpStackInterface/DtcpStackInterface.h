/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <string>
#include <vector>


class DtcpBaseUdpTransport;


class DtcpStackInterface
{
  public:

    DtcpStackInterface () = default;
    ~DtcpStackInterface () = default;



    // Public types
    //
    using t_IpAddressList = vector<string>;

    using t_DtcpPeerIdList = vector<ulong>;


    struct t_DtcpPeerLocation {
        string  ipAddress;
        ushort  port;
    };

    using t_DtcpPeerLocationList = vector<t_DtcpPeerLocation *>;


    struct t_DtcpPeerStatus {
        string  ipAddress;
        ushort  port;
        ulong   lastRecvTime;
        ulong   lastSendTime;
        ulong   avgBandwidth;
        ulong   peakBandwidth;
    };


    struct t_SubnetAddress {
        string  ipAddress;
        string  netMask;
    };

    using t_SubnetAddressList = vector<t_SubnetAddress *>;


    // Supported interface operations
    //
    // NOTE: the PeerID is referred to as the transport ID within 
    // the DTCP stack code.
    //
    static bool  peerExists (const string &  ipAddress,
                             ushort          port);

    static bool  peerExists (ulong  peerId);

    static bool  addDtcpPeer (const string &  ipAddress,
                              ushort          port);

    static bool  getDtcpPeerId (const string &  ipAddress,
                                ushort          port,
                                ulong &         peerId);

    static bool  getDtcpPeerStatus (ulong               peerId,
                                    t_DtcpPeerStatus &  status);

    static bool  getAllDtcpPeerIds (t_DtcpPeerIdList &  peerIdList);

    static bool  peerIsActive (ulong  peerId);

    static bool  activateDtcpPeer (ulong  peerId);

    static bool  deactivateDtcpPeer (ulong  peerId);

    static bool  pingDtcpPeer (ulong  peerId);

    static bool  hostIsExcluded (const string &  ipAddress);

    static bool  subnetIsExcluded (const string &  subnetIpAddress);

    static bool  peerIsExcluded (ulong  peerId);

    static bool  excludeHost (const string & ipAddress);

    static bool  excludeSubnet (const string & subnetIpAddress,
                                const string & subnetMask);

    static bool  allowHost (const string & ipAddress);

    static bool  allowSubnet (const string & subnetIpAddress);

    static bool  listExcludedHosts (t_IpAddressList & ipAddressList);

    static bool  listExcludedSubnets (t_SubnetAddressList & subnetAddressList);


  private:

};

