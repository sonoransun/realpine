/// Copyright (C) 2026 sonoransun — see LICENCE.txt



#include <SpawnProcess.h>
#include <Log.h>
#include <StringUtils.h>
#include <string.h>
#include <vector>

#ifdef ALPINE_PLATFORM_WINDOWS
#include <windows.h>
#else
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <grp.h>
#include <pwd.h>
extern char ** environ;
#endif



SpawnProcess::SpawnProcess ()
{
#ifdef _VERBOSE
    Log::Debug ("SpawnProcess constructor invoked.");
#endif

    initialize ();
}



SpawnProcess::~SpawnProcess ()
{
#ifdef _VERBOSE
    Log::Debug ("SpawnProcess destructor invoked.");
#endif

}



SpawnProcess::SpawnProcess (const SpawnProcess & copy)
{
#ifdef _VERBOSE
    Log::Debug ("SpawnProcess copy constructor invoked.");
#endif

    environmentMap_  = copy.environmentMap_;
    arguments_       = copy.arguments_;
    command_         = copy.command_;
    execAsUser_      = copy.execAsUser_;
    execAsGroup_     = copy.execAsGroup_;
}



void
SpawnProcess::initialize ()
{
#ifdef _VERBOSE
    Log::Debug ("SpawnProcess::initialize invoked.");
#endif

    execAsUser_       = 0;
    execAsGroup_      = 0;
    childProcessId_   = 0;
    childEnvironment_ = nullptr;
    childArguments_   = nullptr;

    arguments_.erase ();
    command_.erase ();

    readEnvVars();
}



void
SpawnProcess::readEnvVars ()
{
#ifdef _VERBOSE
    Log::Debug ("SpawnProcess::readEnvVars invoked.");
#endif

    environmentMap_.clear();

#ifdef ALPINE_PLATFORM_WINDOWS
    // On Windows, use GetEnvironmentStrings
    char* envBlock = GetEnvironmentStringsA();
    if (envBlock) {
        const char* curr = envBlock;
        while (*curr) {
            std::string entry(curr);
            auto pos = entry.find('=');
            if (pos != std::string::npos && pos > 0) {
                environmentMap_.emplace(entry.substr(0, pos), entry.substr(pos + 1));
            }
            curr += entry.size() + 1;
        }
        FreeEnvironmentStringsA(envBlock);
    }
#else
    char **envVars = environ;

    const char *delimiter = "=";
    char *currKey;
    char *currValue;
    char *nextSubstr;
    char *currSet;


    // Parse env settings into key / value pairs and update envronmentMap_
    //
    const int buffSize = 2048;
    currSet = new char[buffSize];

    while (*envVars) {
        strncpy(currSet, *envVars, buffSize-1);

        nextSubstr = nullptr;
        currKey = strtok_r(currSet, delimiter, &nextSubstr);

        if (!currKey) {
            envVars++;
            continue;
        }

        currValue = strtok_r(nullptr, delimiter, &nextSubstr);

        if (!currValue) {
            envVars++;
            continue;
        }

        environmentMap_.emplace(std::string(currKey), std::string(currValue));

        envVars++;
    }

    delete [] currSet;
#endif
}



const SpawnProcess &
SpawnProcess::operator = (const SpawnProcess & copy)
{
#ifdef _VERBOSE
    Log::Debug ("SpawnProcess::operator = invoked.");
#endif

    if (&copy == this) {
        return *this;
    }

    environmentMap_  = copy.environmentMap_;
    arguments_       = copy.arguments_;
    command_         = copy.command_;
    execAsUser_      = copy.execAsUser_;
    execAsGroup_     = copy.execAsGroup_;


    return *this;
}



bool 
SpawnProcess::spawnProcess ()
{
     Log::Debug ("SpawnProcess::spawnProcess invoked for command: "s + command_ );

     if (command_.empty()) {
         return false;
     }
     createExecInfo();

     forkChildProcess();


     return true;
}



