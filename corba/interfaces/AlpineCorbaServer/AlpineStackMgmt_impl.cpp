/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AlpineCorba_impl.h>
#include <NetUtils.h>
#include <Log.h>
#include <StringUtils.h>



AlpineCorba_impl::AlpineStackMgmt::AlpineStackMgmt (const CORBA::ORB_var &          orb,
                                                    const PortableServer::POA_var & poa,
                                                    AlpineCorba_impl *              implParent)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::AlpineStackMgmt constructor invoked.");
#endif

}



AlpineCorba_impl::AlpineStackMgmt::~AlpineStackMgmt ()
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::AlpineStackMgmt destructor invoked.");
#endif

}



void  
AlpineCorba_impl::AlpineStackMgmt::setTotalTransferLimit (uint  transferLimit,
                                                          CORBA::Environment &ACE_TRY_ENV)
     ExThrowSpec ((CORBA::SystemException,
                   Alpine::e_InvalidValue,
                   Alpine::e_AlpineStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::AlpineStackMgmt::setTotalTransferLimit invoked.");
#endif

}



uint  
AlpineCorba_impl::AlpineStackMgmt::getTotalTransferLimit (CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_AlpineStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::AlpineStackMgmt::getTotalTransferLimit invoked.");
#endif

    uint  retVal = 0;

    return retVal;
}



void  
AlpineCorba_impl::AlpineStackMgmt::setPeerTransferLimit (uint  transferLimit,
                                                         CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_InvalidValue,
                  Alpine::e_AlpineStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::AlpineStackMgmt::setPeerTransferLimit invoked.");
#endif

}



uint  
AlpineCorba_impl::AlpineStackMgmt::getPeerTransferLimit (CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_AlpineStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::AlpineStackMgmt::getPeerTransferLimit invoked.");
#endif

    uint  retVal = 0;

    return retVal;
}





