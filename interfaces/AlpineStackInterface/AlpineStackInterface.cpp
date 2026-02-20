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


#include <AlpineStackInterface.h>
#include <AlpineStack.h>
#include <AlpineQueryMgr.h>
#include <AlpineQuery.h>
#include <AlpineQueryOptions.h>
#include <AlpineQueryOptionData.h>
#include <AlpineQueryStatus.h>
#include <AlpineQueryResults.h>
#include <AlpineGroupMgr.h>
#include <AlpineGroup.h>
#include <AlpineExtensionIndex.h>
#include <AlpineModuleMgr.h>
#include <Log.h>
#include <StringUtils.h>



bool  
AlpineStackInterface::startQuery (const t_QueryOptions &  options,
                                  const string &          queryString,
                                  ulong &                 queryId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineStackInterface::startQuery invoked.");
#endif

    bool status;
    AlpineQueryOptions  queryOptions;


    // Set peer group
    //
    if (options.groupName.size ()) {
        status = AlpineGroupMgr::exists (options.groupName);
        if (!status) {
            Log::Error ("Invalid peer group name in query options passed in call to "
                                 "AlpineStackInterface::startQuery!");
            return false;
        }
 
        queryOptions.setGroup (options.groupName);
    }

    queryOptions.setAutoHalt (options.autoHaltLimit);
    queryOptions.setMaxDescPerPeer (options.peerDescMax);


    // Set optional extensions if provided
    //
    if (options.optionId != 0) {
        status = AlpineExtensionIndex::exists (options.optionId);
        if (!status) {
            Log::Error ("Invalid extension option ID in query options passed in call to "
                                 "AlpineStackInterface::startQuery!");
            return false;
        }

        AlpineQueryOptionData *  queryOptionData;
        status = AlpineExtensionIndex::getQueryOptionExt (options.optionId,
                                                          queryOptionData);
        if (!status) {
            Log::Error ("Error locating query extension data in call to "
                                 "AlpineStackInterface::startQuery!");
            return false;
        }

        status = queryOptionData->readData (options.optionData);
        if (!status) {
            Log::Error ("Error reading query extension data in query options passed in call to "
                                 "AlpineStackInterface::startQuery!");

            delete queryOptionData;
            return false;
        }

        queryOptions.setOptionId (options.optionId);
        queryOptions.setOptionData (queryOptionData);
    }


    // Finally, set actual query string before creating this query
    //
    queryOptions.setQuery (queryString);

    status = AlpineQueryMgr::createQuery (queryOptions, queryId);

    if (!status) {
        Log::Error ("Error creating query in call to AlpineStackInterface::startQuery!");
        return false;
    }


    return true;
}



bool  
AlpineStackInterface::queryInProgress (ulong  queryId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineStackInterface::queryInProgress invoked.");
#endif

    bool status;
    status = AlpineQueryMgr::exists (queryId);

    if (!status) {
        Log::Error ("Invalid query ID: "s + std::to_string (queryId) +
                    " passed in call to AlpineStackInterface::queryInProgress!");
        return false;
    }    

    status = AlpineQueryMgr::inProgress (queryId);

    return status;
}



bool  
AlpineStackInterface::getQueryStatus (ulong            queryId,
                                      t_QueryStatus &  queryStatus)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineStackInterface::getQueryStatus invoked.");
