/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AlpineProtocol.h>
#include <AlpineQueryOptionData.h>
#include <AlpineResourceDescSet.h>
#include <Log.h>
#include <StringUtils.h>


static const ulong defaultReserve = 256;


AlpineResourceDescSet::AlpineResourceDescSet()
{
#ifdef _VERBOSE
    Log::Debug("AlpineResourceDescSet constructor invoked.");
#endif

    arraySize_ = defaultReserve;
    numResources_ = 0;
    currOffset_ = 0;
    resourceList_ = new t_ResourceDescArray(arraySize_);


    for (auto & item : *resourceList_) {
        item = nullptr;
    }
}


AlpineResourceDescSet::AlpineResourceDescSet(const AlpineResourceDescSet & copy)
{
#ifdef _VERBOSE
    Log::Debug("AlpineResourceDescSet copy constructor invoked.");
#endif

    arraySize_ = copy.arraySize_;
    numResources_ = copy.numResources_;
    currOffset_ = copy.currOffset_;
    resourceList_ = new t_ResourceDescArray(arraySize_);

    ulong currIdx;
    AlpineResourceDesc * currDesc = nullptr;

    for (currIdx = 0; currIdx < numResources_; currIdx++) {
        currDesc = (*copy.resourceList_)[currIdx];
        if (currDesc) {
            (*resourceList_)[currIdx] = new AlpineResourceDesc(*currDesc);
        } else {
            (*resourceList_)[currIdx] = currDesc;
        }
    }
}


AlpineResourceDescSet::~AlpineResourceDescSet()
{
#ifdef _VERBOSE
    Log::Debug("AlpineResourceDescSet destructor invoked.");
#endif

    if (resourceList_) {

        for (const auto & item : *resourceList_) {
            if (item) {
                delete item;
            }
        }

        delete resourceList_;
    }
}


AlpineResourceDescSet &
AlpineResourceDescSet::operator=(const AlpineResourceDescSet & copy)
{
#ifdef _VERBOSE
    Log::Debug("AlpineResourceDescSet assignment invoked.");
#endif

    if (&copy == this) {
        return *this;
    }

    if (resourceList_) {
        delete resourceList_;
    }

    arraySize_ = copy.arraySize_;
    numResources_ = copy.numResources_;
    currOffset_ = copy.currOffset_;
    resourceList_ = new t_ResourceDescArray(arraySize_);

    ulong currIdx;
    AlpineResourceDesc * currDesc = nullptr;

    for (currIdx = 0; currIdx < numResources_; currIdx++) {
        currDesc = (*copy.resourceList_)[currIdx];
        if (currDesc) {
            (*resourceList_)[currIdx] = new AlpineResourceDesc(*currDesc);
        } else {
            (*resourceList_)[currIdx] = currDesc;
        }
    }


    return *this;
}


bool
AlpineResourceDescSet::clear()
{
#ifdef _VERBOSE
    Log::Debug("AlpineResourceDescSet::clear invoked.");
#endif

    numResources_ = 0;
    currOffset_ = 0;


    for (auto & item : *resourceList_) {
        if (item) {
            delete item;
            item = nullptr;
        }
    }


    return true;
}


bool
AlpineResourceDescSet::reserve(ulong size)
{
#ifdef _VERBOSE
    Log::Debug("AlpineResourceDescSet::reserve invoked.  Reserve Size: "s + std::to_string(size));
#endif

    if (arraySize_ >= size) {
        return true;
    }
    bool status;
    status = resize(size);


    return status;
}


bool
AlpineResourceDescSet::getResourceDataPacket(AlpineQueryPacket * queryPacket)
{
#ifdef _VERBOSE
    Log::Debug("AlpineResourceDescSet::getResourceDataPacket invoked.");
#endif

    if (numResources_ == 0) {
        Log::Debug("Call to AlpineResourceDescSet::getResourceDataPacket with no resources set!");
        return false;
    }
    if ((numResources_ - (currOffset_ + 1)) <= 0) {
        // Nothing left to get...
        Log::Debug("Nothing left to retrieve in AlpineResourceDescSet::getResourceDataPacket!");
        return false;
    }
    // Iterate through our list of resource data and pack the queryPacket until
    // we have used the allowed space for a reply.
    //
    AlpineResourceDesc * currDesc;
    ulong bytesRemaining;
    ulong recordSize;

    AlpineQueryPacket::t_ResourceDescList resultList;

    bytesRemaining = AlpineProtocol::maxResourceListSize_s;
    bool finished = false;

    while (!finished) {
        currDesc = (*resourceList_)[currOffset_];

        if (!currDesc) {
            Log::Error("Unexpected NULL record in resource list during call to "
                       "AlpineResourceDescSet::getResourceDataPacket!");
            return false;
        }
        recordSize = getResourceDescSize(currDesc);

        if (recordSize > AlpineProtocol::maxResourceListSize_s) {
            Log::Error("Record in resource list exceedes resource list byte limit in call to "
                       "AlpineResourceDescSet::getResourceDataPacket!");
            return false;
        }
        if (recordSize > bytesRemaining) {
            finished = true;
            continue;
        }

        resultList.push_back(*currDesc);
        currOffset_++;
        bytesRemaining -= recordSize;
    }

    queryPacket->setResourceDescList(resultList);


    return true;
}


