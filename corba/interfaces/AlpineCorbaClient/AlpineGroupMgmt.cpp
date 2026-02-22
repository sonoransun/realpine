/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AlpineCorbaClient.h>
#include <ReadLock.h>
#include <WriteLock.h>
#include <Log.h>
#include <StringUtils.h>



void  
AlpineCorbaClient::AlpineGroupMgmt::getUserGroupList (Alpine::t_AlpineGroupInfoList_out  groupInfoList,
                                                      CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_AlpineStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaClient::AlpineGroupMgmt::getUserGroupList invoked.");
#endif

    ReadLock  lock(AlpineCorbaClient::dataLock_s);

    if (AlpineCorbaClient::!initialized_s) {
        Log::Error ("AlpineGroupMgmt method invoked before AlpineCorbaClient initialization!");
        CORBA::BAD_INV_ORDER  ex;
        ExThrow (ex);
        return;
    }

    AlpineCorbaClient::alpineGroupMgmtRef_s->getUserGroupList (groupInfoList, ExTryEnv);
}



uint  
AlpineCorbaClient::AlpineGroupMgmt::createUserGroup (const char *  groupName,
                                                     const char *  description,
                                                     CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_GroupAlreadyExists,
                  Alpine::e_AlpineStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaClient::AlpineGroupMgmt::createUserGroup invoked.");
#endif

    ReadLock  lock(AlpineCorbaClient::dataLock_s);

    if (AlpineCorbaClient::!initialized_s) {
        Log::Error ("AlpineGroupMgmt method invoked before AlpineCorbaClient initialization!");
        CORBA::BAD_INV_ORDER  ex;
        ExThrow (ex);
        return false;
    }

    uint  retVal;
    retVal = AlpineCorbaClient::alpineGroupMgmtRef_s->createUserGroup (groupName, description, ExTryEnv);


    return retVal;
}



void  
AlpineCorbaClient::AlpineGroupMgmt::destroyUserGroup (uint  groupId,
                                                      CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_InvalidGroupId,
                  Alpine::e_AlpineStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaClient::AlpineGroupMgmt::destroyUserGroup invoked.");
#endif

    ReadLock  lock(AlpineCorbaClient::dataLock_s);

    if (AlpineCorbaClient::!initialized_s) {
        Log::Error ("AlpineGroupMgmt method invoked before AlpineCorbaClient initialization!");
        CORBA::BAD_INV_ORDER  ex;
        ExThrow (ex);
        return;
    }

    AlpineCorbaClient::alpineGroupMgmtRef_s->destroyUserGroup (groupId, ExTryEnv);
}



void  
AlpineCorbaClient::AlpineGroupMgmt::getPeerUserGroupList (uint                               peerId,
                                                          Alpine::t_AlpineGroupInfoList_out  groupInfoList,
                                                          CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_InvalidPeerId,
                  Alpine::e_AlpineStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaClient::AlpineGroupMgmt::getPeerUserGroupList invoked.");
#endif

    ReadLock  lock(AlpineCorbaClient::dataLock_s);

    if (AlpineCorbaClient::!initialized_s) {
        Log::Error ("AlpineGroupMgmt method invoked before AlpineCorbaClient initialization!");
        CORBA::BAD_INV_ORDER  ex;
        ExThrow (ex);
        return;
    }

    AlpineCorbaClient::alpineGroupMgmtRef_s->getPeerUserGroupList (peerId, groupInfoList, ExTryEnv);
}



void  
AlpineCorbaClient::AlpineGroupMgmt::addPeerToGroup (uint  peerId,
                                                    uint  groupId,
                                                    CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_InvalidPeerId,
                  Alpine::e_InvalidGroupId,
                  Alpine::e_AlpineStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaClient::AlpineGroupMgmt::addPeerToGroup invoked.");
#endif

    ReadLock  lock(AlpineCorbaClient::dataLock_s);

    if (AlpineCorbaClient::!initialized_s) {
        Log::Error ("AlpineGroupMgmt method invoked before AlpineCorbaClient initialization!");
        CORBA::BAD_INV_ORDER  ex;
        ExThrow (ex);
        return;
    }

    AlpineCorbaClient::alpineGroupMgmtRef_s->addPeerToGroup (peerId, groupId, ExTryEnv);
}



void  
AlpineCorbaClient::AlpineGroupMgmt::removePeerFromGroup (uint  peerId,
                                                         uint  groupId,
                                                         CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_InvalidPeerId,
                  Alpine::e_InvalidGroupId,
                  Alpine::e_AlpineStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaClient::AlpineGroupMgmt::removePeerFromGroup invoked.");
#endif

    ReadLock  lock(AlpineCorbaClient::dataLock_s);

    if (AlpineCorbaClient::!initialized_s) {
        Log::Error ("AlpineGroupMgmt method invoked before AlpineCorbaClient initialization!");
        CORBA::BAD_INV_ORDER  ex;
        ExThrow (ex);
        return;
    }

    AlpineCorbaClient::alpineGroupMgmtRef_s->removePeerFromGroup (peerId, groupId, ExTryEnv);
}