#endif

    bool status;
    status = AlpineQueryMgr::exists (queryId);

    if (!status) {
        Log::Error ("Invalid query ID: "s + std::to_string (queryId) +
                    " passed in call to AlpineStackInterface::getQueryStatus!");
        return false;
    }    

    AlpineQueryStatus  alpineQueryStatus;
    status = AlpineQueryMgr::getQueryStatus (queryId, alpineQueryStatus);

    if (!status) {
        Log::Error ("Error retreiving current status of query "
                             "in call to AlpineStackInterface::getQueryStatus!");
        return false;
    }

    AlpineQueryResults *  alpineQueryResults;
    status = AlpineQueryMgr::getQueryResults (queryId, alpineQueryResults);

    if (!status) {
        Log::Error ("Error retreiving results of query "
                             "in call to AlpineStackInterface::getQueryStatus!");
        return false;
    } 


    // Assign status information into queryStatus
    //
    queryStatus.totalPeers        = alpineQueryStatus.totalPackets ();
    queryStatus.peersQueried      = alpineQueryStatus.numPacketsSent ();
    queryStatus.numPeerResponses  = alpineQueryStatus.numRepliesReceived ();
    queryStatus.totalHits         = 0;
    alpineQueryResults->numResources (queryStatus.totalHits);

    delete alpineQueryResults;


    return true;
}



bool  
AlpineStackInterface::pauseQuery (ulong  queryId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineStackInterface::pauseQuery invoked.");
#endif

    bool status;
    status = AlpineQueryMgr::exists (queryId);

    if (!status) {
        Log::Error ("Invalid query ID: "s + std::to_string (queryId) +
                    " passed in call to AlpineStackInterface::pauseQuery!");
        return false;
    }    

    status = AlpineQueryMgr::pauseQuery (queryId);

    if (!status) {
        Log::Error ("Error pausing query ID: "s + std::to_string (queryId) +
                    " in call to AlpineStackInterface::pauseQuery!");
        return false;
    }


    return true;
}



bool  
AlpineStackInterface::resumeQuery (ulong  queryId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineStackInterface::resumeQuery invoked.");
#endif

    bool status;
    status = AlpineQueryMgr::exists (queryId);

    if (!status) {
        Log::Error ("Invalid query ID: "s + std::to_string (queryId) +
                    " passed in call to AlpineStackInterface::resumeQuery!");
        return false;
    }    

    status = AlpineQueryMgr::resumeQuery (queryId);

    if (!status) {
        Log::Error ("Error resuming query ID: "s + std::to_string (queryId) +
                    " in call to AlpineStackInterface::resumeQuery!");
        return false;
    }


    return true;
}



bool  
AlpineStackInterface::cancelQuery (ulong  queryId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineStackInterface::cancelQuery invoked.");
#endif

    bool status;
    status = AlpineQueryMgr::exists (queryId);

    if (!status) {
        Log::Error ("Invalid query ID: "s + std::to_string (queryId) +
                    " passed in call to AlpineStackInterface::cancelQuery!");
        return false;
    }    

    status = AlpineQueryMgr::cancelQuery (queryId);

    if (!status) {
        Log::Error ("Error canceling query ID: "s + std::to_string (queryId) +
                    " in call to AlpineStackInterface::cancelQuery!");
        return false;
    }


    return true;
}



bool  
AlpineStackInterface::getQueryResults (ulong                   queryId,
                                       t_PeerResourcesIndex &  results)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineStackInterface::getQueryResults invoked.");
