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


#include <AlpineQueryIntf.h>
#include <AlpineCorbaClient.h>
#include <Log.h>
#include <StringUtils.h>



bool  
AlpineQueryIntf::getDefaultOptions (t_QueryOptions &  options)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryIntf::getDefaultOptions invoked.");
#endif

    ExNewEnv;

    Alpine::t_QueryOptions_var  corbaOptions;

    ExTry {
    
        AlpineCorbaClient::AlpineQuery::getDefaultOptions (corbaOptions.out(), ExTryEnv);

        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {
        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " from AlpineCorbaClient::AlpineQuery::getDefaultOptions in call to "
                             "AlpineQueryIntf::getDefaultOptions.");
        return false;
    }
    ExCatchAny {
        Log::Error ("Caught unknown exception from "
                             "AlpineCorbaClient::AlpineQuery::getDefaultOptions in call to "
                             "AlpineQueryIntf::getDefaultOptions.");
        return false;
    }
    ExEndTry;
    ExCheck;

    options.groupName      = corbaOptions->groupName;
    options.autoHaltLimit  = corbaOptions->autoHaltLimit;
    options.autoDownload   = corbaOptions->autoDownload;
    options.peerDescMax    = corbaOptions->peerDescMax;
    options.optionId       = corbaOptions->optionId;
    options.optionData     = corbaOptions->optionData;


    return true;
}



bool  
AlpineQueryIntf::setDefaultOptions (const t_QueryOptions &  options)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryIntf::setDefaultOptions invoked.");
#endif

    ExNewEnv;

    Alpine::t_QueryOptions  corbaOptions;
    corbaOptions.groupName      = CORBA::string_dup (options.groupName.c_str());
    corbaOptions.autoHaltLimit  = options.autoHaltLimit;
    corbaOptions.autoDownload   = options.autoDownload;
    corbaOptions.peerDescMax    = options.peerDescMax;
    corbaOptions.optionId       = options.optionId;
    corbaOptions.optionData     = CORBA::string_dup (options.optionData.c_str());


    ExTry {
    
        AlpineCorbaClient::AlpineQuery::setDefaultOptions (corbaOptions, ExTryEnv);

        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {
        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " from AlpineCorbaClient::AlpineQuery::setDefaultOptions in call to "
                             "AlpineQueryIntf::setDefaultOptions.");
        return false;
    }
    ExCatchAny {
        Log::Error ("Caught unknown exception from "
                             "AlpineCorbaClient::AlpineQuery::setDefaultOptions in call to "
                             "AlpineQueryIntf::setDefaultOptions.");
        return false;
    }
    ExEndTry;
    ExCheck;


    return true;
}



bool  
AlpineQueryIntf::startQuery (const t_QueryOptions &  options,
                             const string &          queryString,
                             ulong &                 queryId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryIntf::startQuery invoked.");
#endif

    ExNewEnv;

    Alpine::t_QueryOptions  corbaOptions;
    uint                    corbaId;
    corbaOptions.groupName      = CORBA::string_dup (options.groupName.c_str());
    corbaOptions.autoHaltLimit  = options.autoHaltLimit;
    corbaOptions.autoDownload   = options.autoDownload;
    corbaOptions.peerDescMax    = options.peerDescMax;
    corbaOptions.optionId       = options.optionId;
    corbaOptions.optionData     = CORBA::string_dup (options.optionData.c_str());


    ExTry {

        AlpineCorbaClient::AlpineQuery::startQuery (corbaOptions,
                                                    queryString.c_str(),
                                                    corbaId,
                                                    ExTryEnv);

        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {
        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " from AlpineCorbaClient::AlpineQuery::startQuery in call to "
                             "AlpineQueryIntf::startQuery.");
        return false;
    }
    ExCatchAny {
        Log::Error ("Caught unknown exception from "
                             "AlpineCorbaClient::AlpineQuery::startQuery in call to "
                             "AlpineQueryIntf::startQuery.");
        return false;
    }
    ExEndTry;
    ExCheck;

    queryId = corbaId;


    return true;
}



