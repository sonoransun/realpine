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
#include <ConfigData.h>
#include <string>
#include <vector>
#include <OptHash.h>


class AlpineStackInterface
{
  public:


    // Public types
    //
    struct t_QueryOptions {
        string    groupName;
        ulong     autoHaltLimit;
        bool      autoDownload;
        ulong     peerDescMax;
        ulong     optionId;
        string    optionData;
    };


    struct t_QueryStatus {
        ulong   totalPeers;
        ulong   peersQueried;
        ulong   numPeerResponses;
        ulong   totalHits;
    };

    using t_LocatorList = vector<string>;

    struct t_ResourceDesc {
        ulong           resourceId;
        ulong           size;
        t_LocatorList   locators;
        string          description;
        ulong           optionId;
        string          optionData;
    };

    using t_ResourceDescList = vector<t_ResourceDesc>;


    struct t_PeerResources {
        unsigned long       peerId;
        t_ResourceDescList  resourceDescList;
    };

    using t_PeerResourcesIndex = std::unordered_map < ulong, // Peer ID
                       t_PeerResources,
                       OptHash<ulong>,
                       equal_to<ulong> >;

    using t_PeerResourcesIndexPair = std::pair <ulong, t_PeerResources>;


    struct t_GroupInfo {
        ulong  groupId;
        string groupName;
        string description;
        ulong  numPeers;
        ulong  totalQueries;
        ulong  totalResponses;
    };

    struct t_PeerProfile {
        ulong   peerId;
        short   relativeQuality;
        ulong   totalQueries;
        ulong   totalResponses;
    };


    struct t_ModuleInfo {
        ulong    moduleId;
        string   moduleName;
        string   description;
        string   version;
        string   libraryPath;
        string   bootstrapSymbol;
        ulong    activeTime;
    };

    using t_IdList = vector<ulong>;




    // Supported interface operations
    //

    // Query operations
    //
    static bool  startQuery (const t_QueryOptions &  options,
                             const string &          queryString,
                             ulong &                 queryId);

    static bool  queryInProgress (ulong  queryId);

    static bool  getQueryStatus (ulong            queryId,
                                 t_QueryStatus &  queryStatus);

    static bool  pauseQuery (ulong  queryId);

    static bool  resumeQuery (ulong  queryId);

    static bool  cancelQuery (ulong  queryId);

    static bool  getQueryResults (ulong                   queryId,
                                  t_PeerResourcesIndex &  results);


    // Group operations
    //
    static bool  createGroup (const string &  name,
                              const string &  description,
                              ulong &         groupId);

    static bool  copyGroup (ulong           copyGroupId,
                            const string &  name,
                            const string &  description,
                            ulong           newGroupId);

    static bool  deleteGroup (ulong  groupId);

    static bool  groupExists (const string &  groupName);

    static bool  groupExists (ulong  groupId);

    static bool  listGroups (t_IdList &  groupIdList);

    static bool  getGroupInfo (ulong          groupId,
                               t_GroupInfo &  groupInfo);

    static bool  getDefaultGroupInfo (t_GroupInfo &  groupInfo);

    static bool  getGroupPeerList (ulong       groupId,
                                   t_IdList &  peerIdList);

    static bool  getGroupPeerProfile (ulong            groupId,
                                      ulong            peerId,
                                      t_PeerProfile &  peerProfile);

    static bool  getDefaultPeerProfile (ulong            peerId,
                                        t_PeerProfile &  peerProfile);

    static bool  addPeerToGroup (ulong  groupId,
                                 ulong  peerId);

    static bool  removePeerFromGroup (ulong  groupId,
                                      ulong  peerId);


    // Module operations
    //
    static bool  registerModule (const string &  libraryPath,
                                 const string &  boostrapSymbol,
                                 ulong &         moduleId);

    static bool  moduleExists (ulong  moduleId);

    static bool  moduleExists (const string &  libraryPath);

    static bool  unregisterModule (ulong  moduleId);

    static bool  setModuleConfiguration (ulong         moduleId,
                                         ConfigData &  configData);

    static bool  getModuleConfiguration (ulong         moduleId,
                                         ConfigData &  configData);

    static bool  getModuleInfo (ulong           moduleId,
                                t_ModuleInfo &  moduleInfo);

    static bool  loadModule (ulong  moduleId);

    static bool  moduleIsActive (ulong  moduleId);

    static bool  unloadModule (ulong  moduleId);

    static bool  listActiveModules (t_IdList &  idList);

    static bool  listAllModules (t_IdList &  idList);



  private:

};

