/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AlpineModuleMgmtIntf.h>
#include <Log.h>
#include <StringUtils.h>



bool  
AlpineModuleMgmtIntf::registerModule (const string &  libraryPath,
                                      const string &  boostrapSymbol,
                                      ulong &         moduleId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineModuleMgmtIntf::registerModule invoked.");
#endif


    return true;
}



bool  
AlpineModuleMgmtIntf::unregisterModule (ulong  moduleId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineModuleMgmtIntf::unregisterModule invoked.");
#endif


    return true;
}



bool  
AlpineModuleMgmtIntf::setConfiguration (ulong         moduleId,
                                        ConfigData &  configData)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineModuleMgmtIntf::setConfiguration invoked.");
#endif


    return true;
}



bool  
AlpineModuleMgmtIntf::getConfiguration (ulong         moduleId,
                                        ConfigData &  configData)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineModuleMgmtIntf::getConfiguration invoked.");
#endif


    return true;
}



bool  
AlpineModuleMgmtIntf::getModuleInfo (ulong           moduleId,
                                     t_ModuleInfo &  moduleInfo)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineModuleMgmtIntf::getModuleInfo invoked.");
#endif


    return true;
}



bool  
AlpineModuleMgmtIntf::loadModule (ulong  moduleId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineModuleMgmtIntf::loadModule invoked.");
#endif


    return true;
}



bool  
AlpineModuleMgmtIntf::unloadModule (ulong  moduleId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineModuleMgmtIntf::unloadModule invoked.");
#endif


    return true;
}



bool  
AlpineModuleMgmtIntf::listActiveModules (t_IdList &  idList)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineModuleMgmtIntf::listActiveModules invoked.");
#endif


    return true;
}



bool  
AlpineModuleMgmtIntf::listAllModules (t_IdList &  idList)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineModuleMgmtIntf::listAllModules invoked.");
#endif


    return true;
}



