/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <AlpineCorbaClient.h>
#include <CorbaUtils.h>
#include <string>
#include <vector>


class AlpineGroupMgmtIntf
{
  public:


    // Public types
    //
    struct t_AlpineGroupInfo {
        string   name;
        ulong    id;
        string   description;
        bool     open;
    };

    using t_AlpineGroupInfoList = vector<t_AlpineGroupInfo>;


    // Supported interface operations
    //
    static bool  getUserGroupList (t_AlpineGroupInfoList & groupList);

    static bool  createUserGroup (const string &  groupName,
                                  const string &  description,
                                  ulong &         groupId);

    static bool  destroyUserGroup (ulong groupId);

    static bool  getPeerUserGroupList (ulong                    peerId,
                                       t_AlpineGroupInfoList &  groupList);

    static bool  addPeerToGroup (ulong  peerId,
                                 ulong  groupId);

    static bool  removePeerFromGroup (ulong  peerId,
                                      ulong  groupId);


  private:

};

