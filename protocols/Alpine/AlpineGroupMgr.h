///////
///
///  Copyright (C) 2026  sonoransun
///
///  Permission is hereby granted, free of charge, to any person obtaining a copy
///  of this software and associated documentation files (the "Software"), to deal
///  in the Software without restriction, including without limitation the rights
///  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
///  copies of the Software, and to permit persons to whom the Software is
///  furnished to do so, subject to the following conditions:
///
///  The above copyright notice and this permission notice shall be included in all
///  copies or substantial portions of the Software.
///
///  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
///  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
///  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
///  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
///  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
///  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
///  SOFTWARE.
///
///////


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


