/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AlpineCorbaClient.h>
#include <ReadLock.h>
#include <WriteLock.h>
#include <Log.h>
#include <StringUtils.h>



void  
AlpineCorbaClient::AlpineStackMgmt::setTotalTransferLimit (uint  transferLimit,
                                                           CORBA::Environment &ACE_TRY_ENV)
     ExThrowSpec ((CORBA::SystemException,
                   Alpine::e_InvalidValue,
                   Alpine::e_AlpineStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaClient::AlpineStackMgmt::setTotalTransferLimit invoked.");
#endif

    ReadLock  lock(AlpineCorbaClient::dataLock_s);

    if (AlpineCorbaClient::!initialized_s) {
        Log::Error ("AlpineStackMgmt method invoked before AlpineCorbaClient initialization!");
        ExThrow (CORBA::BAD_INV_ORDER ());
        return;
    }

    AlpineCorbaClient::alpineStackMgmtRef_s->setTotalTransferLimit (transferLimit, ExTryEnv);
}



uint  
AlpineCorbaClient::AlpineStackMgmt::getTotalTransferLimit (CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_AlpineStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaClient::AlpineStackMgmt::getTotalTransferLimit invoked.");
#endif


    ReadLock  lock(AlpineCorbaClient::dataLock_s);

    if (AlpineCorbaClient::!initialized_s) {
        Log::Error ("AlpineStackMgmt method invoked before AlpineCorbaClient initialization!");
        ExThrow (CORBA::BAD_INV_ORDER ());
        return false;
    }

    uint retVal;
    retVal = AlpineCorbaClient::alpineStackMgmtRef_s->getTotalTransferLimit (ExTryEnv);


    return retVal;
}



void  
AlpineCorbaClient::AlpineStackMgmt::setPeerTransferLimit (uint  transferLimit,
                                           CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_InvalidValue,
                  Alpine::e_AlpineStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaClient::AlpineStackMgmt::setPeerTransferLimit invoked.");
#endif

    ReadLock  lock(AlpineCorbaClient::dataLock_s);

    if (AlpineCorbaClient::!initialized_s) {
        Log::Error ("AlpineStackMgmt method invoked before AlpineCorbaClient initialization!");
        ExThrow (CORBA::BAD_INV_ORDER ());
        return;
    }

    AlpineCorbaClient::alpineStackMgmtRef_s->setPeerTransferLimit (transferLimit, ExTryEnv);
}



uint  
AlpineCorbaClient::AlpineStackMgmt::getPeerTransferLimit (CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_AlpineStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaClient::AlpineStackMgmt::getPeerTransferLimit invoked.");
#endif

    ReadLock  lock(AlpineCorbaClient::dataLock_s);

    if (AlpineCorbaClient::!initialized_s) {
        Log::Error ("AlpineStackMgmt method invoked before AlpineCorbaClient initialization!");
        ExThrow (CORBA::BAD_INV_ORDER ());
        return false;
    }

    uint retVal;
    retVal = AlpineCorbaClient::alpineStackMgmtRef_s->getPeerTransferLimit (ExTryEnv);


    return retVal;
}





