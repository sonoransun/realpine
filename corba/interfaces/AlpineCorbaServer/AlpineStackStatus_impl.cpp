/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AlpineCorba_impl.h>
#include <NetUtils.h>
#include <Log.h>
#include <StringUtils.h>



AlpineCorba_impl::AlpineStackStatus::AlpineStackStatus (const CORBA::ORB_var &          orb,
                                                        const PortableServer::POA_var & poa,
                                                        AlpineCorba_impl *              implParent)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::AlpineStackStatus constructor invoked.");
#endif

    orb_        = orb;
    poa_        = poa;
    implParent_ = implParent;
}



AlpineCorba_impl::AlpineStackStatus::~AlpineStackStatus ()
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::AlpineStackStatus destructor invoked.");
#endif
}



void  
AlpineCorba_impl::AlpineStackStatus::getTransferStats (Alpine::t_AlpineStackTransferStats_out  stats,
                                                       CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_AlpineStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::AlpineStackStatus::getTransferStats invoked.");
#endif
}



