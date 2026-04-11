/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <Log.h>
#include <Platform.h>
#include <StringUtils.h>
#include <UserUtils.h>

#ifdef ALPINE_PLATFORM_WINDOWS
#include <windows.h>
#else
#include <grp.h>
#include <pwd.h>
#endif


#ifdef ALPINE_PLATFORM_WINDOWS

// ---------------------------------------------------------------------------
// Windows stubs
// ---------------------------------------------------------------------------

bool
UserUtils::userExists(t_Id /*userId*/)
{
    return false;
}


bool
UserUtils::userExists(const string & /*userName*/)
{
    return false;
}


bool
UserUtils::getUserName(t_Id /*userId*/, string & userName)
{
    char buf[256];
    DWORD len = sizeof(buf);
    if (GetUserNameA(buf, &len)) {
        userName = buf;
        return true;
    }
    return false;
}


bool
UserUtils::getUserId(const string & /*userName*/, t_Id & /*userId*/)
{
    return false;
}


UserUtils::t_Id
UserUtils::getMyRealUserId()
{
    return 0;
}


string
UserUtils::getMyRealUserName()
{
    string result;
    getUserName(0, result);
    return result;
}


UserUtils::t_Id
UserUtils::getMyEffectiveUserId()
{
    return 0;
}


string
UserUtils::getMyEffectiveUserName()
{
    string result;
    getUserName(0, result);
    return result;
}


bool
UserUtils::groupExists(t_Id /*groupId*/)
{
    return false;
}


bool
UserUtils::groupExists(const string & /*groupName*/)
{
    return false;
}


bool
UserUtils::getGroupName(t_Id /*groupId*/, string & /*groupName*/)
{
    return false;
}


bool
UserUtils::getGroupId(const string & /*groupName*/, t_Id & /*groupId*/)
{
    return false;
}


UserUtils::t_Id
UserUtils::getMyRealGroupId()
{
    return 0;
}


string
UserUtils::getMyRealGroupName()
{
    return {};
}


UserUtils::t_Id
UserUtils::getMyEffectiveGroupId()
{
    return 0;
}


string
UserUtils::getMyEffectiveGroupName()
{
    return {};
}


#else  // POSIX


bool
UserUtils::userExists(t_Id userId)
{
    struct passwd * passwdInfo;

    passwdInfo = getpwuid(userId);

    return passwdInfo != nullptr;
}


bool
UserUtils::userExists(const string & userName)
{
    struct passwd * passwdInfo;

    passwdInfo = getpwnam(userName.c_str());

    return passwdInfo != nullptr;
}


bool
UserUtils::getUserName(t_Id userId, string & userName)
{
    struct passwd * passwdInfo;

    passwdInfo = getpwuid(userId);

    if (!passwdInfo) {
        Log::Error("Invalid user id: "s + std::to_string(userId) + " passed in call to UserUtils::getUserName!");
        return false;
    }

    userName = passwdInfo->pw_name;

    return true;
}


bool
UserUtils::getUserId(const string & userName, t_Id & userId)
{
    struct passwd * passwdInfo;

    passwdInfo = getpwnam(userName.c_str());

    if (!passwdInfo) {
        Log::Error("Invalid user name: "s + userName + " passed in call to UserUtils::getUserId!");
        return false;
    }

    userId = passwdInfo->pw_uid;

    return true;
}


UserUtils::t_Id
UserUtils::getMyRealUserId()
{
    return (getuid());
}


string
UserUtils::getMyRealUserName()
{
    string result;
    getUserName(getuid(), result);

    return result;
}


UserUtils::t_Id
UserUtils::getMyEffectiveUserId()
{
    return (geteuid());
}


string
UserUtils::getMyEffectiveUserName()
{
    string result;
    getUserName(geteuid(), result);

    return result;
}


bool
UserUtils::groupExists(t_Id groupId)
{
    struct group * groupInfo;

    groupInfo = getgrgid(groupId);

    return groupInfo != nullptr;
}


bool
UserUtils::groupExists(const string & groupName)
{
    struct group * groupInfo;

    groupInfo = getgrnam(groupName.c_str());

    return groupInfo != nullptr;
}


bool
UserUtils::getGroupName(t_Id groupId, string & groupName)
{
    struct group * groupInfo;

    groupInfo = getgrgid(groupId);

    if (!groupInfo) {
        Log::Error("Invalid group id: "s + std::to_string(groupId) + " passed in call to UserUtils::getGroupName!");
        return false;
    }

    groupName = groupInfo->gr_name;

    return true;
}


bool
UserUtils::getGroupId(const string & groupName, t_Id & groupId)
{
    struct group * groupInfo;

    groupInfo = getgrnam(groupName.c_str());

    if (!groupInfo) {
        Log::Error("Invalid group name: "s + groupName + " passed in call to UserUtils::getGroupId!");
        return false;
    }

    groupId = groupInfo->gr_gid;

    return true;
}


UserUtils::t_Id
UserUtils::getMyRealGroupId()
{
    return (getgid());
}


string
UserUtils::getMyRealGroupName()
{
    string result;
    getGroupName(getgid(), result);

    return result;
}


UserUtils::t_Id
UserUtils::getMyEffectiveGroupId()
{
    return (getegid());
}


string
UserUtils::getMyEffectiveGroupName()
{
    string result;
    getGroupName(getegid(), result);

    return result;
}


#endif
