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
#include <OptHash.h>
#include <string>
#include <unordered_map>
#include <functional>
#include <Platform.h>



class SpawnProcess
{
  public:

    SpawnProcess ();

    SpawnProcess (const SpawnProcess & copy);

    ~SpawnProcess ();

    const SpawnProcess & operator = (const SpawnProcess & copy);



    // Fork and exec child process...
    //
    bool spawnProcess ();


    // Methods for preparing process
    //
    bool setCommand (const string & command);

    bool setCommand (const string & command,
                     const string & arguments);

    void setArguments (const string & arguments);

    bool setEnvVariable (const string & name,
                         const string & value);


    // Must be root to set user ID or group ID
    //
    bool setChildUser (const string & userName);

    bool setChildUser (uid_t userId);

    bool setChildGroup (const string & groupName);

    bool setChildGroup (gid_t groupId);


    // This will return the id for the most recent child process created.
    // otherwise, 0 returned.
    //
    pid_t getChildProcessId ();


    // Waits for child process to exit, return with exit status.
    //
    int waitOnChild ();


    // Internal types
    //
    using t_EnvironmentMap = std::unordered_map< string,
                      string,
                      OptHash<string>,
                      std::equal_to<string> >;



  private:

    // Child process configuration
    //
    t_EnvironmentMap  environmentMap_;
    string            arguments_;
    string            command_;

    // Data structures for system calls.
    //
    char**    childEnvironment_;
    char**    childArguments_;
    uid_t     execAsUser_;
    gid_t     execAsGroup_;
    pid_t     childProcessId_;


    // Private methods
    //
    void  initialize ();

    void  readEnvVars ();

    bool  verifyCommand (string & command);

    bool  checkExecutePermission (const string & file);

    void  createExecInfo ();

    bool  forkChildProcess ();

    void  execChildProcess ();

    void  parseArgumentString (const string & arguments);

    bool  resolveUserName (const string & userName,
                           uid_t &        userId);

    bool  resolveUserId (const uid_t userId,
                         string &    userName);

    bool  resolveGroupName (const string & groupName,
                            gid_t &        groupId);

    bool  resolveGroupId (const gid_t groupId,
                          string &    groupName);

};