#endif

    bool status;
    status = AlpineQueryMgr::exists (queryId);

    if (!status) {
        Log::Error ("Invalid query ID: "s + std::to_string (queryId) +
                    " passed in call to AlpineStackInterface::getQueryResults!");
        return false;
    }    

    AlpineQueryResults *  alpineQueryResults;
    status = AlpineQueryMgr::getQueryResults (queryId, alpineQueryResults);

    if (!status) {
        Log::Error ("Error retreiving results for query ID: "s + std::to_string (queryId) +
                    "in call to AlpineStackInterface::getQueryResults!");
        return false;
    } 

    results.clear ();

    // Assign query result information into results
    //
    AlpineQueryResults::t_PeerIdList       peerIdList;
    AlpineQueryPacket::t_ResourceDescList  alpineResourceDescList;
    AlpineResourceDesc *                   currAlpineDesc;
    AlpineQueryOptionData *                optionData;

    ulong            currId;
    t_ResourceDesc   currDesc;
    t_PeerResources  currResources;


    // Get list of peers who have responded at this point.
    // Use this list to iteratively obtain all resources for each peer and
    // translate into results.
    //
    status = alpineQueryResults->getPeerList (peerIdList);
    if (!status) {
        Log::Error ("Error retreiving peer ID list from query results object "
                             "in call to AlpineStackInterface::getQueryResults!");
        return false;
    }

    for (auto& currId_item : peerIdList) {

        currId = currId_item;

        status = alpineQueryResults->getResourceList (currId, alpineResourceDescList);
        if (!status) {
            Log::Error ("Error retreiving resource desc list from query results object "
                                 "in call to AlpineStackInterface::getQueryResults!");
            return false;
        }

        currResources.peerId = currId;
        currResources.resourceDescList.clear ();

        for (auto& descItem : alpineResourceDescList) {

            currAlpineDesc = &descItem;

            currDesc.resourceId = currAlpineDesc->getMatchId ();
            currDesc.size = currAlpineDesc->getSize ();
            currAlpineDesc->getLocatorList (currDesc.locators);
            currAlpineDesc->getDescription (currDesc.description);
            currDesc.optionId   = currAlpineDesc->getOptionId ();
            currDesc.optionData = "";

            if (currDesc.optionId != 0) {

                // Marshall extended option data into a string (cant use object in results)
                //
                status = currAlpineDesc->getOptionData (optionData);
                if (!status) {
                    Log::Error ("Error retreiving option data from resource desc object "
                                         "in call to AlpineStackInterface::getQueryResults!");
                    return false;
                }

                status = optionData->writeData (currDesc.optionData);
                if (!status) {
                    Log::Error ("Error writing option data to string buffer "
                                         "in call to AlpineStackInterface::getQueryResults!");
                    delete optionData;
                    return false;
                }
 
                delete optionData;
            }

            currResources.resourceDescList.push_back (currDesc);
        }

        // Insert this resource into the results index
        //
        results.emplace (currId, currResources);
    }

    delete alpineQueryResults;


    return true;
}



bool  
AlpineStackInterface::createGroup (const string &  name,
                                   const string &  description,
                                   ulong &         groupId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineStackInterface::createGroup invoked.");
#endif


    return true;
}



bool  
AlpineStackInterface::copyGroup (ulong           copyGroupId,
                                 const string &  name,
                                 const string &  description,
                                 ulong           newGroupId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineStackInterface::copyGroup invoked.");
#endif


    return true;
}



bool  
AlpineStackInterface::deleteGroup (ulong  groupId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineStackInterface::deleteGroup invoked.");
#endif


    return true;
}



bool  
AlpineStackInterface::groupExists (const string &  groupName)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineStackInterface::groupExists (name) invoked.");
#endif


    return true;
}



bool  
AlpineStackInterface::groupExists (ulong  groupId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineStackInterface::groupExists (id) invoked.");
#endif


    return true;
}



bool  
AlpineStackInterface::listGroups (t_IdList &  groupIdList)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineStackInterface::listGroups invoked.");
#endif


    return true;
}



bool  
AlpineStackInterface::getGroupInfo (ulong          groupId,
                                    t_GroupInfo &  groupInfo)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineStackInterface::getGroupInfo invoked.");
#endif


    return true;
}



bool  
AlpineStackInterface::getDefaultGroupInfo (t_GroupInfo &  groupInfo)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineStackInterface::getDefaultGroupInfo invoked.");
#endif


    return true;
}



bool  
AlpineStackInterface::getGroupPeerList (ulong       groupId,
                                        t_IdList &  peerIdList)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineStackInterface::getGroupPeerList invoked.");
#endif


    return true;
}



bool  
AlpineStackInterface::getGroupPeerProfile (ulong            groupId,
                                           ulong            peerId,
                                           t_PeerProfile &  peerProfile)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineStackInterface::getGroupPeerProfile invoked.");
#endif


    return true;
}



bool  
AlpineStackInterface::getDefaultPeerProfile (ulong            peerId,
                                             t_PeerProfile &  peerProfile)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineStackInterface::getDefaultPeerProfile invoked.");
