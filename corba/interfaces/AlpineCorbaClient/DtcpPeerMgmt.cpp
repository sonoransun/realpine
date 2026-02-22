/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AlpineCorbaClient.h>
#include <ReadLock.h>
#include <WriteLock.h>
#include <Log.h>
#include <StringUtils.h>



void  
AlpineCorbaClient::DtcpPeerMgmt::addDtcpPeer (const char *  ipAddress,
                                              ushort        port,
                                              CORBA::Environment &ExTryEnv)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_InvalidAddress,
                  Alpine::e_PeerAlreadyExists,
                  Alpine::e_DtcpStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaClient::DtcpPeerMgmt::addDtcpPeer invoked.");
#endif

    ReadLock  lock(AlpineCorbaClient::dataLock_s);

    if (AlpineCorbaClient::!initialized_s) {
        Log::Error ("DtcpPeerMgmt method invoked before AlpineCorbaClient initialization!");
        ExThrow (CORBA::BAD_INV_ORDER ());
        return;
    }

    AlpineCorbaClient::dtcpPeerMgmtRef_s->addDtcpPeer (ipAddress, port, ExTryEnv);
}



uint  
AlpineCorbaClient::DtcpPeerMgmt::getDtcpPeerId (const char *  ipAddress,
                                                ushort        port,
                                                CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_InvalidAddress,
                  Alpine::e_PeerDoesNotExists,
                  Alpine::e_DtcpStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaClient::DtcpPeerMgmt::getDtcpPeerId invoked.");
#endif

    ReadLock  lock(AlpineCorbaClient::dataLock_s);

    if (AlpineCorbaClient::!initialized_s) {
        Log::Error ("DtcpPeerMgmt method invoked before AlpineCorbaClient initialization!");
        CORBA::BAD_INV_ORDER  ex;
        ExThrow (ex);
        return false;
    }

    uint  retVal;
    retVal = AlpineCorbaClient::dtcpPeerMgmtRef_s->getDtcpPeerId (ipAddress, port, ExTryEnv);


    return retVal;
}



void  
AlpineCorbaClient::DtcpPeerMgmt::getDtcpPeerStatus (uint                            peerId,
                                                    Alpine::t_DtcpPeerStatus_out    peerStatus,
                                                    CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_PeerDoesNotExists,
                  Alpine::e_DtcpStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaClient::DtcpPeerMgmt::getDtcpPeerStatus invoked.");
#endif

    ReadLock  lock(AlpineCorbaClient::dataLock_s);

    if (AlpineCorbaClient::!initialized_s) {
        Log::Error ("DtcpPeerMgmt method invoked before AlpineCorbaClient initialization!");
        CORBA::BAD_INV_ORDER  ex;
        ExThrow (ex);
        return;
    }

    AlpineCorbaClient::dtcpPeerMgmtRef_s->getDtcpPeerStatus (peerId, peerStatus, ExTryEnv);
}



void  
AlpineCorbaClient::DtcpPeerMgmt::getAllDtcpPeerIds (Alpine::t_DtcpPeerIdList_out      peerIdList,
                                                    CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_DtcpStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaClient::DtcpPeerMgmt::getAllDtcpPeerIds invoked.");
#endif

    ReadLock  lock(AlpineCorbaClient::dataLock_s);

    if (AlpineCorbaClient::!initialized_s) {
        Log::Error ("DtcpPeerMgmt method invoked before AlpineCorbaClient initialization!");
        CORBA::BAD_INV_ORDER  ex;
        ExThrow (ex);
        return;
    }

    AlpineCorbaClient::dtcpPeerMgmtRef_s->getAllDtcpPeerIds (peerIdList, ExTryEnv);
}



void  
AlpineCorbaClient::DtcpPeerMgmt::activateDtcpPeer (uint   peerId,
                                                   CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_DtcpStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaClient::DtcpPeerMgmt::activateDtcpPeer invoked.");
#endif

    ReadLock  lock(AlpineCorbaClient::dataLock_s);

    if (AlpineCorbaClient::!initialized_s) {
        Log::Error ("DtcpPeerMgmt method invoked before AlpineCorbaClient initialization!");
        CORBA::BAD_INV_ORDER  ex;
        ExThrow (ex);
        return;
    }

    AlpineCorbaClient::dtcpPeerMgmtRef_s->activateDtcpPeer (peerId, ExTryEnv);
}



void  
AlpineCorbaClient::DtcpPeerMgmt::deactivateDtcpPeer (uint   peerId,
                                                     CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_DtcpStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaClient::DtcpPeerMgmt::deactivateDtcpPeer invoked.");
#endif

    ReadLock  lock(AlpineCorbaClient::dataLock_s);

    if (AlpineCorbaClient::!initialized_s) {
        Log::Error ("DtcpPeerMgmt method invoked before AlpineCorbaClient initialization!");
        CORBA::BAD_INV_ORDER  ex;
        ExThrow (ex);
        return;
    }

    AlpineCorbaClient::dtcpPeerMgmtRef_s->deactivateDtcpPeer (peerId, ExTryEnv);
}



