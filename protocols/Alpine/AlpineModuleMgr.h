/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <ConfigData.h>
#include <OptHash.h>
#include <Platform.h>
#include <ReadWriteSem.h>
#include <vector>


class AlpineModuleInterface;
class AlpineQueryModule;
class AlpineExtensionModule;
class AlpineTransportClientModule;
class AlpineTransportServerModule;
class DynamicLoader;


class AlpineModuleMgr
{
  public:
    // Public Types
    //
    using t_IdList = vector<ulong>;

    struct t_ModuleInfo
    {
        ulong moduleId;
        string libraryPath;
        string bootstrapSymbol;
        string moduleName;
        string description;
        string version;
        ulong activeTime;  // seconds
    };


    // Public Operations
    //
    static bool initialize();

    static bool registerModule(const string & libraryPath, const string & bootstrapSymbol, ulong & moduleId);

    static bool exists(ulong moduleId);

    static bool exists(const string & libraryPath);

    static bool setConfiguration(ulong moduleId, ConfigData & configData);

    static bool getConfiguration(ulong moduleId, ConfigData & configData);

    static bool loadModule(ulong moduleId);

    static bool isActive(ulong moduleId);

    static bool unloadModule(ulong moduleId);

    static bool unregisterModule(ulong moduleId);

    static bool listAllModules(t_IdList & idList);

    static bool listActiveModules(t_IdList & idList);

    // Some data only available for active modules
    //
    static bool getModuleInfo(ulong moduleId, t_ModuleInfo & moduleInfo);


    // Internal types
    //
    struct t_ModuleRecord
    {
        ulong id;
        string libraryPath;
        string bootstrapSymbol;
        bool isActive;
        ConfigData * configuration;
        DynamicLoader * loader;
        AlpineModuleInterface * moduleInterface;
        string moduleName;
        string description;
        string version;
        AlpineQueryModule * queryModule;
        AlpineExtensionModule * extensionModule;
        AlpineTransportClientModule * transportClientModule;
        AlpineTransportServerModule * transportServerModule;
        struct timeval startTime;
    };

    using t_ModuleIndex = std::unordered_map<ulong, t_ModuleRecord *, OptHash<ulong>, equal_to<ulong>>;

    using t_ModuleIndexPair = std::pair<ulong, t_ModuleRecord *>;


    using t_ModulePathIndex = std::unordered_map<string, t_ModuleRecord *, OptHash<string>, equal_to<string>>;

    using t_ModulePathIndexPair = std::pair<string, t_ModuleRecord *>;


  private:
    static ulong currModuleId_s;
    static t_ModuleIndex * moduleIndex_s;
    static t_ModulePathIndex * modulePathIndex_s;
    static ReadWriteSem dataLock_s;
};
