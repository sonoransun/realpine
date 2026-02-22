/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AlpineModuleMgr.h>
#include <AlpineModuleInterface.h>
#include <AlpineQueryModule.h>
#include <AlpineQueryModuleIndex.h>
#include <AlpineExtensionModule.h>
#include <AlpineExtensionIndex.h>
#include <AlpineTransportClientModule.h>
#include <AlpineTransportServerModule.h>
#include <ReadLock.h>
#include <WriteLock.h>
#include <Log.h>
#include <StringUtils.h>
#include <FileUtils.h>
#include <DynamicLoader.h>



ulong                                   AlpineModuleMgr::currModuleId_s = 0;
AlpineModuleMgr::t_ModuleIndex *        AlpineModuleMgr::moduleIndex_s  = nullptr;
AlpineModuleMgr::t_ModulePathIndex *    AlpineModuleMgr::modulePathIndex_s = nullptr;
ReadWriteSem                            AlpineModuleMgr::dataLock_s;



bool  
AlpineModuleMgr::initialize ()
{
#ifdef _VERBOSE
    Log::Debug ("AlpineModuleMgr::initialize invoked.");
#endif

    WriteLock  lock(dataLock_s);

    if (moduleIndex_s) {
        Log::Error ("Attempt to reinitialize AlpineModuleMgr!");
        return false;
    }
    moduleIndex_s     = new t_ModuleIndex;
    modulePathIndex_s = new t_ModulePathIndex;

    return true;
}



bool  
AlpineModuleMgr::registerModule (const string &  libraryPath,
                                 const string &  bootstrapSymbol,
                                 ulong &         moduleId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineModuleMgr::registerModule invoked.");
#endif

    if (!moduleIndex_s) {
        Log::Error ("AlpineModuleMgr uninitialized in call to AlpineModuleMgr::registerModule!");
        return false;
    }
    // Verify that this module is not already defined...
    //
    {
        ReadLock  lock(dataLock_s);

        auto pathIter = modulePathIndex_s->find (libraryPath);

        if (pathIter != modulePathIndex_s->end ()) {
            Log::Error ("Module library: "s + libraryPath +
                        " already defined in call to AlpineModuleMgr::registerModule!");
            return false;
        }
    }


    // Verify that this module exists, and we have permission to use it
    //
    bool  status;
    FileUtils::t_FileInfo  fileInfo;

    status = FileUtils::exists (libraryPath);
    if (!status) {
        Log::Error ("Invalid library: "s + libraryPath + 
                    " passed in call to AlpineModuleMgr::registerModule!  Does not exist!");
        return false;
    }
    status = FileUtils::getFileInfo (libraryPath, fileInfo);
    if (!status) {
        Log::Error ("Invalid library: "s + libraryPath +
                    " passed in call to AlpineModuleMgr::registerModule!  Cannot get file info!");
        return false;
    }
    if ( !fileInfo.canRead () || !fileInfo.canExecute () ) {
        Log::Error ("Invalid permissions for library: "s + libraryPath +
                    " in call to AlpineModuleMgr::registerModule!");
        return false;
    }
    // Everything checks out, create record for this new module
    //
    t_ModuleRecord *  moduleRecord;
    moduleRecord = new t_ModuleRecord;

    WriteLock  lock(dataLock_s);

    moduleRecord->id                     = currModuleId_s++;
    moduleRecord->libraryPath            = libraryPath;
    moduleRecord->bootstrapSymbol        = bootstrapSymbol;
    moduleRecord->isActive               = false;
    moduleRecord->configuration          = new ConfigData;
    moduleRecord->loader                 = nullptr;
    moduleRecord->moduleInterface        = nullptr;
    moduleRecord->moduleName             = "";   ///
    moduleRecord->description            = "";   // Not set until activated for first time
    moduleRecord->version                = "";   ///
    moduleRecord->queryModule            = nullptr;
    moduleRecord->extensionModule        = nullptr;
    moduleRecord->transportClientModule  = nullptr;
    moduleRecord->transportServerModule  = nullptr;

    moduleIndex_s->emplace (moduleRecord->id, moduleRecord);
    modulePathIndex_s->emplace (libraryPath, moduleRecord);


    return true;
}



