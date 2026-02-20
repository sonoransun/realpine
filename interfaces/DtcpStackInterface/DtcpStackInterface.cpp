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


#include <DtcpStackInterface.h>
#include <DtcpStack.h>
#include <DtcpBaseConnTransport.h>
#include <Log.h>
#include <StringUtils.h>
#include <NetUtils.h>



// Ctor defaulted in header


// Dtor defaulted in header



bool  
DtcpStackInterface::peerExists (const string &  ipAddress,
                                ushort          port)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpStackInterface::peerExists invoked.");
#endif

    bool   status;
    ulong  peerIpAddress;

    NetUtils::stringIpToLong (ipAddress, peerIpAddress);

    status = DtcpStack::exists (peerIpAddress, port);


    return status;
}



bool
DtcpStackInterface::peerExists (ulong  peerId)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpStackInterface::peerExists invoked.");
#endif

    bool status;
    status = DtcpStack::exists (peerId);

    return status;
}



bool  
DtcpStackInterface::addDtcpPeer (const string &  ipAddress,
                                 ushort          port)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpStackInterface::addDtcpPeer invoked.");
#endif

    bool   status;
    ulong  peerIpAddress;

    NetUtils::stringIpToLong (ipAddress, peerIpAddress);

    DtcpBaseConnTransport * newTransport;

    status = DtcpStack::createTransport (peerIpAddress,
                                         port,
                                         newTransport);


    return status;
}



bool  
DtcpStackInterface::getDtcpPeerId (const string &  ipAddress,
                                   ushort          port,
                                   ulong &         peerId)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpStackInterface::getDtcpPeerId invoked.");
#endif

    bool   status;
    ulong  peerIpAddress;

    NetUtils::stringIpToLong (ipAddress, peerIpAddress);

    DtcpBaseConnTransport *  transport;

    status = DtcpStack::locateTransport (peerIpAddress,
                                         port,
                                         transport);

    if (!status) {
        return false;
    }

    status = transport->getTransportId (peerId);


    return status;
}



bool  
DtcpStackInterface::getDtcpPeerStatus (ulong               peerId,
                                       t_DtcpPeerStatus &  status)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpStackInterface::getDtcpPeerStatus invoked.");
#endif

    bool retVal;
    DtcpBaseConnTransport *  transport;

    retVal = DtcpStack::locateTransport (peerId,
                                         transport);

    if (!retVal) {
        return false;
    }

    ulong ipAddress;
    retVal = transport->getPeerLocation (ipAddress, status.port);

    if (!retVal) {
        return false;
    }

    NetUtils::longIpToString (ipAddress, status.ipAddress);


    return retVal;
}



bool  
DtcpStackInterface::getAllDtcpPeerIds (t_DtcpPeerIdList &  peerIdList)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpStackInterface::getAllDtcpPeerIds invoked.");
#endif

    bool  status;
    ulong currId;

    DtcpStack::t_ConnTransportList  transportList;
    DtcpBaseConnTransport *         currTransport;


    status = DtcpStack::getAllConnTransports (transportList);

    if (!status) {
        return false;
    }


    peerIdList.clear ();

    for (const auto& item : transportList) {

        currTransport = item;
        currTransport->getTransportId (currId);
        peerIdList.push_back (currId);
    }


    return true;
}



bool  
DtcpStackInterface::peerIsActive (ulong  peerId)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpStackInterface::peerIsActive invoked.");
#endif

    // MRP_TEMP not supported


    return true;
}



bool  
DtcpStackInterface::activateDtcpPeer (ulong  peerId)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpStackInterface::activateDtcpPeer invoked.");
#endif

    // MRP_TEMP not supported


    return true;
}



bool  
DtcpStackInterface::deactivateDtcpPeer (ulong  peerId)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpStackInterface::deactivateDtcpPeer invoked.");
#endif

    // MRP_TEMP not supported


    return true;
}



bool  
DtcpStackInterface::pingDtcpPeer (ulong  peerId)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpStackInterface::pingDtcpPeer invoked.");
#endif

    // MRP_TEMP not supported


    return true;
}



bool  
DtcpStackInterface::hostIsExcluded (const string &  ipAddress)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpStackInterface::hostIsExcluded invoked.");
#endif

    bool   status;
    ulong  hostIpAddress;

    NetUtils::stringIpToLong (ipAddress, hostIpAddress);

    status = DtcpStack::hostIsExcluded (hostIpAddress);


    return status;
}



bool  
DtcpStackInterface::subnetIsExcluded (const string &  subnetIpAddress)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpStackInterface::hostIsExcluded invoked.");
#endif

    bool   status;
    ulong  longSubnetIpAddress;

    NetUtils::stringIpToLong (subnetIpAddress, longSubnetIpAddress);

    status = DtcpStack::subnetIsExcluded (longSubnetIpAddress);


    return status;
}



bool  
DtcpStackInterface::peerIsExcluded (ulong  peerId)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpStackInterface::peerIsExcluded invoked.");
#endif

    bool   status;
    ulong  ipAddress;
    ushort port;
    DtcpBaseConnTransport * transport;

    status = DtcpStack::locateTransport (peerId, transport);

    if (!status) {
        return false;
    }


    transport->getPeerLocation (ipAddress, port);

    status = DtcpStack::hostIsExcluded (ipAddress);


    return status;
}



bool  
DtcpStackInterface::excludeHost (const string & ipAddress)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpStackInterface::excludeHost invoked.");
#endif

    bool   status;
    ulong  hostIpAddress;

    NetUtils::stringIpToLong (ipAddress, hostIpAddress);

    status = DtcpStack::excludeHost (hostIpAddress);


    return status;
}



bool  
DtcpStackInterface::excludeSubnet (const string & subnetIpAddress,
                                   const string & subnetMask)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpStackInterface::excludeSubnet invoked.");
#endif

    bool   status;
    ulong  longSubnetIpAddress;
    ulong  longSubnetMask;

    NetUtils::stringIpToLong (subnetIpAddress, longSubnetIpAddress);
    NetUtils::stringIpToLong (subnetMask, longSubnetMask);

    status = DtcpStack::excludeSubnet (longSubnetIpAddress, longSubnetMask);


    return status;
}

    

bool  
DtcpStackInterface::allowHost (const string & ipAddress)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpStackInterface::allowHost invoked.");
#endif

    bool   status;
    ulong  hostIpAddress;

    NetUtils::stringIpToLong (ipAddress, hostIpAddress);

    status = DtcpStack::allowHost (hostIpAddress);


    return status;
}



bool  
DtcpStackInterface::allowSubnet (const string & subnetIpAddress)
{   
#ifdef _VERBOSE
    Log::Debug ("DtcpStackInterface::allowSubnet invoked.");
#endif

    bool   status;
    ulong  longSubnetIpAddress;

    NetUtils::stringIpToLong (subnetIpAddress, longSubnetIpAddress);

    status = DtcpStack::allowSubnet (longSubnetIpAddress);


    return status;
}



bool  
DtcpStackInterface::listExcludedHosts (t_IpAddressList & ipAddressList)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpStackInterface::listExcludedHosts invoked.");
#endif

    // MRP_TEMP not implemented.


    return true;
}



bool  
DtcpStackInterface::listExcludedSubnets (t_SubnetAddressList & subnetAddressList)
{   
#ifdef _VERBOSE
    Log::Debug ("DtcpStackInterface::listExcludedSubnets invoked.");
#endif

    // MRP_TEMP not implemented.


    return true;
}



