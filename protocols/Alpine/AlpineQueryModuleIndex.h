/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <AlpineQueryModule.h>
#include <Common.h>
#include <OptHash.h>
#include <ReadWriteSem.h>
#include <vector>


class AlpineQueryModuleIndex
{
  public:
    static bool initialize();

    static bool registerQueryModule(AlpineQueryModule * module);

    static bool removeQueryModule(AlpineQueryModule * module);

    // The processed query is dispatched to all modules if no extensions are given,
    // otherwise dispatch only to respective module which supports the extension.
    //
    static bool processQuery(AlpineQueryRequest & queryRequest, AlpineQueryModule::t_ResourceDescList & queryResults);


    // Private types
    //
    using t_OptionIdList = vector<ulong>;

    using t_OptionIdIndex = std::unordered_map<ulong, AlpineQueryModule *, OptHash<ulong>, equal_to<ulong>>;

    using t_OptionIdIndexPair = std::pair<ulong, AlpineQueryModule *>;


    using t_ModuleIndex = std::unordered_map<void *,  // AlpineQueryModule *
                                             t_OptionIdList *,
                                             OptHash<void *>,
                                             equal_to<void *>>;

    using t_ModuleIndexPair = std::pair<void *, t_OptionIdList *>;


  private:
    static bool initialized_s;
    static t_OptionIdIndex * optionIdIndex_s;
    static t_ModuleIndex * moduleIndex_s;
    static ReadWriteSem dataLock_s;
};
