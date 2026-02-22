/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <ReadWriteSem.h>
#include <OptHash.h>
#include <vector>


class AlpineQueryOptionData;
class AlpinePeerOptionData;
class AlpineProxyOptionData;
class AlpineExtensionModule;


class AlpineExtensionIndex
{
  public:


    static bool  initialize ();


    // Obtain various optional extension classes for use in demarshalling packet data, etc.
    //
    static bool  getQueryOptionExt (ulong                     optionId,
                                    AlpineQueryOptionData *&  optionData);

    static bool  getPeerOptionExt (ulong                    optionId,
                                   AlpinePeerOptionData *&  optionData);

    static bool  getProxyOptionExt (ulong                     optionId,
                                    AlpineProxyOptionData *&  optionData);

    static bool  exists (ulong  optionId);



    // Modules providing support for various extensions must register using these methods
    //
    static bool  registerExtensionModule (AlpineExtensionModule *  module);

    static bool  removeExtensionModule (AlpineExtensionModule *  module);



    // Private types 
    //
    using t_OptionIdList = vector<ulong>;

    struct t_ModuleOptionIdInfo {
        t_OptionIdList  queryIdList;
        t_OptionIdList  peerIdList;
        t_OptionIdList  proxyIdList;
    };

    using t_OptionIdIndex = std::unordered_map<ulong,
                     AlpineExtensionModule *,
                     OptHash<ulong>,
                     equal_to<ulong> >;

    using t_OptionIdIndexPair = std::pair<ulong, AlpineExtensionModule *>;


    using t_ModuleIndex = std::unordered_map<void *, // AlpineExtensionModule *
                     t_ModuleOptionIdInfo *,
                     OptHash<void *>,
                     equal_to<void *> >;

    using t_ModuleIndexPair = std::pair<void *, t_ModuleOptionIdInfo *>;



  private:

    static bool               initialized_s;
    static t_OptionIdIndex *  queryOptionIdIndex_s;
    static t_OptionIdIndex *  peerOptionIdIndex_s;
    static t_OptionIdIndex *  proxyOptionIdIndex_s;
    static t_ModuleIndex *    moduleIndex_s;
    static ReadWriteSem       dataLock_s;

};

