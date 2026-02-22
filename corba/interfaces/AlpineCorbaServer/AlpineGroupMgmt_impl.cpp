/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AlpineCorba_impl.h>
#include <NetUtils.h>
#include <Log.h>
#include <StringUtils.h>



AlpineCorba_impl::AlpineGroupMgmt::AlpineGroupMgmt (const CORBA::ORB_var &          orb,
                                                    const PortableServer::POA_var & poa,
                                                    AlpineCorba_impl *              implParent)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::AlpineGroupMgmt constructor invoked.");
#endif

    orb_        = orb;
    poa_        = poa;
    implParent_ = implParent;
}



AlpineCorba_impl::AlpineGroupMgmt::~AlpineGroupMgmt ()
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::AlpineGroupMgmt destructor invoked.");
#endif

}



void  
AlpineCorba_impl::AlpineGroupMgmt::getUserGroupList (Alpine::t_AlpineGroupInfoList_out  groupInfoList,
                                                     CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_AlpineStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::AlpineGroupMgmt::getUserGroupList invoked.");
#endif

    groupInfoList = new Alpine::t_AlpineGroupInfoList;
}



uint  
AlpineCorba_impl::AlpineGroupMgmt::createUserGroup (const char *  groupName,
                                                    const char *  description,
                                                    CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_GroupAlreadyExists,
                  Alpine::e_AlpineStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::AlpineGroupMgmt::createUserGroup invoked.");
#endif

    uint  retVal = 0;

    return retVal;
}



void  
AlpineCorba_impl::AlpineGroupMgmt::destroyUserGroup (uint  groupId,
                                                     CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_InvalidGroupId,
                  Alpine::e_AlpineStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::AlpineGroupMgmt::destroyUserGroup invoked.");
#endif

}



void  
AlpineCorba_impl::AlpineGroupMgmt::getPeerUserGroupList (uint                               peerId,
                                                         Alpine::t_AlpineGroupInfoList_out  groupInfoList,
                                                         CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_InvalidPeerId,
                  Alpine::e_AlpineStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::AlpineGroupMgmt::getPeerUserGroupList invoked.");
#endif

    groupInfoList = new Alpine::t_AlpineGroupInfoList;
}



void  
AlpineCorba_impl::AlpineGroupMgmt::addPeerToGroup (uint  peerId,
                                                   uint  groupId,
                                                   CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_InvalidPeerId,
                  Alpine::e_InvalidGroupId,
                  Alpine::e_AlpineStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::AlpineGroupMgmt::addPeerToGroup invoked.");
#endif

}



void  
AlpineCorba_impl::AlpineGroupMgmt::removePeerFromGroup (uint  peerId,
                                                        uint  groupId,
                                                        CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_InvalidPeerId,
                  Alpine::e_InvalidGroupId,
                  Alpine::e_AlpineStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::AlpineGroupMgmt::removePeerFromGroup invoked.");
#endif

}