CORBA::Boolean  
AlpineCorbaClient::DtcpPeerMgmt::pingDtcpPeer (uint   peerId,
                                               CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_PeerDoesNotExists,
                  Alpine::e_DtcpStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaClient::DtcpPeerMgmt::pingDtcpPeer invoked.");
#endif

    ReadLock  lock(AlpineCorbaClient::dataLock_s);

    if (AlpineCorbaClient::!initialized_s) {
        Log::Error ("DtcpPeerMgmt method invoked before AlpineCorbaClient initialization!");
        CORBA::BAD_INV_ORDER  ex;
        ExThrow (ex);
        return;
    }

    CORBA::Boolean  retVal;
    retVal = AlpineCorbaClient::dtcpPeerMgmtRef_s->pingDtcpPeer (peerId, ExTryEnv);


    return retVal;
}



void  
AlpineCorbaClient::DtcpPeerMgmt::excludeHost (const char *  ipAddress,
                                              CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_InvalidAddress,
                  Alpine::e_DtcpStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaClient::DtcpPeerMgmt::excludeHost invoked.");
#endif

    ReadLock  lock(AlpineCorbaClient::dataLock_s);

    if (AlpineCorbaClient::!initialized_s) {
        Log::Error ("DtcpPeerMgmt method invoked before AlpineCorbaClient initialization!");
        CORBA::BAD_INV_ORDER  ex;
        ExThrow (ex);
        return;
    }

    AlpineCorbaClient::dtcpPeerMgmtRef_s->excludeHost (ipAddress, ExTryEnv);
}



void  
AlpineCorbaClient::DtcpPeerMgmt::excludeSubnet (const char *  subnetIpAddress,
                                                const char *  subnetMask,
                                                CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_InvalidAddress,
                  Alpine::e_DtcpStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaClient::DtcpPeerMgmt::excludeSubnet invoked.");
#endif

    ReadLock  lock(AlpineCorbaClient::dataLock_s);

    if (AlpineCorbaClient::!initialized_s) {
        Log::Error ("DtcpPeerMgmt method invoked before AlpineCorbaClient initialization!");
        CORBA::BAD_INV_ORDER  ex;
        ExThrow (ex);
        return;
    }

    AlpineCorbaClient::dtcpPeerMgmtRef_s->excludeSubnet (subnetIpAddress, subnetMask, ExTryEnv);
}



void  
AlpineCorbaClient::DtcpPeerMgmt::allowHost (const char *  ipAddress,
                                            CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_InvalidAddress,
                  Alpine::e_DtcpStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaClient::DtcpPeerMgmt::allowHost invoked.");
#endif

    ReadLock  lock(AlpineCorbaClient::dataLock_s);

    if (AlpineCorbaClient::!initialized_s) {
        Log::Error ("DtcpPeerMgmt method invoked before AlpineCorbaClient initialization!");
        CORBA::BAD_INV_ORDER  ex;
        ExThrow (ex);
        return;
    }

    AlpineCorbaClient::dtcpPeerMgmtRef_s->allowHost (ipAddress, ExTryEnv);
}



void  
AlpineCorbaClient::DtcpPeerMgmt::allowSubnet (const char *  subnetIpAddress,
                                              CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_InvalidAddress,
                  Alpine::e_DtcpStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaClient::DtcpPeerMgmt::allowSubnet invoked.");
#endif

    ReadLock  lock(AlpineCorbaClient::dataLock_s);

    if (AlpineCorbaClient::!initialized_s) {
        Log::Error ("DtcpPeerMgmt method invoked before AlpineCorbaClient initialization!");
        CORBA::BAD_INV_ORDER  ex;
        ExThrow (ex);
        return;
    }

    AlpineCorbaClient::dtcpPeerMgmtRef_s->allowSubnet (subnetIpAddress, ExTryEnv);
}



void  
AlpineCorbaClient::DtcpPeerMgmt::listExcludedHosts (Alpine::t_IpAddressList_out       ipAddressList,
                                                    CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_DtcpStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaClient::DtcpPeerMgmt::listExcludedHosts invoked.");
#endif

    ReadLock  lock(AlpineCorbaClient::dataLock_s);

    if (AlpineCorbaClient::!initialized_s) {
        Log::Error ("DtcpPeerMgmt method invoked before AlpineCorbaClient initialization!");
        CORBA::BAD_INV_ORDER  ex;
        ExThrow (ex);
        return;
    }

    AlpineCorbaClient::dtcpPeerMgmtRef_s->listExcludedHosts (ipAddressList, ExTryEnv);
}



void  
AlpineCorbaClient::DtcpPeerMgmt::listExcludedSubnets (Alpine::t_SubnetAddressList_out  subnetAddressList,
                                                      CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_DtcpStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaClient::DtcpPeerMgmt::listExcludedSubnets invoked.");
#endif

    ReadLock  lock(AlpineCorbaClient::dataLock_s);

    if (AlpineCorbaClient::!initialized_s) {
        Log::Error ("DtcpPeerMgmt method invoked before AlpineCorbaClient initialization!");
        CORBA::BAD_INV_ORDER  ex;
        ExThrow (ex);
        return;
    }

    AlpineCorbaClient::dtcpPeerMgmtRef_s->listExcludedSubnets (subnetAddressList, ExTryEnv);
}