bool  
AlpineModuleMgr::exists (ulong  moduleId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineModuleMgr::exists (id) invoked.");
#endif

    if (!moduleIndex_s) {
        Log::Error ("AlpineModuleMgr uninitialized in call to AlpineModuleMgr::exists!");
        return false;
    }
    ReadLock  lock(dataLock_s);

    // Attempt to locate record for this module
    //

    return moduleIndex_s->find (moduleId) != moduleIndex_s->end ();
}



bool
AlpineModuleMgr::exists (const string &  libraryPath)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineModuleMgr::exists (path) invoked.");
#endif

    if (!moduleIndex_s) {
        Log::Error ("AlpineModuleMgr uninitialized in call to AlpineModuleMgr::exists!");
        return false;
    }
    ReadLock  lock(dataLock_s);

    // Attempt to locate record for this module
    //

    return modulePathIndex_s->find (libraryPath) != modulePathIndex_s->end ();
}



bool  
AlpineModuleMgr::setConfiguration (ulong         moduleId,
                                   ConfigData &  configData)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineModuleMgr::setConfiguration invoked.");
#endif

    if (!moduleIndex_s) {
        Log::Error ("AlpineModuleMgr uninitialized in call to AlpineModuleMgr::setConfiguration!");
        return false;
    }
    WriteLock  lock(dataLock_s);

    // Locate record for this module
    //
    t_ModuleRecord *  moduleRecord;

    auto iter = moduleIndex_s->find (moduleId);
    if (iter == moduleIndex_s->end ()) {
        Log::Error ("Invalid module ID: "s + std::to_string (moduleId) +
                    " passed in call to AlpineModuleMgr::setConfiguration!");
        return false;
    }
    moduleRecord = (*iter).second;

    if (moduleRecord->configuration) 
        delete moduleRecord->configuration;

    moduleRecord->configuration = new ConfigData (configData);


    // If module is active, pass to module interface for application immediately (if supported)
    //
    if (moduleRecord->isActive) {
#ifdef _VERBOSE
        Log::Debug ("Passing configuration data to module interface in call to "
                             "AlpineModuleMgr::setConfiguration.");
#endif

        bool status;
        status = moduleRecord->moduleInterface->setConfiguration (configData);

        if (!status) {
            Log::Error ("Set configuration failed for module ID: "s + std::to_string (moduleId) +
                        " in call to AlpineModuleMgr::setConfiguration!");

            return false;
        }
    }


    return true;
}



bool  
AlpineModuleMgr::getConfiguration (ulong         moduleId,
                                   ConfigData &  configData)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineModuleMgr::getConfiguration invoked.");
#endif

    if (!moduleIndex_s) {
        Log::Error ("AlpineModuleMgr uninitialized in call to AlpineModuleMgr::getConfiguration!");
        return false;
    }
    ReadLock  lock(dataLock_s);

    // Locate record for this module
    //
    t_ModuleRecord *  moduleRecord;

    auto iter = moduleIndex_s->find (moduleId);
    if (iter == moduleIndex_s->end ()) {
        Log::Error ("Invalid module ID: "s + std::to_string (moduleId) +
                    " passed in call to AlpineModuleMgr::getConfiguration!");
        return false;
    }
    moduleRecord = (*iter).second;

    if (!moduleRecord->configuration) {
        Log::Error ("No configuration available for module ID: "s + std::to_string (moduleId) +
                    " in call to AlpineModuleMgr::getConfiguration!");
        return false;
    }
    configData = *(moduleRecord->configuration);


    return true;
}