#endif


    return true;
}



bool  
AlpineStackInterface::addPeerToGroup (ulong  groupId,
                                      ulong  peerId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineStackInterface::addPeerToGroup invoked.");
#endif


    return true;
}



bool  
AlpineStackInterface::removePeerFromGroup (ulong  groupId,
                                           ulong  peerId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineStackInterface::removePeerFromGroup invoked.");
#endif


    return true;
}



bool  
AlpineStackInterface::registerModule (const string &  libraryPath,
                                      const string &  boostrapSymbol,
                                      ulong &         moduleId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineStackInterface::registerModule invoked.");
#endif

    // Verify that this module has not been previously defined
    //
    bool status;
    status = AlpineModuleMgr::exists (libraryPath);
    if (!status) {
        Log::Error ("Module library: "s + libraryPath + " is already defined "
                             "in call to AlpineStackInterface::registerModule!");
        return false;
    }

    status = AlpineModuleMgr::registerModule (libraryPath,
                                              boostrapSymbol,
                                              moduleId);
    if (!status) {
        Log::Error ("Error registering module library: "s + libraryPath +
                    " in call to AlpineStackInterface::registerModule!");
        return false;
    }


    return true;
}



bool  
AlpineStackInterface::moduleExists (ulong  moduleId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineStackInterface::moduleExists (id) invoked.");
#endif

    bool status;
    status = AlpineModuleMgr::exists (moduleId);

    return status;
}



bool
AlpineStackInterface::moduleExists (const string &  libraryPath)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineStackInterface::moduleExists (path) invoked.");
#endif

    bool status;
    status = AlpineModuleMgr::exists (libraryPath);

    return status;
}



bool  
AlpineStackInterface::unregisterModule (ulong  moduleId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineStackInterface::unregisterModule invoked.");
#endif

    bool status;
    status = AlpineModuleMgr::exists (moduleId);

    if (!status) {
        Log::Error ("Invalid module ID: "s + std::to_string (moduleId) +
                    " passed in call to AlpineStackInterface::unregisterModule!");
        return false;
    }

    status = AlpineModuleMgr::unregisterModule (moduleId);

    if (!status) {
        Log::Error ("Unregister module ID: "s + std::to_string (moduleId) +
                    " failed in call to AlpineStackInterface::unregisterModule!");
        return false;
    }


    return true;
}



bool  
AlpineStackInterface::setModuleConfiguration (ulong         moduleId,
                                              ConfigData &  configData)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineStackInterface::setModuleConfiguration invoked.");
#endif

    bool status;
    status = AlpineModuleMgr::exists (moduleId);

    if (!status) {
        Log::Error ("Invalid module ID: "s + std::to_string (moduleId) +
                    " passed in call to AlpineStackInterface::setModuleConfiguration!");
        return false;
    }

    status = AlpineModuleMgr::setConfiguration (moduleId, configData);

    if (!status) {
        Log::Error ("Error setting configuration for module ID: "s + std::to_string (moduleId) +
                    " in call to AlpineStackInterface::setModuleConfiguration!");
        return false;
    }


    return true;
}



bool  
AlpineStackInterface::getModuleConfiguration (ulong         moduleId,
                                              ConfigData &  configData)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineStackInterface::getModuleConfiguration invoked.");
#endif

    bool status;
    status = AlpineModuleMgr::exists (moduleId);

    if (!status) {
        Log::Error ("Invalid module ID: "s + std::to_string (moduleId) +
                    " passed in call to AlpineStackInterface::getModuleConfiguration!");
        return false;
    }

    status = AlpineModuleMgr::getConfiguration (moduleId, configData);

    if (!status) {
        Log::Error ("Error getting configuration for module ID: "s + std::to_string (moduleId) +
                    " in call to AlpineStackInterface::getModuleConfiguration!");
        return false;
    }


    return true;
}



