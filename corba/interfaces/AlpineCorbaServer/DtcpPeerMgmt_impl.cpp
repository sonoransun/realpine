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


#include <AlpineCorba_impl.h>
#include <DtcpStackInterface.h>
#include <NetUtils.h>
#include <Log.h>
#include <StringUtils.h>



AlpineCorba_impl::DtcpPeerMgmt::DtcpPeerMgmt (const CORBA::ORB_var &          orb,
                                              const PortableServer::POA_var & poa,
                                              AlpineCorba_impl *              implParent)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::DtcpPeerMgmt constructor invoked.");
#endif

    orb_        = orb;
    poa_        = poa;
    implParent_ = implParent;
}



AlpineCorba_impl::DtcpPeerMgmt::~DtcpPeerMgmt ()
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::DtcpPeerMgmt destructor invoked.");
#endif
}



void  
AlpineCorba_impl::DtcpPeerMgmt::addDtcpPeer (const char *         ipAddress,
                                             ushort               port,
                                             CORBA::Environment & ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_InvalidAddress,
                  Alpine::e_PeerAlreadyExists,
                  Alpine::e_DtcpStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::DtcpPeerMgmt::addDtcpPeer invoked.");
#endif

    bool   status;
    ulong  longIpAddress;

    if (ipAddress == 0) {
        Alpine::e_InvalidAddress  ex;
        ExThrow (ex);
        return;
    }

    status = NetUtils::stringIpToLong (std::string(ipAddress), longIpAddress);

    if (!status) {
        Alpine::e_InvalidAddress  ex;
        ExThrow (ex);
        return;
    }

    status = DtcpStackInterface::addDtcpPeer (std::string(ipAddress), port);

    if (!status) {
        Alpine::e_DtcpStackError  ex;
        ExThrow (ex);
    }
}



uint   
AlpineCorba_impl::DtcpPeerMgmt::getDtcpPeerId (const char *         ipAddress,
                                               ushort               port,
                                               CORBA::Environment & ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_InvalidAddress,
                  Alpine::e_PeerDoesNotExists,
                  Alpine::e_DtcpStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::DtcpPeerMgmt::getDtcpPeerId invoked.");
#endif

    bool   status;
    ulong  longIpAddress;

    if (ipAddress == 0) {
        Alpine::e_InvalidAddress  ex;
        ExThrow (ex);
        return;
    }

    status = NetUtils::stringIpToLong (std::string(ipAddress), longIpAddress);

    if (!status) {
        Alpine::e_InvalidAddress  ex;
        ExThrow (ex);
        return false;
    }

    ulong  peerId = 0;


    return peerId;
}



void  
AlpineCorba_impl::DtcpPeerMgmt::getDtcpPeerStatus (uint                            peerId,
                                                   Alpine::t_DtcpPeerStatus_out    peerStatus,
                                                   CORBA::Environment &            ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_PeerDoesNotExists,
                  Alpine::e_DtcpStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::DtcpPeerMgmt::getDtcpPeerStatus invoked.");
#endif

    bool status;
    status  = DtcpStackInterface::peerExists (peerId);

    if (!status) {
        Log::Debug ("Invalid peer ID passed in call to "
                             "AlpineCorba_impl::DtcpPeerMgmt::getDtcpPeerStatus!");

        Alpine::e_PeerDoesNotExists  ex;
        ExThrow (ex);
        return;
    }

    DtcpStackInterface::t_DtcpPeerStatus  dtcpStatus;
    status = DtcpStackInterface::getDtcpPeerStatus (peerId, dtcpStatus);

    if (!status) {
        Log::Debug ("getDtcpPeerStatus call failed in call to "
                             "AlpineCorba_impl::DtcpPeerMgmt::getDtcpPeerStatus!");

        Alpine::e_DtcpStackError  ex;
        ExThrow (ex);
        return;
    }

    peerStatus =  new Alpine::t_DtcpPeerStatus;
    peerStatus->ipAddress      = CORBA::string_dup (dtcpStatus.ipAddress.c_str());
    peerStatus->port           = dtcpStatus.port;
    peerStatus->lastRecvTime   = dtcpStatus.lastRecvTime;
    peerStatus->lastSendTime   = dtcpStatus.lastSendTime;
    peerStatus->avgBandwidth   = dtcpStatus.avgBandwidth;
    peerStatus->peakBandwidth  = dtcpStatus.peakBandwidth;
}



void  
AlpineCorba_impl::DtcpPeerMgmt::getAllDtcpPeerIds (Alpine::t_DtcpPeerIdList_out    peerIdList,
                                                   CORBA::Environment &            ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_DtcpStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::DtcpPeerMgmt::getAllDtcpPeerIds invoked.");
#endif

    peerIdList = new Alpine::t_DtcpPeerIdList;
}



void  
AlpineCorba_impl::DtcpPeerMgmt::activateDtcpPeer (uint                 peerId,
                                                  CORBA::Environment & ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_DtcpStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::DtcpPeerMgmt::activateDtcpPeer invoked.");
#endif

    bool status;
    status  = DtcpStackInterface::peerExists (peerId);

    if (!status) {
        Log::Debug ("Invalid peer ID passed in call to "
                             "AlpineCorba_impl::DtcpPeerMgmt::activateDtcpPeer!");

        Alpine::e_PeerDoesNotExists  ex;
        ExThrow (ex);
    }
}



