/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <AlpineCorbaClient.h>
#include <CorbaUtils.h>
#include <ConfigData.h>
#include <string>
#include <vector>


class AlpineModuleMgmtIntf
{
  public:


    // Public types
    //
    struct t_ModuleInfo {
        ulong    moduleId;
        string   moduleName;
        string   description;
        string   version;
        string   libraryPath;
        string   bootstrapSymbol;
        ulong    activeTime;
    };

    using t_IdList = vector<ulong>;



    // Supported interface operations
    //
    static bool  registerModule (const string &  libraryPath,
                                 const string &  boostrapSymbol,
                                 ulong &         moduleId);

    static bool  unregisterModule (ulong  moduleId);

    static bool  setConfiguration (ulong         moduleId,
                                   ConfigData &  configData);

    static bool  getConfiguration (ulong         moduleId,
                                   ConfigData &  configData);

    static bool  getModuleInfo (ulong           moduleId,
                                t_ModuleInfo &  moduleInfo);

    static bool  loadModule (ulong  moduleId);

    static bool  unloadModule (ulong  moduleId);

    static bool  listActiveModules (t_IdList &  idList);

    static bool  listAllModules (t_IdList &  idList);



  private:

};