bool  
AlpineStackInterface::getModuleInfo (ulong           moduleId,
                                     t_ModuleInfo &  moduleInfo)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineStackInterface::getModuleInfo invoked.");
#endif

    bool status;
    status = AlpineModuleMgr::exists (moduleId);

    if (!status) {
        Log::Error ("Invalid module ID: "s + std::to_string (moduleId) +
                    " passed in call to AlpineStackInterface::getModuleInfo!");
        return false;
    }

    AlpineModuleMgr::t_ModuleInfo  alpineModuleInfo;
    status = AlpineModuleMgr::getModuleInfo (moduleId, alpineModuleInfo);

    if (!status) {
        Log::Error ("Error getting information for module ID: "s + std::to_string (moduleId) +
                    " in call to AlpineStackInterface::getModuleInfo!");
        return false;
    }


    // Assign information into moduleInfo
    //
    moduleInfo.moduleId         = moduleId;
    moduleInfo.moduleName       = alpineModuleInfo.moduleName;
    moduleInfo.description      = alpineModuleInfo.description;
    moduleInfo.version          = alpineModuleInfo.version;
    moduleInfo.libraryPath      = alpineModuleInfo.libraryPath;
    moduleInfo.bootstrapSymbol  = alpineModuleInfo.bootstrapSymbol;
    moduleInfo.activeTime       = alpineModuleInfo.activeTime;


    return true;
}



bool  
AlpineStackInterface::loadModule (ulong  moduleId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineStackInterface::loadModule invoked.");
#endif

    bool status;
    status = AlpineModuleMgr::exists (moduleId);

    if (!status) {
        Log::Error ("Invalid module ID: "s + std::to_string (moduleId) +
                    " passed in call to AlpineStackInterface::loadModule!");
        return false;
    }

    status = AlpineModuleMgr::loadModule (moduleId);

    if (!status) {
        Log::Error ("Attempted load for module ID: "s + std::to_string (moduleId) +
                    " failed in call to AlpineStackInterface::loadModule!");
        return false;
    }


    return true;
}



bool  
AlpineStackInterface::moduleIsActive (ulong  moduleId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineStackInterface::moduleIsActive invoked.");
#endif

    bool status;
    status = AlpineModuleMgr::exists (moduleId);

    if (!status) {
        Log::Error ("Invalid module ID: "s + std::to_string (moduleId) +
                    " passed in call to AlpineStackInterface::moduleIsActive!");
        return false;
    }

    status = AlpineModuleMgr::isActive (moduleId);

    return status;
}



bool  
AlpineStackInterface::unloadModule (ulong  moduleId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineStackInterface::unloadModule invoked.");
#endif

    bool status;
    status = AlpineModuleMgr::exists (moduleId);

    if (!status) {
        Log::Error ("Invalid module ID: "s + std::to_string (moduleId) +
                    " passed in call to AlpineStackInterface::unloadModule!");
        return false;
    }

    status = AlpineModuleMgr::unloadModule (moduleId);

    if (!status) {
        Log::Error ("Attempted unload for module ID: "s + std::to_string (moduleId) +
                    " failed in call to AlpineStackInterface::unloadModule!");
        return false;
    }


    return true;
}



bool  
AlpineStackInterface::listActiveModules (t_IdList &  idList)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineStackInterface::listActiveModules invoked.");
#endif

    bool status;
    status = AlpineModuleMgr::listActiveModules (idList);

    if (!status) {
        Log::Error ("Unable to get active module ID list "
                             "in call to AlpineStackInterface::listActiveModules!");
        return false;
    }


    return true;
}



bool  
AlpineStackInterface::listAllModules (t_IdList &  idList)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineStackInterface::listAllModules invoked.");
#endif

    bool status;
    status = AlpineModuleMgr::listAllModules (idList);

    if (!status) {
        Log::Error ("Unable to get module ID list "
                             "in call to AlpineStackInterface::listAllModules!");
        return false;
    }


    return true;
}



