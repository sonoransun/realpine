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



