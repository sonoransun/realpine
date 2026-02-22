/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AlpineCorbaClient.h>
#include <ReadLock.h>
#include <WriteLock.h>
#include <Log.h>
#include <StringUtils.h>



void  
AlpineCorbaClient::AlpineModuleMgmt::registerModule (const char *   libraryPath,
                                                     const char *   boostrapSymbol,
                                                     uint &         moduleId,
                                                     CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_ModuleAlreadyExists,
                  Alpine::e_InvalidLibraryPath,
                  Alpine::e_InvalidLibraryPermissions,
                  Alpine::e_ModuleError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaClient::AlpineModuleMgmt::registerModule invoked.");
#endif

    ReadLock  lock(AlpineCorbaClient::dataLock_s);

    if (AlpineCorbaClient::!initialized_s) {
        Log::Error ("AlpineModuleMgmt method invoked before AlpineCorbaClient initialization!");
        CORBA::BAD_INV_ORDER  ex;
        ExThrow (ex);
        return;
    }

    AlpineCorbaClient::alpineModuleMgmtRef_s->registerModule (libraryPath,
                                                              boostrapSymbol,
                                                              moduleId,
                                                              ExTryEnv);
}



void  
AlpineCorbaClient::AlpineModuleMgmt::unregisterModule (uint  moduleId,
                                                       CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_InvalidModuleId,
                  Alpine::e_ModuleActive,
                  Alpine::e_ModuleError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaClient::AlpineModuleMgmt::unregisterModule invoked.");
#endif

    ReadLock  lock(AlpineCorbaClient::dataLock_s);

    if (AlpineCorbaClient::!initialized_s) {
        Log::Error ("AlpineModuleMgmt method invoked before AlpineCorbaClient initialization!");
        CORBA::BAD_INV_ORDER  ex;
        ExThrow (ex);
        return;
    }

    AlpineCorbaClient::alpineModuleMgmtRef_s->unregisterModule (moduleId, ExTryEnv);
}



void  
AlpineCorbaClient::AlpineModuleMgmt::setConfiguration (uint                          moduleId,
                                                       const Alpine::t_ConfigData &  configData,
                                                       CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_InvalidModuleId,
                  Alpine::e_ModuleError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaClient::AlpineModuleMgmt::setConfiguration invoked.");
#endif

    ReadLock  lock(AlpineCorbaClient::dataLock_s);

    if (AlpineCorbaClient::!initialized_s) {
        Log::Error ("AlpineModuleMgmt method invoked before AlpineCorbaClient initialization!");
        CORBA::BAD_INV_ORDER  ex;
        ExThrow (ex);
        return;
    }

    AlpineCorbaClient::alpineModuleMgmtRef_s->setConfiguration (moduleId,
                                                                configData,
                                                                ExTryEnv);
}



void  
AlpineCorbaClient::AlpineModuleMgmt::getConfiguration (uint                       moduleId,
                                                       Alpine::t_ConfigData_out   configData,
                                                       CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_InvalidModuleId,
                  Alpine::e_ModuleError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaClient::AlpineModuleMgmt::getConfiguration invoked.");
#endif

    ReadLock  lock(AlpineCorbaClient::dataLock_s);

    if (AlpineCorbaClient::!initialized_s) {
        Log::Error ("AlpineModuleMgmt method invoked before AlpineCorbaClient initialization!");
        CORBA::BAD_INV_ORDER  ex;
        ExThrow (ex);
        return;
    }

    AlpineCorbaClient::alpineModuleMgmtRef_s->getConfiguration (moduleId,
                                                                configData,
                                                                ExTryEnv);
}



void  
AlpineCorbaClient::AlpineModuleMgmt::getModuleInfo (uint                            moduleId,
                                                    Alpine::t_AlpineModuleInfo_out  moduleInfo,
                                                    CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_InvalidModuleId,
                  Alpine::e_ModuleError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaClient::AlpineModuleMgmt::getModuleInfo invoked.");
#endif

    ReadLock  lock(AlpineCorbaClient::dataLock_s);

    if (AlpineCorbaClient::!initialized_s) {
        Log::Error ("AlpineModuleMgmt method invoked before AlpineCorbaClient initialization!");
        CORBA::BAD_INV_ORDER  ex;
        ExThrow (ex);
        return;
    }

    AlpineCorbaClient::alpineModuleMgmtRef_s->getModuleInfo (moduleId,
                                                             moduleInfo,
                                                             ExTryEnv);
}



void  
AlpineCorbaClient::AlpineModuleMgmt::loadModule (uint  moduleId,
                                                 CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_InvalidModuleId,
                  Alpine::e_ModuleActive,
                  Alpine::e_ModuleError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaClient::AlpineModuleMgmt::loadModule invoked.");
#endif

    ReadLock  lock(AlpineCorbaClient::dataLock_s);

    if (AlpineCorbaClient::!initialized_s) {
        Log::Error ("AlpineModuleMgmt method invoked before AlpineCorbaClient initialization!");
        CORBA::BAD_INV_ORDER  ex;
        ExThrow (ex);
        return;
    }

    AlpineCorbaClient::alpineModuleMgmtRef_s->loadModule (moduleId, ExTryEnv);
}



void  
AlpineCorbaClient::AlpineModuleMgmt::unloadModule (uint  moduleId,
                                                   CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_InvalidModuleId,
                  Alpine::e_ModuleNotActive,
                  Alpine::e_ModuleError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaClient::AlpineModuleMgmt::unloadModule invoked.");
#endif

    ReadLock  lock(AlpineCorbaClient::dataLock_s);

    if (AlpineCorbaClient::!initialized_s) {
        Log::Error ("AlpineModuleMgmt method invoked before AlpineCorbaClient initialization!");
        CORBA::BAD_INV_ORDER  ex;
        ExThrow (ex);
        return;
    }

    AlpineCorbaClient::alpineModuleMgmtRef_s->unloadModule (moduleId, ExTryEnv);
}



void  
AlpineCorbaClient::AlpineModuleMgmt::listActiveModules (Alpine::t_ModuleIdList_out  idList,
                                                        CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_ModuleError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaClient::AlpineModuleMgmt::listActiveModules invoked.");
#endif

    ReadLock  lock(AlpineCorbaClient::dataLock_s);

    if (AlpineCorbaClient::!initialized_s) {
        Log::Error ("AlpineModuleMgmt method invoked before AlpineCorbaClient initialization!");
        CORBA::BAD_INV_ORDER  ex;
        ExThrow (ex);
        return;
    }

    AlpineCorbaClient::alpineModuleMgmtRef_s->listActiveModules (idList, ExTryEnv);
}



void  
AlpineCorbaClient::AlpineModuleMgmt::listAllModules (Alpine::t_ModuleIdList_out  idList,
                                                     CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_ModuleError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaClient::AlpineModuleMgmt::listAllModules invoked.");
#endif

    ReadLock  lock(AlpineCorbaClient::dataLock_s);

    if (AlpineCorbaClient::!initialized_s) {
        Log::Error ("AlpineModuleMgmt method invoked before AlpineCorbaClient initialization!");
        CORBA::BAD_INV_ORDER  ex;
        ExThrow (ex);
        return;
    }

    AlpineCorbaClient::alpineModuleMgmtRef_s->listAllModules (idList, ExTryEnv);
}



