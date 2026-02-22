/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AlpineCorba_impl.h>
#include <NetUtils.h>
#include <Log.h>
#include <StringUtils.h>



AlpineCorba_impl::DtcpStackStatus::DtcpStackStatus (const CORBA::ORB_var &          orb,
                                                   const PortableServer::POA_var & poa,
                                                   AlpineCorba_impl *              implParent)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::DtcpStackStatus constructor invoked.");
#endif

    orb_        = orb;
    poa_        = poa;
    implParent_ = implParent;
}



AlpineCorba_impl::DtcpStackStatus::~DtcpStackStatus ()
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::DtcpStackStatus destructor invoked.");
#endif
}



void  
AlpineCorba_impl::DtcpStackStatus::getBufferStats (Alpine::t_DtcpStackBufferStats_out   stats,
                                                   CORBA::Environment &ACE_TRY_ENV)
     ExThrowSpec ((CORBA::SystemException,
                   Alpine::e_DtcpStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::DtcpStackStatus::getBufferStats invoked.");
#endif
}



