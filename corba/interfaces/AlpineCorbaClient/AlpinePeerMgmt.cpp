/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AlpineCorbaClient.h>
#include <ReadLock.h>
#include <WriteLock.h>
#include <Log.h>
#include <StringUtils.h>



void  
AlpineCorbaClient::AlpinePeerMgmt::getExtendedPeerList (Alpine::t_DtcpPeerIdList_out  peerIdList,
                                                        CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_AlpineStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaClient::AlpinePeerMgmt::getExtendedPeerList invoked.");
#endif

    ReadLock  lock(AlpineCorbaClient::dataLock_s);

    if (AlpineCorbaClient::!initialized_s) {
        Log::Error ("AlpinePeerMgmt method invoked before AlpineCorbaClient initialization!");
        CORBA::BAD_INV_ORDER  ex;
        ExThrow (ex);
        return;
    }

    AlpineCorbaClient::alpinePeerMgmtRef_s->getExtendedPeerList (peerIdList, ExTryEnv);
}



void  
AlpineCorbaClient::AlpinePeerMgmt::getPeerInformation (Alpine::t_AlpinePeerInfo_out  peerInfo,
                                                       CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_InvalidPeerId,
                  Alpine::e_AlpineStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaClient::AlpinePeerMgmt::getPeerInformation invoked.");
#endif

    ReadLock  lock(AlpineCorbaClient::dataLock_s);

    if (AlpineCorbaClient::!initialized_s) {
        Log::Error ("AlpinePeerMgmt method invoked before AlpineCorbaClient initialization!");
        CORBA::BAD_INV_ORDER  ex;
        ExThrow (ex);
        return;
    }

    AlpineCorbaClient::alpinePeerMgmtRef_s->getPeerInformation (peerInfo, ExTryEnv);
}



void 
AlpineCorbaClient::AlpinePeerMgmt::updatePeerInformation (const Alpine::t_AlpinePeerInfo &  peerInfo,
                                                          CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_InvalidPeerId,
                  Alpine::e_AlpineStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaClient::AlpinePeerMgmt::updatePeerInformation invoked.");
#endif

    ReadLock  lock(AlpineCorbaClient::dataLock_s);

    if (AlpineCorbaClient::!initialized_s) {
        Log::Error ("AlpinePeerMgmt method invoked before AlpineCorbaClient initialization!");
        CORBA::BAD_INV_ORDER  ex;
        ExThrow (ex);
        return;
    }

    AlpineCorbaClient::alpinePeerMgmtRef_s->updatePeerInformation (peerInfo, ExTryEnv);
}



