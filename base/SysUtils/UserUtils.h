/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#ifndef ALPINE_PLATFORM_WINDOWS
#include <sys/types.h>
#endif


class UserUtils
{
  public:
    // Public types
    //
    using t_Id = uint;


    // User methods
    //
    static bool userExists(t_Id userId);

    static bool userExists(const string & userName);

    static bool getUserName(t_Id userId, string & userName);

    static bool getUserId(const string & userName, t_Id & userId);

    static t_Id getMyRealUserId();

    static string getMyRealUserName();

    static t_Id getMyEffectiveUserId();

    static string getMyEffectiveUserName();


    // Group methods
    //
    static bool groupExists(t_Id groupId);

    static bool groupExists(const string & groupName);

    static bool getGroupName(t_Id groupId, string & groupName);

    static bool getGroupId(const string & groupName, t_Id & groupId);

    static t_Id getMyRealGroupId();

    static string getMyRealGroupName();

    static t_Id getMyEffectiveGroupId();

    static string getMyEffectiveGroupName();


  private:
};
