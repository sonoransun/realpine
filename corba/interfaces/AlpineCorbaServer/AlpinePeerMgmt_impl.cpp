/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AlpineCorba_impl.h>
#include <NetUtils.h>
#include <Log.h>
#include <StringUtils.h>



AlpineCorba_impl::AlpinePeerMgmt::AlpinePeerMgmt (const CORBA::ORB_var &          orb,
                                                  const PortableServer::POA_var & poa,
                                                  AlpineCorba_impl *              implParent)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::AlpinePeerMgmt constructor invoked.");
#endif

    orb_        = orb;
    poa_        = poa;
    implParent_ = implParent;
}



AlpineCorba_impl::AlpinePeerMgmt::~AlpinePeerMgmt ()
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::AlpinePeerMgmt destructor invoked.");
#endif
}



void  
AlpineCorba_impl::AlpinePeerMgmt::getExtendedPeerList (Alpine::t_DtcpPeerIdList_out  peerIdList,
                                                       CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_AlpineStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::AlpinePeerMgmt::getExtendedPeerList invoked.");
#endif

    peerIdList = new Alpine::t_DtcpPeerIdList;
}



void  
AlpineCorba_impl::AlpinePeerMgmt::getPeerInformation (Alpine::t_AlpinePeerInfo_out  peerInfo,
                                                      CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_InvalidPeerId,
                  Alpine::e_AlpineStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::AlpinePeerMgmt::getPeerInformation invoked.");
#endif

    peerInfo = new Alpine::t_AlpinePeerInfo;
}



void 
AlpineCorba_impl::AlpinePeerMgmt::updatePeerInformation (const Alpine::t_AlpinePeerInfo &  peerInfo,
                                                         CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_InvalidPeerId,
                  Alpine::e_AlpineStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::AlpinePeerMgmt::updatePeerInformation invoked.");
#endif
}