bool  
AlpineModuleMgr::loadModule (ulong  moduleId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineModuleMgr::loadModule invoked.");
#endif

    if (!moduleIndex_s) {
        Log::Error ("AlpineModuleMgr uninitialized in call to AlpineModuleMgr::loadModule!");
        return false;
    }
    WriteLock  lock(dataLock_s);

    // Locate record for this module
    //
    t_ModuleRecord *  moduleRecord;

    auto iter = moduleIndex_s->find (moduleId);
    if (iter == moduleIndex_s->end ()) {
        Log::Error ("Invalid module ID: "s + std::to_string (moduleId) +
                    " passed in call to AlpineModuleMgr::loadModule!");
        return false;
    }
    moduleRecord = (*iter).second;

    if (moduleRecord->isActive) {
        Log::Error ("Module ID: "s + std::to_string (moduleId) +
                    " is already active in call to AlpineModuleMgr::loadModule!");
        return false;
    }
    // Activation of a module consists of three main steps.
    // 1) Loading the dynamic library and getting the module interface reference
    // 2) Setting module configuration
    // 3) Activation of the module
    //

    // Verify that module library is still present and loadable
    //
    bool  status;
    string  libraryPath;
    FileUtils::t_FileInfo  fileInfo;

    libraryPath = moduleRecord->libraryPath;

    status = FileUtils::exists (libraryPath);
    if (!status) {
        Log::Error ("Module library: "s + libraryPath + 
                    " does NOT EXIST in call to AlpineModuleMgr::loadModule!");
        return false;
    }
    status = FileUtils::getFileInfo (libraryPath, fileInfo);
    if (!status) {
        Log::Error ("Module library: "s + libraryPath +
                    " is not accessible in call to AlpineModuleMgr::loadModule!");
        return false;
    }
    if ( !fileInfo.canRead () || !fileInfo.canExecute () ) {
        Log::Error ("Invalid permissions for module library: "s + libraryPath +
                    " in call to AlpineModuleMgr::loadModule!");
        return false;
    }
    // Load module and get interface reference
    //
    DynamicLoader *  loader;
    loader = new DynamicLoader;

    loader->setSymbolScope (DynamicLoader::t_SymbolScope::Global);
    loader->setBindingMethod (DynamicLoader::t_BindingMethod::Now);

    status = loader->load (libraryPath);
    if (!status) {
        Log::Error ("Loading module library: "s + libraryPath +
                    " failed in call to AlpineModuleMgr::loadModule!");

        delete loader;
        return false;
    }
    void *  funcHandle;
    string  symbolName (moduleRecord->bootstrapSymbol);

    status = loader->getFunctionHandle (symbolName, funcHandle);
    if (!status) {
        Log::Error ("Locating symbol name: "s + symbolName +
                    " in module library: "s + libraryPath +
                    " failed in call to AlpineModuleMgr::loadModule!");

        delete loader;
        return false;
    }
    AlpineModuleInterface * (*boostrapMethod)();
    boostrapMethod = (AlpineModuleInterface * (*)())funcHandle;
    moduleRecord->moduleInterface = (*boostrapMethod)();

    if (!moduleRecord->moduleInterface) {
        Log::Error ("Return from symbol name: "s + symbolName +
                    " in module library: "s + libraryPath +
                    " NULL in call to AlpineModuleMgr::loadModule!  Not a valid ModuleInterface!");
    
        delete loader;                 
        return false;                
    }
    moduleRecord->loader = loader;
    moduleRecord->isActive = true;
    gettimeofday (&(moduleRecord->startTime), nullptr);

    AlpineModuleInterface *  moduleIntf = moduleRecord->moduleInterface;


    // Set configuration and start module
    // At this point, we notify of any errors, but still consider the module active.
    //
    status = moduleIntf->setConfiguration (*(moduleRecord->configuration));
    if (!status) {
        Log::Error ("Setting configuration for module library: "s + libraryPath +
                    " failed in call to AlpineModuleMgr::loadModule!");
        return false;
    }

    status = moduleIntf->start ();
    if (!status) {
        Log::Error ("Start invocation for module library: "s + libraryPath +
                    " failed in call to AlpineModuleMgr::loadModule!");
        return false;
    }

    status = moduleIntf->getModuleInfo (moduleRecord->moduleName,
                                        moduleRecord->description,
                                        moduleRecord->version);
    if (!status) {
        Log::Error ("Error retrieving info for module library: "s + libraryPath +
                    " in call to AlpineModuleMgr::loadModule!");
        return false;
    }



    // Get supported interface references
    //

    // Query Interface
    //
    status = moduleIntf->getQueryInterface (moduleRecord->queryModule);
    if (!status) {
        Log::Info ("No query interface available for module library: "s + libraryPath +
                    " in call to AlpineModuleMgr::loadModule.");
        moduleRecord->queryModule = nullptr;
    }
    else {
        AlpineQueryModuleIndex::registerQueryModule (moduleRecord->queryModule);
    }

    // Protocol Extension Interface
    //
    status = moduleIntf->getExtensionInterface (moduleRecord->extensionModule);
    if (!status) {
        Log::Info ("No extension interface available for module library: "s + libraryPath +
                    " in call to AlpineModuleMgr::loadModule.");
        moduleRecord->extensionModule = nullptr;
    }
    else {
        AlpineExtensionIndex::registerExtensionModule (moduleRecord->extensionModule);
    }

    // Transport Client Interface
    //
    status = moduleIntf->getTransportClientInterface (moduleRecord->transportClientModule);
    if (!status) {
        Log::Info ("No transport client interface available for module library: "s + libraryPath +
                    " in call to AlpineModuleMgr::loadModule.");
        moduleRecord->queryModule = nullptr;
        return false;
    }

    // Transport Server Interface
    //
    status = moduleIntf->getTransportServerInterface (moduleRecord->transportServerModule);
    if (!status) {
        Log::Info ("No transport server interface available for module library: "s + libraryPath +
                    " in call to AlpineModuleMgr::loadModule.");
        moduleRecord->queryModule = nullptr;
        return false;
    }   


    return true;
}