bool
AlpineResourceDescSet::addResourceDataPacket(AlpineQueryPacket * queryPacket)
{
#ifdef _VERBOSE
    Log::Debug("AlpineResourceDescSet::addResourceDataPacket invoked.");
#endif

    // Add the associated set of resource descriptions to our ongoing list.
    //
    bool status;
    t_ResourceDescList resultList;

    status = queryPacket->getResourceDescList(resultList);

    if (!status) {
        Log::Error("getResourceDescList failed for query packet passed in call to "
                   "AlpineResourceDescSet::addResourceDataPacket!");
        return false;
    }
    // If we need to allocate more space to hold this many resources, do so now.
    //
    if (resultList.size() > (arraySize_ - currOffset_)) {
        ulong newSize = arraySize_ + (resultList.size() * 2);
        status = resize(newSize);

        if (!status) {
            Log::Error("Resize failed in call to AlpineResourceDescSet::addResourceDataPacket!");
            return false;
        }
    }

    for (const auto & item : resultList) {
        (*resourceList_)[currOffset_] = new AlpineResourceDesc(item);
        currOffset_++;
        numResources_++;
    }


    return true;
}


ulong
AlpineResourceDescSet::size()
{
#ifdef _VERBOSE
    Log::Debug("AlpineResourceDescSet::size invoked.");
#endif

    return numResources_;
}


bool
AlpineResourceDescSet::getCurrOffset(ulong & offset)
{
#ifdef _VERBOSE
    Log::Debug("AlpineResourceDescSet::getCurrOffset invoked.");
#endif

    offset = currOffset_;

    return true;
}


bool
AlpineResourceDescSet::getRemaining(ulong & numRemaining)
{
#ifdef _VERBOSE
    Log::Debug("AlpineResourceDescSet::getRemaining invoked.");
#endif

    numRemaining = numResources_ - (currOffset_ + 1);

    return true;
}


bool
AlpineResourceDescSet::addResource(AlpineResourceDesc & resource)
{
#ifdef _VERBOSE
    Log::Debug("AlpineResourceDescSet::addResource invoked.");
#endif

    // If we need to allocate more space to hold another resource record, do so now.
    //
    bool status;

    if ((arraySize_ - currOffset_) < 1) {
        ulong newSize = arraySize_ + (arraySize_ * 2);
        status = resize(newSize);

        if (!status) {
            Log::Error("Resize failed in call to AlpineResourceDescSet::addResource!");
            return false;
        }
    }

    (*resourceList_)[currOffset_] = new AlpineResourceDesc(resource);
    currOffset_++;
    numResources_++;


    return true;
}


bool
AlpineResourceDescSet::addResourceList(t_ResourceDescList & resourceList)
{
#ifdef _VERBOSE
    Log::Debug("AlpineResourceDescSet::addResourceList invoked.");
#endif

    // If we need to allocate more space to hold this many resources, do so now.
    //
    bool status;

    if (resourceList.size() > (arraySize_ - currOffset_)) {
        ulong newSize = arraySize_ + (resourceList.size() * 2);
        status = resize(newSize);

        if (!status) {
            Log::Error("Resize failed in call to AlpineResourceDescSet::addResourceList!");
            return false;
        }
    }

    for (const auto & item : resourceList) {
        (*resourceList_)[currOffset_] = new AlpineResourceDesc(item);
        currOffset_++;
        numResources_++;
    }


    return true;
}


bool
AlpineResourceDescSet::getResourceList(t_ResourceDescList & resourceList)
{
#ifdef _VERBOSE
    Log::Debug("AlpineResourceDescSet::getResourceList invoked.");
#endif

    resourceList.clear();

    ulong i;
    for (i = 0; i < numResources_; i++) {
        resourceList.push_back(*((*resourceList_)[i]));
    }


    return true;
}


ulong
AlpineResourceDescSet::getResourceDescSize(AlpineResourceDesc * desc)
{
    ulong retSize;

    AlpineResourceDesc::t_LocatorList locatorList;

    string description;
    AlpineQueryOptionData * optionData;

    retSize = sizeof(long) +  // matchId
              sizeof(long) +  // size
              sizeof(short);  // Locator list length

    desc->getLocatorList(locatorList);
    for (const auto & locatorItem : locatorList) {
        retSize += locatorItem.size() + 1;
    }

    desc->getDescription(description);
    retSize += description.size() + 1;
    retSize += sizeof(long);  // option ID

    if (desc->getOptionId() != 0) {
        desc->getOptionData(optionData);
        retSize += optionData->getOptionDataLength();
        delete optionData;
    }


    return retSize;
}


bool
AlpineResourceDescSet::resize(ulong newSize)
{
#ifdef _VERBOSE
    Log::Debug("AlpineResourceDescSet::resize invoked.");
#endif

    // General idea here is to create a copy vector of sufficient size, and then transfer
    // ownership of all allocated resources to the new one.
    //
    t_ResourceDescArray * newList;
    newList = new t_ResourceDescArray(newSize);

    ulong i;
    AlpineResourceDesc * currDesc;

    for (i = 0; i < numResources_; i++) {
        currDesc = (*resourceList_)[i];
        (*newList)[i] = currDesc;
    }

    // Initialize remaining vector members
    for (; i < newSize; i++) {
        (*newList)[i] = nullptr;
    }

    delete resourceList_;
    resourceList_ = newList;
    arraySize_ = newSize;


    return true;
}