bool  
AlpineQueryIntf::inProgress (ulong  queryId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryIntf::inProgress invoked.");
#endif

    ExNewEnv;

    CORBA::Boolean  retVal;

    ExTry {

        retVal = AlpineCorbaClient::AlpineQuery::inProgress (queryId, ExTryEnv);

        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {
        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " from AlpineCorbaClient::AlpineQuery::inProgress in call to "
                             "AlpineQueryIntf::inProgress.");
        return false;
    }
    ExCatchAny {
        Log::Error ("Caught unknown exception from "
                             "AlpineCorbaClient::AlpineQuery::inProgress in call to "
                             "AlpineQueryIntf::inProgress.");
        return false;
    }
    ExEndTry;
    ExCheck;

    if (retVal == 0) {
        return false;
    }


    return true;
}



bool  
AlpineQueryIntf::getQueryStatus (ulong            queryId,
                                 t_QueryStatus &  status)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryIntf::getQueryStatus invoked.");
#endif

    ExNewEnv;

    Alpine::t_QueryStatus  corbaStatus;

    ExTry {

        AlpineCorbaClient::AlpineQuery::getQueryStatus (queryId, corbaStatus, ExTryEnv);

        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {
        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " from AlpineCorbaClient::AlpineQuery::getQueryStatus in call to "
                             "AlpineQueryIntf::getQueryStatus.");
        return false;
    }
    ExCatchAny {
        Log::Error ("Caught unknown exception from "
                             "AlpineCorbaClient::AlpineQuery::getQueryStatus in call to "
                             "AlpineQueryIntf::getQueryStatus.");
        return false;
    }
    ExEndTry;
    ExCheck;

    status.totalPeers        = corbaStatus.totalPeers;
    status.peersQueried      = corbaStatus.peersQueried;
    status.numPeerResponses  = corbaStatus.numPeerResponses;
    status.totalHits         = corbaStatus.totalHits;


    return true;
}



bool  
AlpineQueryIntf::pauseQuery (ulong  queryId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryIntf::pauseQuery invoked.");
#endif

    ExNewEnv;

    ExTry {

        AlpineCorbaClient::AlpineQuery::pauseQuery (queryId, ExTryEnv);

        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {
        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " from AlpineCorbaClient::AlpineQuery::pauseQuery in call to "
                             "AlpineQueryIntf::pauseQuery.");
        return false;
    }
    ExCatchAny {
        Log::Error ("Caught unknown exception from "
                             "AlpineCorbaClient::AlpineQuery::pauseQuery in call to "
                             "AlpineQueryIntf::pauseQuery.");
        return false;
    }
    ExEndTry;
    ExCheck;


    return true;
}



bool  
AlpineQueryIntf::resumeQuery (ulong  queryId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryIntf::resumeQuery invoked.");
#endif

    ExNewEnv;

    ExTry {

        AlpineCorbaClient::AlpineQuery::resumeQuery (queryId, ExTryEnv);

        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {
        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " from AlpineCorbaClient::AlpineQuery::resumeQuery in call to "
                             "AlpineQueryIntf::resumeQuery.");
        return false;
    }
    ExCatchAny {
        Log::Error ("Caught unknown exception from "
                             "AlpineCorbaClient::AlpineQuery::resumeQuery in call to "
                             "AlpineQueryIntf::resumeQuery.");
        return false;
    }
    ExEndTry;
    ExCheck;


    return true;
}



bool  
AlpineQueryIntf::cancelQuery (ulong  queryId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryIntf::cancelQuery invoked.");
#endif

    ExNewEnv;

    ExTry {

        AlpineCorbaClient::AlpineQuery::cancelQuery (queryId, ExTryEnv);

        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {
        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " from AlpineCorbaClient::AlpineQuery::cancelQuery in call to "
                             "AlpineQueryIntf::cancelQuery.");
        return false;
    }
    ExCatchAny {
        Log::Error ("Caught unknown exception from "
                             "AlpineCorbaClient::AlpineQuery::cancelQuery in call to "
                             "AlpineQueryIntf::cancelQuery.");
        return false;
    }
    ExEndTry;
    ExCheck;


    return true;
}