bool  
AlpineModuleMgr::isActive (ulong  moduleId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineModuleMgr::isActive invoked.");
#endif

    if (!moduleIndex_s) {
        Log::Error ("AlpineModuleMgr uninitialized in call to AlpineModuleMgr::isActive!");
        return false;
    }
    ReadLock  lock(dataLock_s);

    // Locate record for this module
    //
    t_ModuleRecord *  moduleRecord;

    auto iter = moduleIndex_s->find (moduleId);
    if (iter == moduleIndex_s->end ()) {
        Log::Error ("Invalid module ID: "s + std::to_string (moduleId) +
                    " passed in call to AlpineModuleMgr::isActive!");
        return false;
    }
    moduleRecord = (*iter).second;

    return moduleRecord->isActive;
}



bool  
AlpineModuleMgr::unloadModule (ulong  moduleId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineModuleMgr::unloadModule invoked.");
#endif

    if (!moduleIndex_s) {
        Log::Error ("AlpineModuleMgr uninitialized in call to AlpineModuleMgr::unloadModule!");
        return false;
    }
    WriteLock  lock(dataLock_s);

    // Locate record for this module
    //
    t_ModuleRecord *  moduleRecord;

    auto iter = moduleIndex_s->find (moduleId);
    if (iter == moduleIndex_s->end ()) {
        Log::Error ("Invalid module ID: "s + std::to_string (moduleId) +
                    " passed in call to AlpineModuleMgr::unloadModule!");
        return false;
    }
    bool status;
    moduleRecord = (*iter).second;

    // If the module is not active, return error
    //
    if (!moduleRecord->isActive) {
        Log::Error ("Module ID: "s + std::to_string (moduleId) +
                    " is already stopped in call to AlpineModuleMgr::unloadModule!");
        return false;
    }
    // Remove all registered module interfaces, stop module and update module record
    //
    if (moduleRecord->queryModule) {
        AlpineQueryModuleIndex::removeQueryModule (moduleRecord->queryModule);
    }

    if (moduleRecord->extensionModule) {
        AlpineExtensionIndex::removeExtensionModule (moduleRecord->extensionModule);
    }
   
    status = moduleRecord->moduleInterface->stop ();
    if (!status) {
        Log::Error ("Attempt to stop module ID: "s + std::to_string (moduleId) +
                    " failed in call to AlpineModuleMgr::unloadModule!");
 
        // Continue on as if module had stopped.  Nothing further we can do at this point.
        return false;
    }


    // Unload dynamic library for this module via delete of DynamicLoader
    //
    delete moduleRecord->loader;
    moduleRecord->loader = nullptr;

    // Finished, update record state
    //
    moduleRecord->isActive               = false;
    moduleRecord->moduleInterface        = nullptr;
    moduleRecord->queryModule            = nullptr;
    moduleRecord->extensionModule        = nullptr;
    moduleRecord->transportClientModule  = nullptr;
    moduleRecord->transportServerModule  = nullptr;


    return true;
}



