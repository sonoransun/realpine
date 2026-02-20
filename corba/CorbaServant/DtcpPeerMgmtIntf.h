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
#include <AlpineCorbaClient.h>
#include <CorbaUtils.h>
#include <string>
#include <vector>


class DtcpPeerMgmtIntf
{
  public:


    // Public types
    //
    using t_IpAddressList = vector<string>;

    struct t_SubnetAddress {
        string  ipAddress;
        string  netMask;
    };

    using t_SubnetAddressList = vector<t_SubnetAddress>;

    using t_DtcpPeerIdList = vector<ulong>;


    struct t_DtcpPeerLocation {
        string  ipAddress;
        ushort  port;
    };

    using t_DtcpPeerLocationList = vector<t_DtcpPeerLocation>;


    struct t_DtcpPeerStatus {
        string  ipAddress;
        ushort  port;
        ulong   lastRecvTime;
        ulong   lastSendTime;
        ulong   avgBandwidth;
        ulong   peakBandwidth;
    };



    // Supported interface operations
    //
    // NOTE: the PeerID is referred to as the transport ID within 
    // the DTCP stack code.
    //
    static bool  addDtcpPeer (const string &  ipAddress,
                              ushort          port);

    static bool  getDtcpPeerId (const string &  ipAddress,
                                ushort          port,
                                ulong &         peerId);

    static bool  getDtcpPeerStatus (ulong               peerId,
                                    t_DtcpPeerStatus &  status);

    static bool  getAllDtcpPeerIds (t_DtcpPeerIdList &  peerIdList);

    static bool  activateDtcpPeer (ulong  peerId);

    static bool  deactivateDtcpPeer (ulong  peerId);

    static bool  pingDtcpPeer (ulong  peerId);

    static bool  excludeHost (const string & ipAddress);

    static bool  excludeSubnet (const string &  subnetIpAddress,
                                const string &  subnetMask);

    static bool  allowHost (const string & ipAddress);

    static bool  allowSubnet (const string &  subnetIpAddress);

    static bool  listExcludedHosts (t_IpAddressList & ipAddressList);

    static bool  listExcludedSubnets (t_SubnetAddressList &  subnetAddressList);


  private:

};