bool
SpawnProcess::setCommand (const string & command)
{
#ifdef _VERBOSE
    Log::Debug ("SpawnProcess::setCommand invoked.");
#endif

    string newCommand = command;
    bool cmdValid = true;

    cmdValid = verifyCommand (newCommand);

    if( cmdValid ) {
        command_ = newCommand;
    }
  
 
    return cmdValid;
}



bool 
SpawnProcess::setCommand (const string & command,
                          const string & arguments)
{
#ifdef _VERBOSE
    Log::Debug ("SpawnProcess::setCommand invoked.");
#endif

    bool status;

    status = setCommand (command);

    if (status) {
        arguments_ = arguments;
    }


    return status;
}



void
SpawnProcess::setArguments (const string & arguments)
{
#ifdef _VERBOSE
    Log::Debug ("SpawnProcess::setArguments invoked.");
#endif

    arguments_ = arguments;
}



bool
SpawnProcess::setChildUser (const string & userName)
{
#ifdef _VERBOSE
    Log::Debug ("SpawnProcess::setChildUser invoked.");
#endif

#ifdef ALPINE_PLATFORM_WINDOWS
    Log::Info ("SpawnProcess::setChildUser not supported on Windows.");
    return false;
#else
    bool userIdSet = false;

    uid_t userId;

    // Only allow if we are running with root priveleges.
    //
    if ( (getuid () == 0) ||
         (geteuid () == 0) ) {

        userIdSet = resolveUserName (userName, userId);

        if (userIdSet) {
            execAsUser_ = userId;
        }
    }


    return userIdSet;
#endif
}



bool
SpawnProcess::setChildUser (uid_t userId)
{
#ifdef _VERBOSE
    Log::Debug ("SpawnProcess::setChildUser invoked.");
#endif

#ifdef ALPINE_PLATFORM_WINDOWS
    Log::Info ("SpawnProcess::setChildUser not supported on Windows.");
    return false;
#else
    bool userIdSet = false;

    // Only allow if we are running with root.
    //
    if ( (getuid () == 0) ||
         (geteuid () == 0) ) {

        string userName;
        userIdSet = resolveUserId (userId, userName);

        if (userIdSet) {
            execAsUser_ = userId;
        }
    }


    return(userIdSet);
#endif
}



bool
SpawnProcess::setChildGroup (const string & groupName)
{
#ifdef _VERBOSE
    Log::Debug ("SpawnProcess::setChildGroup invoked.");
#endif

#ifdef ALPINE_PLATFORM_WINDOWS
    Log::Info ("SpawnProcess::setChildGroup not supported on Windows.");
    return false;
#else
    bool groupIdSet = false;

    gid_t groupId;

    // Only allow changing group ids if we are running with root.
    //
    if ( (getuid () == 0) ||
         (geteuid () == 0) ) {

        groupIdSet = resolveGroupName (groupName, groupId);

        if (groupIdSet) {
            execAsGroup_ = groupId;
        }
    }


    return(groupIdSet);
#endif
}



bool
SpawnProcess::setChildGroup (gid_t groupId)
{
#ifdef _VERBOSE
    Log::Debug ("SpawnProcess::setChildGroup invoked.");
#endif

#ifdef ALPINE_PLATFORM_WINDOWS
    Log::Info ("SpawnProcess::setChildGroup not supported on Windows.");
    return false;
#else
    bool groupIdSet = false;

    // Only allow changing group ids if we are running with root.
    //
    if ( (getuid () == 0) ||
         (geteuid () == 0) ) {

        string groupName;
        groupIdSet = resolveGroupId (groupId, groupName);

        if (groupIdSet) {
            execAsGroup_ = groupId;
        }
    }


    return(groupIdSet);
#endif
}



bool
SpawnProcess::setEnvVariable (const string & name,
                              const string & value)
{
#ifdef _VERBOSE
    Log::Debug ("SpawnProcess::setEnvVariable invoked.");
#endif

    // If this value exists, we overwrite, otherwise add...
    //
    auto iter = environmentMap_.find (name);
   
    if (iter == environmentMap_.end()) {
        environmentMap_.emplace(name, value);
    }
    else {
        (*iter).second = value;
    }


    return(true);
}