void  
AlpineCorba_impl::DtcpPeerMgmt::deactivateDtcpPeer (uint                 peerId,
                                                    CORBA::Environment & ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_DtcpStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::DtcpPeerMgmt::deactivateDtcpPeer invoked.");
#endif

    bool status;
    status  = DtcpStackInterface::peerExists (peerId);

    if (!status) {
        Log::Debug ("Invalid peer ID passed in call to "
                             "AlpineCorba_impl::DtcpPeerMgmt::deactivateDtcpPeer!");

        Alpine::e_PeerDoesNotExists  ex;
        ExThrow (ex);
    }
}



CORBA::Boolean
AlpineCorba_impl::DtcpPeerMgmt::pingDtcpPeer (uint                 peerId,
                                              CORBA::Environment & ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_PeerDoesNotExists,
                  Alpine::e_DtcpStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::DtcpPeerMgmt::pingDtcpPeer invoked.");
#endif

    bool status;
    status  = DtcpStackInterface::peerExists (peerId);

    if (!status) {
        Log::Debug ("Invalid peer ID passed in call to "
                             "AlpineCorba_impl::DtcpPeerMgmt::pingDtcpPeer!");

        Alpine::e_PeerDoesNotExists  ex;
        ExThrow (ex);
        return;
    }


    return 1;
}



void  
AlpineCorba_impl::DtcpPeerMgmt::excludeHost (const char *         ipAddress,
                                             CORBA::Environment & ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_InvalidAddress,
                  Alpine::e_DtcpStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::DtcpPeerMgmt::excludeHost invoked.");
#endif

    bool   status;
    ulong  longIpAddress;

    if (ipAddress == 0) {
        Alpine::e_InvalidAddress  ex;
        ExThrow (ex);
        return;
    }

    status = NetUtils::stringIpToLong (std::string(ipAddress), longIpAddress);

    if (!status) {
        Alpine::e_InvalidAddress  ex;
        ExThrow (ex);
    }
}



void  
AlpineCorba_impl::DtcpPeerMgmt::excludeSubnet (const char *  subnetIpAddress,
                                               const char *  subnetMask,
                                               CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_InvalidAddress,
                  Alpine::e_DtcpStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::DtcpPeerMgmt::excludeSubnet invoked.");
#endif

    bool   status;
    ulong  longSubnetIpAddress;
    ulong  longSubnetMask;

    if ( (subnetIpAddress == 0) || (subnetMask == 0) ) {
        Alpine::e_InvalidAddress  ex;
        ExThrow (ex);
        return;
    }

    status = NetUtils::stringIpToLong (std::string(subnetIpAddress), longSubnetIpAddress);

    if (!status) {
        Alpine::e_InvalidAddress  ex;
        ExThrow (ex);
        return;
    }

    status = NetUtils::stringIpToLong (std::string(subnetMask), longSubnetMask);

    if (!status) {
        Alpine::e_InvalidAddress  ex;
        ExThrow (ex);
    }
}


void  
AlpineCorba_impl::DtcpPeerMgmt::allowHost (const char *         ipAddress,
                                           CORBA::Environment & ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_InvalidAddress,
                  Alpine::e_DtcpStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::DtcpPeerMgmt::allowHost invoked.");
#endif

    bool   status;
    ulong  longIpAddress;

    if (ipAddress == 0) {
        Alpine::e_InvalidAddress  ex;
        ExThrow (ex);
        return;
    }

    status = NetUtils::stringIpToLong (std::string(ipAddress), longIpAddress);

    if (!status) {
        Alpine::e_InvalidAddress  ex;
        ExThrow (ex);
    }
}



void  
AlpineCorba_impl::DtcpPeerMgmt::allowSubnet (const char *  subnetIpAddress,
                                             CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_InvalidAddress,
                  Alpine::e_DtcpStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::DtcpPeerMgmt::allowSubnet invoked.");
#endif

    bool   status;
    ulong  longSubnetIpAddress;

    if (subnetIpAddress == 0) {
        Alpine::e_InvalidAddress  ex;
        ExThrow (ex);
        return;
    }

    status = NetUtils::stringIpToLong (std::string(subnetIpAddress), longSubnetIpAddress);

    if (!status) {
        Alpine::e_InvalidAddress  ex;
        ExThrow (ex);
    }
}



void  
AlpineCorba_impl::DtcpPeerMgmt::listExcludedHosts (Alpine::t_IpAddressList_out    ipAddressList,
                                                   CORBA::Environment &           ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_DtcpStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::DtcpPeerMgmt::listExcludedHosts invoked.");
#endif

    ipAddressList = new Alpine::t_IpAddressList;
}



void  
AlpineCorba_impl::DtcpPeerMgmt::listExcludedSubnets (Alpine::t_SubnetAddressList_out  subnetAddressList,
                                                     CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_DtcpStackError))
{
#ifdef _VERBOSE 
    Log::Debug ("AlpineCorba_impl::DtcpPeerMgmt::listExcludedSubnets invoked.");
#endif

    subnetAddressList = new Alpine::t_SubnetAddressList;
}



