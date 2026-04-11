/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AlpineGroup.h>
#include <AlpineGroupMgr.h>
#include <Log.h>
#include <ReadLock.h>
#include <StringUtils.h>
#include <WriteLock.h>


bool AlpineGroupMgr::initialized_s = false;
AlpineGroup * AlpineGroupMgr::defaultGroup_s = nullptr;
AlpineGroupMgr::t_GroupIdIndex * AlpineGroupMgr::groupIdIndex_s = nullptr;
AlpineGroupMgr::t_GroupNameIndex * AlpineGroupMgr::groupNameIndex_s = nullptr;
ulong AlpineGroupMgr::currGroupId_s = 0;
ReadWriteSem AlpineGroupMgr::dataLock_s;


// Ctor defaulted in header


// Dtor defaulted in header


bool
AlpineGroupMgr::initialize()
{
#ifdef _VERBOSE
    Log::Debug("AlpineGroupMgr::initialize invoked.");
#endif

    WriteLock lock(dataLock_s);

    if (initialized_s) {
        Log::Error("Attempt to reinitialize AlpineGroupMgr!");
        return false;
    }

    ulong defaultGroupId = currGroupId_s++;
    string defaultGroupName("default");
    string defaultGroupDesc("default peer group");

    defaultGroup_s = new AlpineGroup(defaultGroupId, defaultGroupName, defaultGroupDesc);

    groupIdIndex_s = new t_GroupIdIndex;
    groupNameIndex_s = new t_GroupNameIndex;

    // Add default group to indexes
    //
    groupIdIndex_s->emplace(defaultGroupId, defaultGroup_s);
    groupNameIndex_s->emplace(defaultGroupName, defaultGroup_s);

    initialized_s = true;


    return true;
}


bool
AlpineGroupMgr::createGroup(const string & name, const string & description, AlpineGroup *& group)
{
#ifdef _VERBOSE
    Log::Debug("AlpineGroupMgr::createGroup invoked.  Values: "s + "\n Name: "s + name + "\n Desc: "s + description +
               "\n");
#endif

    WriteLock lock(dataLock_s);

    if (!initialized_s) {
        Log::Error("Call to AlpineGroupMgr::createGroup before initialization!");
        return false;
    }

    // Make sure we dont have a group with this name
    //
    auto iter = groupNameIndex_s->find(name);

    if (iter != groupNameIndex_s->end()) {
        Log::Error("Duplicate group name in call to AlpineGroupMgr::createGroup!");
        return false;
    }

    // MRP_TEMP handle rollover condition
    //
    ulong groupId = currGroupId_s++;

    group = new AlpineGroup(groupId, name, description);


    // Add new group to indexes
    //
    groupIdIndex_s->emplace(groupId, group);
    groupNameIndex_s->emplace(name, group);


    return true;
}


bool
AlpineGroupMgr::copyGroup(ulong copyGroupId, const string & name, const string & description, AlpineGroup *& group)
{
#ifdef _VERBOSE
    Log::Debug("AlpineGroupMgr::copyGroup invoked.  Values: "s + "\n Copy ID: "s + std::to_string(copyGroupId) +
               "\n Name: "s + name + "\n Desc: "s + description + "\n");
#endif

    WriteLock lock(dataLock_s);

    if (!initialized_s) {
        Log::Error("Call to AlpineGroupMgr::copyGroup before initialization!");
        return false;
    }

    // Make sure we dont have a group with this name
    //
    auto nameIter = groupNameIndex_s->find(name);

    if (nameIter != groupNameIndex_s->end()) {
        Log::Error("Duplicate group name in call to AlpineGroupMgr::copyGroup!");
        return false;
    }

    AlpineGroup * copyGroup;
    auto idIter = groupIdIndex_s->find(copyGroupId);

    if (idIter == groupIdIndex_s->end()) {
        Log::Error("Group ID to copy does not exist in call to AlpineGroupMgr::copyGroup!");
        return false;
    }

    copyGroup = (*idIter).second;


    // MRP_TEMP handle rollover condition
    //
    ulong groupId = currGroupId_s++;

    group = new AlpineGroup(copyGroup, groupId, name, description);


    // Add new group to indexes
    //
    groupIdIndex_s->emplace(groupId, group);
    groupNameIndex_s->emplace(name, group);


    return true;
}


