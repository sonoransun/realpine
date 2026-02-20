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
#include <sys/types.h>


class UserUtils
{
  public:


    // Public types
    //
    using t_Id = uint;


    // User methods
    //
    static bool  userExists (t_Id  userId);

    static bool  userExists (const string &  userName);

    static bool  getUserName (t_Id      userId,
                              string &  userName);

    static bool  getUserId (const string &  userName,
                            t_Id &          userId);

    static t_Id  getMyRealUserId ();

    static string  getMyRealUserName ();

    static t_Id  getMyEffectiveUserId ();

    static string  getMyEffectiveUserName ();



    // Group methods
    //
    static bool  groupExists (t_Id  groupId);

    static bool  groupExists (const string &  groupName);

    static bool  getGroupName (t_Id      groupId,
                               string &  groupName);

    static bool  getGroupId (const string &  groupName,
                             t_Id &          groupId);

    static t_Id  getMyRealGroupId ();

    static string  getMyRealGroupName ();

    static t_Id  getMyEffectiveGroupId ();

    static string  getMyEffectiveGroupName ();


  private:

};

