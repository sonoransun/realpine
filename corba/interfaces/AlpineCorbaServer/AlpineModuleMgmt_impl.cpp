/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AlpineCorba_impl.h>
#include <NetUtils.h>
#include <Log.h>
#include <StringUtils.h>



AlpineCorba_impl::AlpineModuleMgmt::AlpineModuleMgmt (const CORBA::ORB_var &          orb,
                                                      const PortableServer::POA_var & poa,
                                                      AlpineCorba_impl *              implParent)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::AlpineModuleMgmt constructor invoked.");
#endif

    orb_        = orb;
    poa_        = poa;
    implParent_ = implParent;
}



AlpineCorba_impl::AlpineModuleMgmt::~AlpineModuleMgmt ()
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::AlpineModuleMgmt destructor invoked.");
#endif
}



void  
AlpineCorba_impl::AlpineModuleMgmt::registerModule (const char *   libraryPath,
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
    Log::Debug ("AlpineCorba_impl::AlpineModuleMgmt::registerModule invoked.");
#endif
}



void  
AlpineCorba_impl::AlpineModuleMgmt::unregisterModule (uint  moduleId,
                                                      CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_InvalidModuleId,
                  Alpine::e_ModuleActive,
                  Alpine::e_ModuleError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::AlpineModuleMgmt::unregisterModule invoked.");
#endif
}



void  
AlpineCorba_impl::AlpineModuleMgmt::setConfiguration (uint                          moduleId,
                                                      const Alpine::t_ConfigData &  configData,
                                                      CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_InvalidModuleId,
                  Alpine::e_ModuleError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::AlpineModuleMgmt::setConfiguration invoked.");
#endif
}



void  
AlpineCorba_impl::AlpineModuleMgmt::getConfiguration (uint                       moduleId,
                                                      Alpine::t_ConfigData_out   configData,
                                                      CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_InvalidModuleId,
                  Alpine::e_ModuleError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::AlpineModuleMgmt::getConfiguration invoked.");
#endif

    configData = new Alpine::t_ConfigData;
}



void  
AlpineCorba_impl::AlpineModuleMgmt::getModuleInfo (uint                            moduleId,
                                                   Alpine::t_AlpineModuleInfo_out  moduleInfo,
                                                   CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_InvalidModuleId,
                  Alpine::e_ModuleError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::AlpineModuleMgmt::getModuleInfo invoked.");
#endif

    moduleInfo = new Alpine::t_AlpineModuleInfo;
}



void  
AlpineCorba_impl::AlpineModuleMgmt::loadModule (uint  moduleId,
                                                CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_InvalidModuleId,
                  Alpine::e_ModuleActive,
                  Alpine::e_ModuleError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::AlpineModuleMgmt::loadModule invoked.");
#endif
}



void  
AlpineCorba_impl::AlpineModuleMgmt::unloadModule (uint  moduleId,
                                                  CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_InvalidModuleId,
                  Alpine::e_ModuleNotActive,
                  Alpine::e_ModuleError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::AlpineModuleMgmt::unloadModule invoked.");
#endif
}



void  
AlpineCorba_impl::AlpineModuleMgmt::listActiveModules (Alpine::t_ModuleIdList_out  idList,
                                                       CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_ModuleError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::AlpineModuleMgmt::listActiveModules invoked.");
#endif

    idList = new Alpine::t_ModuleIdList;
}



void  
AlpineCorba_impl::AlpineModuleMgmt::listAllModules (Alpine::t_ModuleIdList_out  idList,
                                                    CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_ModuleError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::AlpineModuleMgmt::listAllModules invoked.");
#endif

    idList = new Alpine::t_ModuleIdList;
}