int
SpawnProcess::waitOnChild ()
{
#ifdef _VERBOSE
    Log::Debug ("SpawnProcess::waitOnChild invoked.");
#endif

    int exitStatus = 0;

#ifdef ALPINE_PLATFORM_WINDOWS
    if (childProcessId_ == 0) return 0;
    HANDLE hProcess = OpenProcess(SYNCHRONIZE | PROCESS_QUERY_INFORMATION, FALSE, childProcessId_);
    if (hProcess) {
        WaitForSingleObject(hProcess, INFINITE);
        DWORD code = 0;
        GetExitCodeProcess(hProcess, &code);
        exitStatus = static_cast<int>(code);
        CloseHandle(hProcess);
    }
#else
    // First verify that child process is alive...
    //
    if (kill (childProcessId_, 0)) {
        // Child process is not alive, no exit status...
        //
        return(exitStatus);
    }

    const int waitOptions = 0;
    waitpid (childProcessId_, &exitStatus, waitOptions);
#endif

    return(exitStatus);
}



bool
SpawnProcess::forkChildProcess ()
{
#ifdef _VERBOSE
    Log::Debug ("SpawnProcess::forkChildProcess invoked.");
#endif

    bool statusOk = true;

#ifdef ALPINE_PLATFORM_WINDOWS
    // Build command line: "command arg1 arg2 ..."
    std::string cmdLine = command_;
    if (!arguments_.empty()) {
        cmdLine += " " + arguments_;
    }

    // Build environment block: "KEY=VALUE\0KEY=VALUE\0\0"
    std::string envBlock;
    for (const auto& item : environmentMap_) {
        envBlock += item.first + "=" + item.second;
        envBlock.push_back('\0');
    }
    envBlock.push_back('\0');

    STARTUPINFOA si = {};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi = {};

    BOOL created = CreateProcessA(
        nullptr,
        const_cast<char*>(cmdLine.c_str()),
        nullptr, nullptr,
        FALSE,
        0,
        const_cast<char*>(envBlock.c_str()),
        nullptr,
        &si, &pi);

    if (!created) {
        Log::Error("CreateProcess failed in SpawnProcess::forkChildProcess. Error: "s +
                   std::to_string(GetLastError()));
        statusOk = false;
        childProcessId_ = 0;
    } else {
        childProcessId_ = static_cast<pid_t>(pi.dwProcessId);
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    }
#else
    childProcessId_ = fork ();

    if (childProcessId_ == -1) {
        // fork failed.
        //
        Log::Error ("fork() call failed in SpawnProcess::forkChildProcess.  Errno: "s +
                    std::string(strerror(errno)));

        statusOk = false;
        childProcessId_ = 0;
    }
    else if (childProcessId_ == 0) {
        // we are the child process
        //
        execChildProcess();
        return true;
    }


    // parent process, cleanup...
    //
    int i;

    i = 0;
    while (childEnvironment_[i]) {
        free (childEnvironment_[i++]);
    }

    i = 0;
    while (childArguments_[i]) {
        free (childArguments_[i++]);
    }

    delete [] childEnvironment_;
    delete [] childArguments_;

    childEnvironment_ = nullptr;
    childArguments_   = nullptr;
#endif

    return(statusOk);
}



void
SpawnProcess::createExecInfo ()
{
#ifdef _VERBOSE
    Log::Debug ("SpawnProcess::createExecInfo invoked.");
#endif

    // allocate and populate the environment map...    
    //
    childEnvironment_ = new char * [(environmentMap_.size() + 1)];
   
    int currStr = 0;    for (const auto& item : environmentMap_) {

        string envPair = item.first + "=" + item.second;
        childEnvironment_[currStr++] = strdup (envPair.c_str());
    }

    childEnvironment_[currStr] = nullptr;


    // Create our list of arguments
    //
    vector<string> argumentList;
 
    argumentList.push_back (command_);

    if (arguments_.size ()) {

        const char * delimiter = " ";
        char * argList;
        char * currValue;
        char * nextSubstr = nullptr;
        const int buffSize = 2048;
        argList = new char[buffSize];
        bool finished = false;

        strncpy (argList, arguments_.c_str(), buffSize-1);

        currValue = strtok_r (argList, delimiter, &nextSubstr);

        if (!currValue) {
            finished = true;
        }
        else {
            argumentList.emplace_back(currValue);
        }

        while (!finished) {

            currValue = strtok_r (nullptr, delimiter, &nextSubstr);

            if (!currValue) {
                finished = true;
                continue;
            }

            argumentList.emplace_back(currValue);
        }

        delete [] argList;
    }


    childArguments_ = new char * [(argumentList.size() + 1)];

    // populate command array..
    //
    currStr = 0;

    for (const auto& arg : argumentList) {

        childArguments_[currStr++] = strdup (arg.c_str());
    }

    childArguments_[currStr] = nullptr;
}



