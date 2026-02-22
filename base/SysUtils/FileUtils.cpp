/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <FileUtils.h>
#include <UserUtils.h>
#include <Log.h>
#include <StringUtils.h>
#include <cerrno>



bool  
FileUtils::exists (const string &  filePath)
{
    int retVal;
    struct stat  fileStat;


    retVal = stat (filePath.c_str(), &fileStat);

    if (retVal != 0) {
        if ( (errno == ENOENT) ||
             (errno == ENOTDIR) ||
             (errno == ENAMETOOLONG) ) {

            // Assume this is normal, file does not exist
            return false;
        }

        Log::Error ("Stat call failed for file: "s + filePath +
                    " in call to FileUtils::exists with errno: "s +
                    std::to_string (errno));

        return false;
    }

    return true;
}



bool  
FileUtils::getFileInfo (const string &  filePath,
                        t_FileInfo &    fileInfo)
{
    int retVal;
    struct stat  fileStat;


    retVal = stat (filePath.c_str(), &fileStat);

    if (retVal != 0) {
        Log::Error ("Stat call failed for file: "s + filePath +
                    " in call to FileUtils::getFileInfo with errno: "s +
                    std::to_string (errno));

        return false;
    }


    // Assign file information
    //
#ifdef ALPINE_PLATFORM_WINDOWS
    fileInfo.fileId       = 0;
    fileInfo.userId       = 0;
    fileInfo.groupId      = 0;
#else
    fileInfo.fileId       = fileStat.st_ino;
    fileInfo.userId       = fileStat.st_uid;
    fileInfo.groupId      = fileStat.st_gid;
#endif
    fileInfo.size         = fileStat.st_size;
    fileInfo.accessTime   = fileStat.st_atime;
    fileInfo.modTime      = fileStat.st_mtime;
    fileInfo.changeTime   = fileStat.st_ctime;

#ifdef ALPINE_PLATFORM_WINDOWS
    // Windows: simplified permissions — read/write based on _S_IREAD/_S_IWRITE
    bool readable   = (fileStat.st_mode & _S_IREAD)  != 0;
    bool writeable  = (fileStat.st_mode & _S_IWRITE) != 0;
    bool executable = readable;  // Windows doesn't have execute bit

    fileInfo.userAccess  = { readable, writeable, executable };
    fileInfo.groupAccess = { readable, writeable, executable };
    fileInfo.worldAccess = { readable, writeable, executable };
#else
    fileInfo.userAccess.isReadable    = (fileStat.st_mode & S_IRUSR) != 0;
    fileInfo.userAccess.isWriteable   = (fileStat.st_mode & S_IWUSR) != 0;
    fileInfo.userAccess.isExecutable  = (fileStat.st_mode & S_IXUSR) != 0;

    fileInfo.groupAccess.isReadable   = (fileStat.st_mode & S_IRGRP) != 0;
    fileInfo.groupAccess.isWriteable  = (fileStat.st_mode & S_IWGRP) != 0;
    fileInfo.groupAccess.isExecutable = (fileStat.st_mode & S_IXGRP) != 0;

    fileInfo.worldAccess.isReadable   = (fileStat.st_mode & S_IROTH) != 0;
    fileInfo.worldAccess.isWriteable  = (fileStat.st_mode & S_IWOTH) != 0;
    fileInfo.worldAccess.isExecutable = (fileStat.st_mode & S_IXOTH) != 0;
#endif

    if (S_ISREG (fileStat.st_mode)) {
        fileInfo.fileType = t_FileType::Regular;
    }
    else if (S_ISDIR (fileStat.st_mode)) {
        fileInfo.fileType = t_FileType::Directory;
    }
    else {
        fileInfo.fileType = t_FileType::Other;
    }


    return true;
}


  
bool  
FileUtils::canRead (const string &  filePath)
{
    bool status;
    t_FileInfo  fileInfo;

    status = exists (filePath);
    if (!status) {
        Log::Error ("File path: "s + filePath +
                    " passed in call to FileUtils::canRead does not exist!");
        return false;
    }

    status = getFileInfo (filePath, fileInfo);
    if (!status) {
        Log::Error ("Get file info failed for file: "s + filePath +
                    " in call to FileUtils::canRead!");
        return false;
    }

    status = canRead (fileInfo);
    return status;
}



bool  
FileUtils::canRead (t_FileInfo &  fileInfo)
{
    // Check permissions for world, then group, then user.
    // (Note that we check against effective user ID)
    //
    if (fileInfo.worldAccess.isReadable) {
        return true;
    }

    UserUtils::t_Id  myId;
    myId = UserUtils::getMyEffectiveGroupId ();

    if ( (fileInfo.groupId == myId) && (fileInfo.groupAccess.isReadable) ) {
        return true;
    }

    myId = UserUtils::getMyEffectiveUserId ();

    return (fileInfo.userId == myId) && (fileInfo.userAccess.isReadable);
}



bool  
FileUtils::canWrite (const string &  filePath)
{
    bool status;
    t_FileInfo  fileInfo;

    status = exists (filePath);
    if (!status) {
        Log::Error ("File path: "s + filePath + 
                    " passed in call to FileUtils::canWrite does not exist!");
        return false;
    }

    status = getFileInfo (filePath, fileInfo);
    if (!status) {
        Log::Error ("Get file info failed for file: "s + filePath +
                    " in call to FileUtils::canWrite!");
        return false;
    }

    status = canWrite (fileInfo);
    return status;
}



bool  
FileUtils::canWrite (t_FileInfo &  fileInfo)
{
    // Check permissions for world, then group, then user.
    // (Note that we check against effective user ID)
    //
    if (fileInfo.worldAccess.isWriteable) {
        return true;
    }

    UserUtils::t_Id  myId;
    myId = UserUtils::getMyEffectiveGroupId ();

    if ( (fileInfo.groupId == myId) && (fileInfo.groupAccess.isWriteable) ) {
        return true;
    }

    myId = UserUtils::getMyEffectiveUserId ();

    return (fileInfo.userId == myId) && (fileInfo.userAccess.isWriteable);
}



bool  
FileUtils::canExecute (const string &  filePath)
{
    bool status;
    t_FileInfo  fileInfo;

    status = exists (filePath);
    if (!status) {
        Log::Error ("File path: "s + filePath + 
                    " passed in call to FileUtils::canExecute does not exist!");
        return false;
    }

    status = getFileInfo (filePath, fileInfo);
    if (!status) {
        Log::Error ("Get file info failed for file: "s + filePath +
                    " in call to FileUtils::canExecute!");
        return false;
    }

    status = canExecute (fileInfo);
    return status;
}



bool  
FileUtils::canExecute (t_FileInfo &  fileInfo)
{
    // Check permissions for world, then group, then user.
    // (Note that we check against effective user ID)
    //
    if (fileInfo.worldAccess.isExecutable) {
        return true;
    }

    UserUtils::t_Id  myId;
    myId = UserUtils::getMyEffectiveGroupId ();

    if ( (fileInfo.groupId == myId) && (fileInfo.groupAccess.isExecutable) ) {
        return true;
    }

    myId = UserUtils::getMyEffectiveUserId ();

    return (fileInfo.userId == myId) && (fileInfo.userAccess.isExecutable);
}



