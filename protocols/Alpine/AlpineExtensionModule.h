/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <ConfigData.h>
#include <vector>


class AlpineQueryOptionData;
class AlpinePeerOptionData;
class AlpineProxyOptionData;


class AlpineExtensionModule
{
  public:

    AlpineExtensionModule () {};

    virtual ~AlpineExtensionModule () {};



    using t_OptionIdList = vector<ulong>;


    // Query Option Extensions
    //
    virtual bool  getQueryOptionExtensionList (t_OptionIdList &  optionIdList) = 0;

    virtual bool  createQueryOptionData (ulong                     optionId,
                                         AlpineQueryOptionData *&  optionData) = 0;


    // Peer Discovery Extensions
    //
    virtual bool  getPeerOptionExtensionList (t_OptionIdList &  optionIdList) = 0;

    virtual bool  createPeerOptionData (ulong                    optionId,
                                        AlpinePeerOptionData *&  optionData) = 0;


    // Proxy Extensions
    //
    virtual bool  getProxyOptionExtensionList (t_OptionIdList &  optionIdList) = 0;

    virtual bool  createProxyOptionData (ulong                     optionId,
                                         AlpineProxyOptionData *&  optionData) = 0;



};

