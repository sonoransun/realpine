/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <AlpineQueryRequest.h>
#include <AlpineResourceDesc.h>
#include <Common.h>
#include <vector>


class AlpineQueryModule
{
  public:
    AlpineQueryModule(){};
    virtual ~AlpineQueryModule(){};


    using t_OptionIdList = vector<ulong>;

    using t_ResourceDescList = vector<AlpineResourceDesc>;


    // Control
    //
    virtual bool start() = 0;

    virtual bool isActive() = 0;

    virtual bool stop() = 0;


    // Search Facility
    //
    virtual bool getQueryOptionExtensionList(t_OptionIdList & optionIdList) = 0;

    virtual bool processQuery(AlpineQueryRequest & queryRequest, t_ResourceDescList & queryResults) = 0;
};