void
SpawnProcess::execChildProcess ()
{
#ifdef ALPINE_PLATFORM_WINDOWS
    // On Windows, CreateProcess is used in forkChildProcess — this is unreachable
#else
    // close all open files, use sysconf to obtain FD limit.
    //
    int maxFds = sysconf(_SC_OPEN_MAX);

    if (maxFds == -1) {
        // Sys call failed, default to 1024
        maxFds = 1024;
    }

    for (int fd = 3; fd < maxFds; fd++) {
        close(fd);
    }

    // start a new sessions, no tty.
    setsid();

    // change user and group id's if root and values defined.
    //
    if ( (getuid () == 0) ||
         (geteuid () == 0) ) {

        if (execAsUser_ != 0) {
            setuid (execAsUser_);
        }
        if (execAsGroup_ != 0) {
            setgid (execAsGroup_);
        }
    }


    execve (command_.c_str(), childArguments_, childEnvironment_);

    // if we are still here at this point, the exec failed.
    // use _exit to avoid problems with parent envrionment.
    //
    _exit(1);
#endif
}



bool
SpawnProcess::verifyCommand (string & command) 
{
#ifdef _VERBOSE
    Log::Debug ("SpawnProcess::verifyCommand invoked.");
#endif


    if (command.empty()) {
        return(false);
    }


    // If the entire command line was passed (erroniously) then split into
    // command and arguments...
    //
    vector<string>  tokens;

    const char * delimiter = " ";
    char * argList;
    char * currValue;
    char * nextSubstr = nullptr;
    const int buffSize = 2048;
    argList = new char[buffSize];
    bool finished = false;

    strncpy (argList, command.c_str(), buffSize-1);

    currValue = strtok_r (argList, delimiter, &nextSubstr);

    if (!currValue) {
        finished = true;
    }
    else {
        tokens.emplace_back(currValue);
    }

    while (!finished) {

        currValue = strtok_r (nullptr, delimiter, &nextSubstr);

        if (!currValue) {
            finished = true;
            continue;
        }

        tokens.emplace_back(currValue);
    }

    if (tokens.size() > 1) {
        string arguments;

        auto iter = tokens.begin();
        command = (*iter);
        iter++;

        for ( ; iter != tokens.end(); iter++) { 
            arguments += (*iter) + " ";
        }

        setArguments (arguments);
    }


    // Verify command within path and executable...
#ifdef ALPINE_PLATFORM_WINDOWS
    delimiter = ";";
    const char   *pathEnv = "PATH";
    const char   *dirSeparator = "\\";
#else
    delimiter = ":";
    const char   *pathEnv = "PATH";
    const char   *dirSeparator = "/";
#endif
    char         *currSet;
    char         *currDir;
    string       fullCommand;
    bool         commandValid = false;

  
    // Only check path if a relative path was given for
    // the process command... 
    if (command[0] == *dirSeparator) {
        commandValid = checkExecutePermission (command.c_str());
    }
    else {
        // Relative command, check for an executable file in path...

        // buffer for parsing path directories.
        currSet = new char[buffSize];

        strncpy (currSet, getenv(pathEnv), buffSize-1);

        nextSubstr = nullptr;
        currDir = strtok_r (currSet, delimiter, &nextSubstr);

        bool done = false;
        while (!done && (currDir)) {
            fullCommand = currDir;
            fullCommand += dirSeparator + command;

            if (checkExecutePermission (fullCommand.c_str())) {
                command = fullCommand;
                commandValid = true;
                done = true;
                continue;
            }

            currDir = strtok_r (nullptr, delimiter, &nextSubstr);
        }

        delete [] currSet;
    }


    return commandValid;
}



