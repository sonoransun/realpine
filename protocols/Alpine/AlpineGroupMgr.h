/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <ReadWriteSem.h>
#include <OptHash.h>
#include <vector>


class AlpineGroup;


class AlpineGroupMgr
{
  public:

    AlpineGroupMgr () = default;
    ~AlpineGroupMgr () = default;


    // Public types
    //
    using t_GroupIdList = vector<ulong>;



    static bool  createGroup (const string &  name,
                              const string &  description,
                              AlpineGroup *&  group);

    static bool  copyGroup (ulong           copyGroupId,
                            const string &  name,
                            const string &  description,
                            AlpineGroup *&  group);

    static bool  deleteGroup (ulong  groupId);

    static bool  exists (const string &  groupName);

    static bool  exists (ulong  groupId);

    static bool  listGroups (t_GroupIdList &  groupIdList);

    static bool  locateGroup (ulong           groupId,
                              AlpineGroup *&  group);

    static bool  locateGroup (const string &  groupName,
                              AlpineGroup *&  group);

    static bool  getDefaultGroup (AlpineGroup *&  group);



    // Private types
    //
    using t_GroupIdIndex = std::unordered_map< ulong,
                      AlpineGroup *,
                      OptHash<ulong>,
                      equal_to<ulong> >;

    using t_GroupIdIndexPair = std::pair<ulong, AlpineGroup *>;

    using t_GroupNameIndex = std::unordered_map< string,
                      AlpineGroup *,
                      OptHash<string>,
                      equal_to<string> >;

    using t_GroupNameIndexPair = std::pair<string, AlpineGroup *>;


  private:

    static bool                 initialized_s;
    static AlpineGroup *        defaultGroup_s;
    static t_GroupIdIndex *     groupIdIndex_s;
    static t_GroupNameIndex *   groupNameIndex_s;
    static ulong                currGroupId_s;
    static ReadWriteSem         dataLock_s;


    // Initialization performed by AlpineStack
    //
    static bool  initialize ();



    friend class AlpineStack;
    friend class AlpineGroup;
};


