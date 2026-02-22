/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <Platform.h>


class FileUtils
{
  public:


    // Public types
    //
    struct t_FilePermission {
        bool  isReadable;
        bool  isWriteable;
        bool  isExecutable;
    };

    enum class t_FileType { Regular, Directory, Other };

    using t_Id = uint;
    using t_FileId = ulong;
    using t_FileSize = long;

    struct t_FileInfo {
        t_FileId            fileId; // aka I-node number
        t_Id                userId;
        t_Id                groupId;
        t_FilePermission    userAccess;
        t_FilePermission    groupAccess;
        t_FilePermission    worldAccess;
        t_FileType          fileType;
        t_FileSize          size; // bytes
        time_t              accessTime;
        time_t              modTime;
        time_t              changeTime;

        // Helper methods  (simply wrap calls to FileUtils)
        //
        bool  canRead () { return (FileUtils::canRead (*this)); }

        bool  canWrite () { return (FileUtils::canWrite (*this)); }

        bool  canExecute () { return (FileUtils::canExecute (*this)); }

    };



    // Utility operations
    //
    static bool  exists (const string &  filePath);

    static bool  getFileInfo (const string &  filePath,
                              t_FileInfo &    fileInfo);
  
 
    // Check to see if current user can perform various operations based on file permissions
    // 
    static bool  canRead (const string &  filePath);

    static bool  canRead (t_FileInfo &  fileInfo);

    static bool  canWrite (const string &  filePath);

    static bool  canWrite (t_FileInfo &  fileInfo);

    static bool  canExecute (const string &  filePath);

    static bool  canExecute (t_FileInfo &  fileInfo);


  private:

};