bool
SpawnProcess::checkExecutePermission (const string & file)
{
#ifdef _VERBOSE
    Log::Debug ("SpawnProcess::checkExecutePermission invoked.");
#endif

    bool fileExecutable = false;

#ifdef ALPINE_PLATFORM_WINDOWS
    // On Windows, check if file exists and has .exe/.bat/.cmd extension
    struct _stat fileStat;
    if (_stat(file.c_str(), &fileStat) == 0) {
        if (fileStat.st_mode & _S_IFREG) {
            fileExecutable = true;
        }
    }
#else
    struct stat  fileStat;

    if (stat (file.c_str(), &fileStat) == 0) {

        // Verify that this is a file (not directory, FIFO, etc)
        if (S_ISREG(fileStat.st_mode)) {

            // Check world execute permissions...
            if (fileStat.st_mode & S_IXOTH) {
                fileExecutable = true;
            }
            // Check group execute permissions...
            else if ( ((fileStat.st_gid == getgid()) || (fileStat.st_gid == getegid()))
                      && (fileStat.st_mode & S_IXGRP) ) {
                fileExecutable = true;
            }
            // Check user execute permissions...
            else if ( ((fileStat.st_uid == getuid()) || (fileStat.st_uid == geteuid()))
                      && fileStat.st_mode & S_IXUSR) {
                fileExecutable = true;
            }
        }
    }
#endif

    return fileExecutable;
}



#ifdef ALPINE_PLATFORM_WINDOWS

bool SpawnProcess::resolveUserName (const string &, uid_t &)  { return false; }
bool SpawnProcess::resolveUserId (const uid_t, string &)      { return false; }
bool SpawnProcess::resolveGroupName (const string &, gid_t &) { return false; }
bool SpawnProcess::resolveGroupId (const gid_t, string &)     { return false; }

#else

bool
SpawnProcess::resolveUserName (const string & userName,
                               uid_t &        userId)
{
#ifdef _VERBOSE
    Log::Debug ("SpawnProcess::resolveUserName invoked.");
#endif

    bool userExists;

    struct passwd *passwdInfo;

    passwdInfo = getpwnam (userName.c_str());

    if (!passwdInfo) {
        userExists = false;
    }
    else {
        userExists = true;
        userId     = passwdInfo->pw_uid;
    }


    return userExists;
}



bool
SpawnProcess::resolveUserId (const uid_t userId,
                             string &    userName)
{
#ifdef _VERBOSE
    Log::Debug ("SpawnProcess::resolveUserId invoked.");
#endif

    bool userExists;

    struct passwd * passwdInfo;

    passwdInfo = getpwuid (userId);

    if (!passwdInfo) {
        userExists = false;
    }
    else {
        userExists = true;
        userName   = passwdInfo->pw_name;
    }


    return userExists;
}



bool
SpawnProcess::resolveGroupName (const string & groupName,
                                gid_t &        groupId)
{
#ifdef _VERBOSE
    Log::Debug ("SpawnProcess::resolveGroupName invoked.");
#endif

    bool groupExists;

    struct group * groupInfo;

    groupInfo = getgrnam (groupName.c_str());

    if (!groupInfo) {
        groupExists = false;
    }
    else {
        groupExists = true;
        groupId     = groupInfo->gr_gid;
    }


    return groupExists;
}



bool
SpawnProcess::resolveGroupId (const gid_t groupId,
                              string &    groupName)
{
#ifdef _VERBOSE
    Log::Debug ("SpawnProcess::resolveGroupId invoked.");
#endif

    bool groupExists;

    struct group * groupInfo;

    groupInfo = getgrgid (groupId);

    if (!groupInfo) {
        groupExists = false;
    }
    else {
        groupExists = true;
        groupName   = groupInfo->gr_name;
    }


    return groupExists;
}

#endif



