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


#include <AlpineGroupMgmtIntf.h>
#include <Log.h>
#include <StringUtils.h>



bool  
AlpineGroupMgmtIntf::getUserGroupList (t_AlpineGroupInfoList & groupList)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineGroupMgmtIntf::getUserGroupList invoked.");
#endif

    ExNewEnv;

    Alpine::t_AlpineGroupInfoList_var  corbaGroupList;

    ExTry {

        AlpineCorbaClient::AlpineGroupMgmt::getUserGroupList (corbaGroupList.out(), ExTryEnv);

        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {
        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " from AlpineCorbaClient::AlpineGroupMgmt::getUserGroupList in call to "
                             "AlpineGroupMgmtIntf::getUserGroupList.");
        return false;
    }
    ExCatchAny {
        Log::Error ("Caught unknown exception from "
                             "AlpineCorbaClient::AlpineGroupMgmt::getUserGroupList in call to "
                             "AlpineGroupMgmtIntf::getUserGroupList.");
        return false;
    }
    ExEndTry;
    ExCheck;

    groupList.clear ();

    uint  i;
    t_AlpineGroupInfo  currGroupInfo; 

    for (i = 0; i < corbaGroupList->length(); i++) {
        currGroupInfo.name         = corbaGroupList[i].name;
        currGroupInfo.id           = corbaGroupList[i].id;
        currGroupInfo.description  = corbaGroupList[i].description;
        currGroupInfo.open         = corbaGroupList[i].open;

        groupList.push_back (currGroupInfo);
    }


    return true;
}



bool  
AlpineGroupMgmtIntf::createUserGroup (const string &  groupName,
                                      const string &  description,
                                      ulong &         groupId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineGroupMgmtIntf::createUserGroup invoked.");
#endif

    ExNewEnv;

    ExTry {

        groupId = AlpineCorbaClient::AlpineGroupMgmt::createUserGroup (groupName.c_str(),
                                                                       description.c_str(),
                                                                       ExTryEnv);

        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {
        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " from AlpineCorbaClient::AlpineGroupMgmt::createUserGroup in call to "
                             "AlpineGroupMgmtIntf::createUserGroup.");
        return false;
    }
    ExCatchAny {
        Log::Error ("Caught unknown exception from "
                             "AlpineCorbaClient::AlpineGroupMgmt::createUserGroup in call to "
                             "AlpineGroupMgmtIntf::createUserGroup.");
        return false;
    }
    ExEndTry;
    ExCheck;


    return true;
}



bool  
AlpineGroupMgmtIntf::destroyUserGroup (ulong groupId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineGroupMgmtIntf::destroyUserGroup invoked.");
#endif

    ExNewEnv;

    ExTry {

        AlpineCorbaClient::AlpineGroupMgmt::destroyUserGroup (groupId, ExTryEnv);

        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {
        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " from AlpineCorbaClient::AlpineGroupMgmt::destroyUserGroup in call to "
                             "AlpineGroupMgmtIntf::destroyUserGroup.");
        return false;
    }
    ExCatchAny {
        Log::Error ("Caught unknown exception from "
                             "AlpineCorbaClient::AlpineGroupMgmt::destroyUserGroup in call to "
                             "AlpineGroupMgmtIntf::destroyUserGroup.");
        return false;
    }
    ExEndTry;
    ExCheck;


    return true;
}



bool  
AlpineGroupMgmtIntf::getPeerUserGroupList (ulong                    peerId,
                                           t_AlpineGroupInfoList &  groupList)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineGroupMgmtIntf::getPeerUserGroupList invoked.");
#endif

    ExNewEnv;

    Alpine::t_AlpineGroupInfoList_var  corbaGroupList;

    ExTry {

        AlpineCorbaClient::AlpineGroupMgmt::getPeerUserGroupList (peerId,
                                                                  corbaGroupList.out(), 
                                                                  ExTryEnv);

        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {
        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " from AlpineCorbaClient::AlpineGroupMgmt::getPeerUserGroupList in call to "
                             "AlpineGroupMgmtIntf::getPeerUserGroupList.");
        return false;
    }
    ExCatchAny {
        Log::Error ("Caught unknown exception from "
                             "AlpineCorbaClient::AlpineGroupMgmt::getPeerUserGroupList in call to "
                             "AlpineGroupMgmtIntf::getPeerUserGroupList.");
        return false;
    }
    ExEndTry;
    ExCheck;

    groupList.clear ();

    uint i;
    t_AlpineGroupInfo  currGroup;

    for (i = 0; i < corbaGroupList->length(); i++) {
        currGroup.name         = corbaGroupList[i].name;
        currGroup.id           = corbaGroupList[i].id;
        currGroup.description  = corbaGroupList[i].description;
        currGroup.open         = corbaGroupList[i].open;

        groupList.push_back (currGroup);
    }


    return true;
}



bool  
AlpineGroupMgmtIntf::addPeerToGroup (ulong  peerId,
                                     ulong  groupId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineGroupMgmtIntf::addPeerToGroup invoked.");
#endif

    ExNewEnv;

    ExTry {

        AlpineCorbaClient::AlpineGroupMgmt::addPeerToGroup (peerId, groupId, ExTryEnv);

        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {
        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " from AlpineCorbaClient::AlpineGroupMgmt::addPeerToGroup in call to "
                             "AlpineGroupMgmtIntf::addPeerToGroup.");
        return false;
    }
    ExCatchAny {
        Log::Error ("Caught unknown exception from "
                             "AlpineCorbaClient::AlpineGroupMgmt::addPeerToGroup in call to "
                             "AlpineGroupMgmtIntf::addPeerToGroup.");
        return false;
    }
    ExEndTry;
    ExCheck;


    return true;
}



bool  
AlpineGroupMgmtIntf::removePeerFromGroup (ulong  peerId,
                                          ulong  groupId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineGroupMgmtIntf::removePeerFromGroup invoked.");
#endif

    ExNewEnv;

    ExTry {

        AlpineCorbaClient::AlpineGroupMgmt::removePeerFromGroup (peerId, groupId, ExTryEnv);

        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {
        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " from AlpineCorbaClient::AlpineGroupMgmt::removePeerFromGroup in call to "
                             "AlpineGroupMgmtIntf::removePeerFromGroup.");
        return false;
    }
    ExCatchAny {
        Log::Error ("Caught unknown exception from "
                             "AlpineCorbaClient::AlpineGroupMgmt::removePeerFromGroup in call to "
                             "AlpineGroupMgmtIntf::removePeerFromGroup.");
        return false;
    }
    ExEndTry;
    ExCheck;


    return true;
}



