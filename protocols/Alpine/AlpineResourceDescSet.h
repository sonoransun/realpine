/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <AlpineQueryPacket.h>
#include <AlpineResourceDesc.h>
#include <Common.h>
#include <vector>


class AlpineResourceDescSet
{
  public:
    AlpineResourceDescSet();

    AlpineResourceDescSet(const AlpineResourceDescSet & copy);

    ~AlpineResourceDescSet();

    AlpineResourceDescSet & operator=(const AlpineResourceDescSet & copy);


    using t_ResourceDescList = vector<AlpineResourceDesc>;


    bool clear();

    bool reserve(ulong size);  // if the expected size is know, provide a hint

    // These methods track data from query reply packets
    //
    bool getResourceDataPacket(AlpineQueryPacket * queryPacket);

    bool addResourceDataPacket(AlpineQueryPacket * queryPacket);

    ulong size();

    bool getCurrOffset(ulong & offset);

    bool getRemaining(ulong & numRemaining);

    bool addResource(AlpineResourceDesc & resource);

    bool addResourceList(t_ResourceDescList & resourceList);

    bool getResourceList(t_ResourceDescList & resourceList);


    // Internal types
    //
    using t_ResourceDescArray = vector<AlpineResourceDesc *>;


  private:
    ulong arraySize_;
    ulong numResources_;
    ulong currOffset_;
    t_ResourceDescArray * resourceList_;


    static ulong getResourceDescSize(AlpineResourceDesc * desc);

    bool resize(ulong newSize);
};