bool
AlpineGroupMgr::deleteGroup(ulong groupId)
{
#ifdef _VERBOSE
    Log::Debug("AlpineGroupMgr::deleteGroup invoked.  Group ID: "s + std::to_string(groupId));
#endif

    WriteLock lock(dataLock_s);

    if (!initialized_s) {
        Log::Error("Call to AlpineGroupMgr::deleteGroup before initialization!");
        return false;
    }

    AlpineGroup * group;
    auto iter = groupIdIndex_s->find(groupId);

    if (iter == groupIdIndex_s->end()) {
        Log::Error("Invalid group ID passed to AlpineGroupMgr::deleteGroup!");
        return false;
    }

    bool status;
    group = (*iter).second;


    // Cannot delete default group
    //
    if (group == defaultGroup_s) {
        Log::Error("Attempt to delete default group in AlpineGroupMgr::deleteGroup!");
        return false;
    }


    // Clean up and pending queries or other operations which may be inprogress before
    // destroying group entirely.
    //
    status = group->cancelAll();

    if (!status) {
        Log::Error("Cancel operations for group failed in AlpineGroupMgr::deleteGroup!");
        return false;
    }

    string groupName;
    group->getName(groupName);

    groupIdIndex_s->erase(groupId);
    groupNameIndex_s->erase(groupName);

    delete group;


    return true;
}


bool
AlpineGroupMgr::exists(const string & groupName)
{
#ifdef _VERBOSE
    Log::Debug("AlpineGroupMgr::exists(name) invoked.  Group Name: "s + groupName);
#endif

    ReadLock lock(dataLock_s);

    if (!initialized_s) {
        Log::Error("Call to AlpineGroupMgr::exists before initialization!");
        return false;
    }

    return groupNameIndex_s->find(groupName) != groupNameIndex_s->end();
}


bool
AlpineGroupMgr::exists(ulong groupId)
{
#ifdef _VERBOSE
    Log::Debug("AlpineGroupMgr::exists(id) invoked.  Group ID: "s + std::to_string(groupId));
#endif

    ReadLock lock(dataLock_s);

    if (!initialized_s) {
        Log::Error("Call to AlpineGroupMgr::exists before initialization!");
        return false;
    }

    return groupIdIndex_s->find(groupId) != groupIdIndex_s->end();
}


bool
AlpineGroupMgr::listGroups(t_GroupIdList & groupIdList)
{
#ifdef _VERBOSE
    Log::Debug("AlpineGroupMgr::listGroups invoked.");
#endif

    ReadLock lock(dataLock_s);

    if (!initialized_s) {
        Log::Error("Call to AlpineGroupMgr::listGroups before initialization!");
        return false;
    }

    groupIdList.clear();


    for (const auto & item : *groupIdIndex_s) {
        groupIdList.push_back(item.first);
    }


    return true;
}


bool
AlpineGroupMgr::locateGroup(ulong groupId, AlpineGroup *& group)
{
#ifdef _VERBOSE
    Log::Debug("AlpineGroupMgr::locateGroup(id) invoked.  Group ID: "s + std::to_string(groupId));
#endif

    ReadLock lock(dataLock_s);

    if (!initialized_s) {
        Log::Error("Call to AlpineGroupMgr::locateGroup before initialization!");
        return false;
    }

    auto iter = groupIdIndex_s->find(groupId);

    if (iter == groupIdIndex_s->end()) {
        Log::Error("Invalid group ID passed to AlpineGroupMgr::locateGroup!");
        return false;
    }

    group = (*iter).second;


    return true;
}


bool
AlpineGroupMgr::locateGroup(const string & groupName, AlpineGroup *& group)
{
#ifdef _VERBOSE
    Log::Debug("AlpineGroupMgr::locateGroup(name) invoked.  Group Name: "s + groupName);
#endif

    ReadLock lock(dataLock_s);

    if (!initialized_s) {
        Log::Error("Call to AlpineGroupMgr::locateGroup before initialization!");
        return false;
    }

    auto iter = groupNameIndex_s->find(groupName);

    if (iter == groupNameIndex_s->end()) {
        Log::Error("Invalid group name passed to AlpineGroupMgr::locateGroup!");
        return false;
    }

    group = (*iter).second;


    return true;
}


bool
AlpineGroupMgr::getDefaultGroup(AlpineGroup *& group)
{
#ifdef _VERBOSE
    Log::Debug("AlpineGroupMgr::getDefaultGroup invoked.");
#endif

    ReadLock lock(dataLock_s);

    if (!initialized_s) {
        Log::Error("Call to AlpineGroupMgr::getDefaultGroup before initialization!");
        return false;
    }

    group = defaultGroup_s;


    return true;
}
