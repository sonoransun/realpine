/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AlpineCorbaClient.h>
#include <ReadLock.h>
#include <WriteLock.h>
#include <Log.h>
#include <StringUtils.h>



void  
AlpineCorbaClient::DtcpStackMgmt::natDiscovery (CORBA::Boolean  isRequired,
                                                CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_DtcpStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaClient::DtcpStackMgmt::natDiscovery invoked.");
#endif

    ReadLock  lock(AlpineCorbaClient::dataLock_s);

    if (AlpineCorbaClient::!initialized_s) {
        Log::Error ("DtcpStackMgmt method invoked before AlpineCorbaClient initialization!");
        CORBA::BAD_INV_ORDER  ex;
        ExThrow (ex);
        return;
    }

    AlpineCorbaClient::dtcpStackMgmtRef_s->natDiscovery (isRequired, ExTryEnv);
}



CORBA::Boolean 
AlpineCorbaClient::DtcpStackMgmt::natDiscoveryRequired (CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_DtcpStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaClient::DtcpStackMgmt::natDiscoveryRequired invoked.");
#endif

    ReadLock  lock(AlpineCorbaClient::dataLock_s);

    if (AlpineCorbaClient::!initialized_s) {
        Log::Error ("DtcpStackMgmt method invoked before AlpineCorbaClient initialization!");
        CORBA::BAD_INV_ORDER  ex;
        ExThrow (ex);
        return;
    }

    CORBA::Boolean  retVal;
    retVal = AlpineCorbaClient::dtcpStackMgmtRef_s->natDiscoveryRequired (ExTryEnv);



    return retVal;
}



void  
AlpineCorbaClient::DtcpStackMgmt::setDataSendingLimit (uint  bpsLimit,
                                                       CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_DtcpStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaClient::DtcpStackMgmt::setDataSendingLimit invoked.");
#endif

    ReadLock  lock(AlpineCorbaClient::dataLock_s);

    if (AlpineCorbaClient::!initialized_s) {
        Log::Error ("DtcpStackMgmt method invoked before AlpineCorbaClient initialization!");
        CORBA::BAD_INV_ORDER  ex;
        ExThrow (ex);
        return;
    }

    AlpineCorbaClient::dtcpStackMgmtRef_s->setDataSendingLimit (bpsLimit, ExTryEnv);
}



uint  
AlpineCorbaClient::DtcpStackMgmt::getDataSendingLimit (CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_DtcpStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaClient::DtcpStackMgmt::getDataSendingLimit invoked.");
#endif

    ReadLock  lock(AlpineCorbaClient::dataLock_s);

    if (AlpineCorbaClient::!initialized_s) {
        Log::Error ("DtcpStackMgmt method invoked before AlpineCorbaClient initialization!");
        CORBA::BAD_INV_ORDER  ex;
        ExThrow (ex);
        return false;
    }

    uint  retVal;
    retVal = AlpineCorbaClient::dtcpStackMgmtRef_s->getDataSendingLimit (ExTryEnv);


    return retVal;
}



void  
AlpineCorbaClient::DtcpStackMgmt::setStackThreadLimit (uint  threadLimit,
                                                       CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_DtcpStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaClient::DtcpStackMgmt::setStackThreadLimit invoked.");
#endif

    ReadLock  lock(AlpineCorbaClient::dataLock_s);

    if (AlpineCorbaClient::!initialized_s) {
        Log::Error ("DtcpStackMgmt method invoked before AlpineCorbaClient initialization!");
        CORBA::BAD_INV_ORDER  ex;
        ExThrow (ex);
        return;
    }

    AlpineCorbaClient::dtcpStackMgmtRef_s->setStackThreadLimit (threadLimit, ExTryEnv);
}



uint  
AlpineCorbaClient::DtcpStackMgmt::getStackThreadLimit (CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_DtcpStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaClient::DtcpStackMgmt::getStackThreadLimit invoked.");
#endif

    ReadLock  lock(AlpineCorbaClient::dataLock_s);

    if (AlpineCorbaClient::!initialized_s) {
        Log::Error ("DtcpStackMgmt method invoked before AlpineCorbaClient initialization!");
        CORBA::BAD_INV_ORDER  ex;
        ExThrow (ex);
        return false;
    }

    uint  retVal;
    retVal = AlpineCorbaClient::dtcpStackMgmtRef_s->getStackThreadLimit (ExTryEnv);


    return retVal;
}



void  
AlpineCorbaClient::DtcpStackMgmt::setReceiveBufferLimit (uint  recvBufferLimit,
                                                         CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_InvalidValue,
                  Alpine::e_DtcpStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaClient::DtcpStackMgmt::setReceiveBufferLimit invoked.");
#endif

    ReadLock  lock(AlpineCorbaClient::dataLock_s);

    if (AlpineCorbaClient::!initialized_s) {
        Log::Error ("DtcpStackMgmt method invoked before AlpineCorbaClient initialization!");
        CORBA::BAD_INV_ORDER  ex;
        ExThrow (ex);
        return;
    }

    AlpineCorbaClient::dtcpStackMgmtRef_s->setReceiveBufferLimit (recvBufferLimit, ExTryEnv);
}



uint  
AlpineCorbaClient::DtcpStackMgmt::getReceiveBufferLimit (CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_DtcpStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaClient::DtcpStackMgmt::getReceiveBufferLimit invoked.");
#endif

    ReadLock  lock(AlpineCorbaClient::dataLock_s);

    if (AlpineCorbaClient::!initialized_s) {
        Log::Error ("DtcpStackMgmt method invoked before AlpineCorbaClient initialization!");
        CORBA::BAD_INV_ORDER  ex;
        ExThrow (ex);
        return false;
    }

    uint  retVal;
    retVal = AlpineCorbaClient::dtcpStackMgmtRef_s->getReceiveBufferLimit (ExTryEnv);


    return retVal;
}



void  
AlpineCorbaClient::DtcpStackMgmt::setSendBufferLimit (uint  sendBufferLimit,
                                                      CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_InvalidValue,
                  Alpine::e_DtcpStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaClient::DtcpStackMgmt::setSendBufferLimit invoked.");
#endif

    ReadLock  lock(AlpineCorbaClient::dataLock_s);

    if (AlpineCorbaClient::!initialized_s) {
        Log::Error ("DtcpStackMgmt method invoked before AlpineCorbaClient initialization!");
        CORBA::BAD_INV_ORDER  ex;
        ExThrow (ex);
        return;
    }

    AlpineCorbaClient::dtcpStackMgmtRef_s->setSendBufferLimit (sendBufferLimit, ExTryEnv);
}



uint  
AlpineCorbaClient::DtcpStackMgmt::getSendBufferLimit (CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_DtcpStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaClient::DtcpStackMgmt::getSendBufferLimit invoked.");
#endif

    ReadLock  lock(AlpineCorbaClient::dataLock_s);

    if (AlpineCorbaClient::!initialized_s) {
        Log::Error ("DtcpStackMgmt method invoked before AlpineCorbaClient initialization!");
        CORBA::BAD_INV_ORDER  ex;
        ExThrow (ex);
        return false;
    }

    uint  retVal;
    retVal = AlpineCorbaClient::dtcpStackMgmtRef_s->getSendBufferLimit (ExTryEnv);


    return retVal;
}