bool  
AlpineModuleMgr::unregisterModule (ulong  moduleId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineModuleMgr::unregisterModule invoked.");
#endif

    if (!moduleIndex_s) {
        Log::Error ("AlpineModuleMgr uninitialized in call to AlpineModuleMgr::unregisterModule!");
        return false;
    }
    if (!exists (moduleId)) {
        Log::Error ("Invalid module ID: "s + std::to_string (moduleId) +
                    " passed in call to AlpineModuleMgr::unregisterModule!");
        return false;
    }
    // Make sure module is not active before unregistering it
    //
    if (isActive (moduleId)) {
        unloadModule (moduleId);
    }

    WriteLock  lock(dataLock_s);

    // Locate record for this module
    //
    t_ModuleRecord *  moduleRecord;

    auto iter = moduleIndex_s->find (moduleId);
    if (iter == moduleIndex_s->end ()) {
        // This is a race condition.  Should not happen, but possible.
        //
        Log::Error ("Invalid module ID: "s + std::to_string (moduleId) +
                    " passed in call to AlpineModuleMgr::unregisterModule!");
        return false;
    }
    moduleRecord = (*iter).second;

    modulePathIndex_s->erase (moduleRecord->libraryPath);
    moduleIndex_s->erase (moduleId);

    if (moduleRecord->configuration) {
        delete moduleRecord->configuration;
    }

    delete moduleRecord;


    return true;
}



bool  
AlpineModuleMgr::listAllModules (t_IdList &  idList)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineModuleMgr::listAllModules invoked.");
#endif

    if (!moduleIndex_s) {
        Log::Error ("AlpineModuleMgr uninitialized in call to AlpineModuleMgr::listAllModules!");
        return false;
    }
    idList.clear ();

    ReadLock  lock(dataLock_s);


    for (const auto& item : *moduleIndex_s) {
        idList.push_back (item.first);
    }


    return true;
}



bool  
AlpineModuleMgr::listActiveModules (t_IdList &  idList)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineModuleMgr::listActiveModules invoked.");
#endif

    if (!moduleIndex_s) {
        Log::Error ("AlpineModuleMgr uninitialized in call to AlpineModuleMgr::listActiveModules!");
        return false;
    }
    idList.clear ();

    ReadLock  lock(dataLock_s);

    t_ModuleRecord *  currRecord;

    for (auto& [key, value] : *moduleIndex_s) {
        currRecord = value;

        if (currRecord->isActive) {
            idList.push_back (currRecord->id);
        }
    }


    return true;
}



bool  
AlpineModuleMgr::getModuleInfo (ulong           moduleId,
                                t_ModuleInfo &  moduleInfo)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineModuleMgr::getModuleInfo invoked.");
#endif

    if (!moduleIndex_s) {
        Log::Error ("AlpineModuleMgr uninitialized in call to AlpineModuleMgr::listActiveModules!");
        return false;
    }
    WriteLock  lock(dataLock_s);

    // Locate record for this module
    //
    t_ModuleRecord *  moduleRecord;

    auto iter = moduleIndex_s->find (moduleId);
    if (iter == moduleIndex_s->end ()) {
        Log::Error ("Invalid module ID: "s + std::to_string (moduleId) +
                    " passed in call to AlpineModuleMgr::getModuleInfo!");
        return false;
    }
    moduleRecord = (*iter).second;

    moduleInfo.moduleId         = moduleId;
    moduleInfo.libraryPath      = moduleRecord->libraryPath;
    moduleInfo.bootstrapSymbol  = moduleRecord->bootstrapSymbol;
    moduleInfo.moduleName       = moduleRecord->moduleName;
    moduleInfo.description      = moduleRecord->description;
    moduleInfo.version          = moduleRecord->version;

    if (moduleRecord->isActive) {
        moduleInfo.activeTime = time(0) - moduleRecord->startTime.tv_sec;
    }
    else {
        moduleInfo.activeTime = 0;
    }


    return true;
}