bool  
AlpineQueryIntf::getQueryResults (ulong                   queryId,
                                  t_PeerResourcesIndex &  results)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryIntf::getQueryResults invoked.");
#endif

    ExNewEnv;

    Alpine::t_PeerResourcesList_var  corbaResults;

    ExTry {

        AlpineCorbaClient::AlpineQuery::getQueryResults (queryId, corbaResults.out(), ExTryEnv);

        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {
        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " from AlpineCorbaClient::AlpineQuery::getQueryResults in call to "
                             "AlpineQueryIntf::getQueryResults.");
        return false;
    }
    ExCatchAny {
        Log::Error ("Caught unknown exception from "
                             "AlpineCorbaClient::AlpineQuery::getQueryResults in call to "
                             "AlpineQueryIntf::getQueryResults.");
        return false;
    }
    ExEndTry;
    ExCheck;

    results.clear ();

    uint i;
    uint j;
    ulong  currPeerId;
    t_ResourceDesc   currDesc;
    t_PeerResources  newResources;

    for (i = 0; i < corbaResults->length (); i++) {
        currPeerId = corbaResults[i].peerId;
        newResources.peerId = currPeerId;
        newResources.resourceDescList.clear ();

        for (j = 0; j < corbaResults[i].resourceDescList.length (); j++) {
            // Assign values in the current resource description
            //
            currDesc.resourceId   = corbaResults[i].resourceDescList[j].resourceId;
            currDesc.size         = corbaResults[i].resourceDescList[j].size;
            currDesc.locator      = corbaResults[i].resourceDescList[j].locator;
            currDesc.description  = corbaResults[i].resourceDescList[j].description;
            currDesc.optionId     = corbaResults[i].resourceDescList[j].optionId;
            currDesc.optionData   = corbaResults[i].resourceDescList[j].optionData;

            newResources.resourceDescList.push_back (currDesc);
        }

        // index this peers resource list
        //
        results.emplace (currPeerId, newResources);
    }


    return true;
}



bool  
AlpineQueryIntf::getActiveQueryIdList (t_QueryIdList &  queryIdList)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryIntf::getActiveQueryIdList invoked.");
#endif

    ExNewEnv;

    Alpine::t_QueryIdList_var  corbaIdList;

    ExTry {

        AlpineCorbaClient::AlpineQuery::getActiveQueryIdList (corbaIdList.out(), ExTryEnv);

        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {
        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " from AlpineCorbaClient::AlpineQuery::getActiveQueryIdList in call to "
                             "AlpineQueryIntf::getActiveQueryIdList.");
        return false;
    }
    ExCatchAny {
        Log::Error ("Caught unknown exception from "
                             "AlpineCorbaClient::AlpineQuery::getActiveQueryIdList in call to "
                             "AlpineQueryIntf::getActiveQueryIdList.");
        return false;
    }
    ExEndTry;
    ExCheck;

    queryIdList.clear ();

    uint i;
    for (i = 0; i < corbaIdList->length(); i++) {
        queryIdList.push_back (corbaIdList[i]);
    }


    return true;
}



bool  
AlpineQueryIntf::getPastQueryIdList (t_QueryIdList &  queryIdList)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryIntf::getPastQueryIdList invoked.");
#endif

    ExNewEnv;

    Alpine::t_QueryIdList_var  corbaIdList;

    ExTry {

        AlpineCorbaClient::AlpineQuery::getPastQueryIdList (corbaIdList.out(), ExTryEnv);

        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {
        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " from AlpineCorbaClient::AlpineQuery::getPastQueryIdList in call to "
                             "AlpineQueryIntf::getPastQueryIdList.");
        return false;
    }
    ExCatchAny {
        Log::Error ("Caught unknown exception from "
                             "AlpineCorbaClient::AlpineQuery::getPastQueryIdList in call to "
                             "AlpineQueryIntf::getPastQueryIdList.");
        return false;
    }
    ExEndTry;
    ExCheck;

    queryIdList.clear ();

    uint i;
    for (i = 0; i < corbaIdList->length(); i++) {
        queryIdList.push_back (corbaIdList[i]);
    }


    return true;
}



