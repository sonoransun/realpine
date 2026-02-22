/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <CorbaUtils.h>
#include <AlpineC.h>
#include <ReadWriteSem.h>


class AlpineCorbaClient
{
  public:

    static bool  initialize (const CORBA::ORB_var &                 orb,
                             const Alpine::DtcpPeerMgmt_var &       dtcpPeerMgmt,
                             const Alpine::DtcpStackMgmt_var &      dtcpStackMgmt,
                             const Alpine::DtcpStackStatus_var &    dtcpStackStatus,
                             const Alpine::AlpineStackMgmt_var &    alpineStackMgmt,
                             const Alpine::AlpineStackStatus_var &  alpineStackStatus,
                             const Alpine::AlpineGroupMgmt_var &    alpineGroupMgmt,
                             const Alpine::AlpinePeerMgmt_var &     alpinePeerMgmt,
                             const Alpine::AlpineQuery_var &        alpineQuery,
                             const Alpine::AlpineModuleMgmt_var &   alpineModuleMgmt);




    // DtcpPeerMgmt Interface member class
    //
    class DtcpPeerMgmt
    {
      public:

        // Add Dtcp Peer
        //
        static void  addDtcpPeer (const char *  ipAddress,
                                  ushort        port,
                                  CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_InvalidAddress,
                          Alpine::e_PeerAlreadyExists,
                          Alpine::e_DtcpStackError));



        // Get Dtcp Peer ID
        //
        static uint  getDtcpPeerId (const char *  ipAddress,
                                    ushort        port,
                                    CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_InvalidAddress,
                          Alpine::e_PeerDoesNotExists,
                          Alpine::e_DtcpStackError));



        // Get Dtcp Peer Status
        //
        static void  getDtcpPeerStatus (uint                            peerId,
                                        Alpine::t_DtcpPeerStatus_out    peerStatus,
                                        CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_PeerDoesNotExists,
                          Alpine::e_DtcpStackError));



        // Get All Dtcp Peer IDs
        //
        static void  getAllDtcpPeerIds (Alpine::t_DtcpPeerIdList_out      peerIdList,
                                        CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_DtcpStackError));



        // Activate Dtcp Peer
        //
        static void  activateDtcpPeer (uint   peerId,
                                       CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_DtcpStackError));



        // Deactivate Dtcp Peer
        //
        static void  deactivateDtcpPeer (uint   peerId,
                                         CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_DtcpStackError));



        // Ping Peer
        //
        static CORBA::Boolean  pingDtcpPeer (uint   peerId,
                                             CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_PeerDoesNotExists,
                          Alpine::e_DtcpStackError));



        // Exclude Host
        //
        static void  excludeHost (const char *  ipAddress,
                                  CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_InvalidAddress,
                          Alpine::e_DtcpStackError));



        // Exclude a subnet / filter
        //
        static void  excludeSubnet (const char *  subnetIpAddress,
                                    const char *  subnetMask,
                                    CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_InvalidAddress,
                          Alpine::e_DtcpStackError));



        // Allow Host
        //
        static void  allowHost (const char *  ipAddress,
                                CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_InvalidAddress,
                          Alpine::e_DtcpStackError));



        // Remove ban on subnet
        //
        static void  allowSubnet (const char *  subnetIpAddress,
                                  CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_InvalidAddress,
                          Alpine::e_DtcpStackError));



        // List All Excluded Hosts
        //
        static void  listExcludedHosts (Alpine::t_IpAddressList_out       ipAddressList,
                                        CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_DtcpStackError));



        // List all excluded subnets
        //
        static void  listExcludedSubnets (Alpine::t_SubnetAddressList_out  subnetAddressList,
                                          CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_DtcpStackError));


      private:

    };




    // DtcpStackMgmt Interface member class
    //
    class DtcpStackMgmt
    {
      public:

        // Perform NAT dicovery and adaptation
        //
        static void  natDiscovery (CORBA::Boolean  isRequired,
                                   CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_DtcpStackError));



        // Check for NAT discovery
        //
        static CORBA::Boolean natDiscoveryRequired (CORBA::Environment &ACE_TRY_ENV = TAO_default_environment())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_DtcpStackError));



        // Outgoing data throttling (_bits_ / sec)  (0 == no limit)
        //
        static void  setDataSendingLimit (uint  bpsLimit,
                                          CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_DtcpStackError));


        static uint  getDataSendingLimit (CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_DtcpStackError));



        // Threading limits
        //
        static void  setStackThreadLimit (uint  threadLimit,
                                          CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_DtcpStackError));


        static uint  getStackThreadLimit (CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_DtcpStackError));



        // Receive buffer limit (number of buffers available for receiving data)
        //
        static void  setReceiveBufferLimit (uint  recvBufferLimit,
                                            CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_InvalidValue,
                          Alpine::e_DtcpStackError));


        static uint  getReceiveBufferLimit (CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_DtcpStackError));



        // Send buffer limit (number of buffers available for sending data)
        //
        static void  setSendBufferLimit (uint  sendBufferLimit,
                                         CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_InvalidValue,
                          Alpine::e_DtcpStackError));


        static uint  getSendBufferLimit (CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_DtcpStackError));


      private:

    };




    // DtcpStackStatus Interface member class
    //
    class DtcpStackStatus
    {
      public:

        // Get transfer buffer stats
        //
        static void  getBufferStats (Alpine::t_DtcpStackBufferStats_out   stats,
                                     CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

             ExThrowSpec ((CORBA::SystemException,
                           Alpine::e_DtcpStackError));


      private:

    };




    // AlpineStackMgmt Interface member class
    //
    class AlpineStackMgmt
    {
      public:

        // Transaction data transfer limits
        //
        static void  setTotalTransferLimit (uint  transferLimit,
                                            CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

             ExThrowSpec ((CORBA::SystemException,
                           Alpine::e_InvalidValue,
                           Alpine::e_AlpineStackError));


        static uint  getTotalTransferLimit (CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_AlpineStackError));



        // Per user concurrent transaction data transfer limits (i.e. num per user)
        //
        static void  setPeerTransferLimit (uint  transferLimit,
                                           CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_InvalidValue,
                          Alpine::e_AlpineStackError));


        static uint  getPeerTransferLimit (CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_AlpineStackError));


      private:

    };




    // AlpineStackStatus Interface member class
    //
    class AlpineStackStatus
    {
      public:

        // Get Alpine transfer stats
        //
        static void  getTransferStats (Alpine::t_AlpineStackTransferStats_out  stats,
                                       CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_AlpineStackError));


      private:

    };




    // AlpineGroupMgmt Interface member class
    //
    class AlpineGroupMgmt
    {
      public:

        // List user defined groups
        //
        static void  getUserGroupList (Alpine::t_AlpineGroupInfoList_out  groupInfoList,
                                       CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_AlpineStackError));



        // Create new group (returns created group ID)
        //
        static uint  createUserGroup (const char *  groupName,
                                      const char *  description,
                                      CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_GroupAlreadyExists,
                          Alpine::e_AlpineStackError));




        // Destroy user group
        //
        static void  destroyUserGroup (uint  groupId,
                                       CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_InvalidGroupId,
                          Alpine::e_AlpineStackError));



        // List groups peer is a member of
        //
        static void  getPeerUserGroupList (uint                               peerId,
                                           Alpine::t_AlpineGroupInfoList_out  groupInfoList,
                                           CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_InvalidPeerId,
                          Alpine::e_AlpineStackError));



        // Add peer to specific group
        //
        static void  addPeerToGroup (uint  peerId,
                                     uint  groupId,
                                     CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_InvalidPeerId,
                          Alpine::e_InvalidGroupId,
                          Alpine::e_AlpineStackError));



        // Remove peer from specific group
        //
        static void  removePeerFromGroup (uint  peerId,
                                          uint  groupId,
                                          CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_InvalidPeerId,
                          Alpine::e_InvalidGroupId,
                          Alpine::e_AlpineStackError));


      private:

    };




    // AlpinePeerMgmt Interface member class
    //
    class AlpinePeerMgmt
    {
      public:

        // List peers with extended information
        //
        static void  getExtendedPeerList (Alpine::t_DtcpPeerIdList_out  peerIdList,
                                          CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_AlpineStackError));



        // Get information for a specific peer
        //
        static void  getPeerInformation (Alpine::t_AlpinePeerInfo_out  peerInfo,
                                         CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_InvalidPeerId,
                          Alpine::e_AlpineStackError));



        // Update information for a specific peer
        //
        static void updatePeerInformation (const Alpine::t_AlpinePeerInfo &  peerInfo,
                                           CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_InvalidPeerId,
                          Alpine::e_AlpineStackError));


      private:

    };




    // AlpineQuery Interface member class
    //
    class AlpineQuery
    {
      public:

        // Get default options for queries
        //
        static void  getDefaultOptions (Alpine::t_QueryOptions_out  options,
                                        CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_AlpineStackError));



        // Set default options for queries
        //
        static void  setDefaultOptions (const Alpine::t_QueryOptions &  options,
                                        CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_AlpineStackError,
                          Alpine::e_InvalidQueryOptions));



        // Create query
        //
        static void  startQuery (const Alpine::t_QueryOptions &  options,
                                 const char *                    queryString,
                                 uint &                          queryId,
                                 CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_AlpineStackError,
                          Alpine::e_InvalidQueryOptions));



        // Check query in progress
        //
        static CORBA::Boolean  inProgress (uint  queryId,
                                           CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_InvalidQueryId));



        // Get query status information
        //
        static void  getQueryStatus (uint                       queryId,
                                     Alpine::t_QueryStatus_out  status,
                                     CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_AlpineStackError,
                          Alpine::e_InvalidQueryId,
                          Alpine::e_InvalidQueryOperation));



        // Pause active query
        //
        static void  pauseQuery (uint  queryId,
                                 CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_AlpineStackError,
                          Alpine::e_InvalidQueryId,
                          Alpine::e_InvalidQueryOperation));



        // Resume paused query
        //
        static void  resumeQuery (uint  queryId,
                                  CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_AlpineStackError,
                          Alpine::e_InvalidQueryId,
                          Alpine::e_InvalidQueryOperation));



        // Cancel query
        //
        static void  cancelQuery (uint  queryId,
                                  CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_AlpineStackError,
                          Alpine::e_InvalidQueryId,
                          Alpine::e_InvalidQueryOperation));



        // Get query results
        //
        static void  getQueryResults (uint        queryId,
                                      Alpine::t_PeerResourcesList_out  results,
                                      CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_AlpineStackError,
                          Alpine::e_InvalidQueryId,
                          Alpine::e_InvalidQueryOperation));



        // Get a list of active query IDs
        //
        static void  getActiveQueryIdList (Alpine::t_QueryIdList_out  queryIdList,
                                           CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_AlpineStackError));



        // Get a list of past query IDs
        //
        static void  getPastQueryIdList (Alpine::t_QueryIdList_out  queryIdList,
                                         CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_AlpineStackError));



      private:

    };



    // AlpineModuleMgmt Interface member class
    //
    class AlpineModuleMgmt 
    {
      public:
   
        // Register a new ALPINE Server module
        //
        static void  registerModule (const char *   libraryPath,
                                     const char *   boostrapSymbol,
                                     uint &         moduleId,
                                     CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_ModuleAlreadyExists,
                          Alpine::e_InvalidLibraryPath,
                          Alpine::e_InvalidLibraryPermissions,
                          Alpine::e_ModuleError));



        // Unregister a module
        //
        static void  unregisterModule (uint  moduleId,
                                       CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_InvalidModuleId,
                          Alpine::e_ModuleActive,
                          Alpine::e_ModuleError));



        // Set configuration values for registered module
        //
        static void  setConfiguration (uint                          moduleId,
                                       const Alpine::t_ConfigData &  configData,
                                       CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_InvalidModuleId,
                          Alpine::e_ModuleError));



        // Get current configuration values for module
        //
        static void  getConfiguration (uint                       moduleId,
                                       Alpine::t_ConfigData_out   configData,
                                       CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_InvalidModuleId,
                          Alpine::e_ModuleError));



        // Get module information
        //
        static void  getModuleInfo (uint                            moduleId,
                                    Alpine::t_AlpineModuleInfo_out  moduleInfo,
                                    CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_InvalidModuleId,
                          Alpine::e_ModuleError));



        // Load a registered module (i.e. activate it)
        //
        static void  loadModule (uint  moduleId,
                                 CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_InvalidModuleId,
                          Alpine::e_ModuleActive,
                          Alpine::e_ModuleError));



        // Unload module (deactivate)
        //
        static void  unloadModule (uint  moduleId,
                                   CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_InvalidModuleId,
                          Alpine::e_ModuleNotActive,
                          Alpine::e_ModuleError));



        // List all active/loaded modules
        //
        static void  listActiveModules (Alpine::t_ModuleIdList_out  idList,
                                        CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_ModuleError));



        // List all modules
        //
        static void  listAllModules (Alpine::t_ModuleIdList_out  idList,
                                     CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_ModuleError));



      private:

    };




  private:

    static CORBA::ORB_var                 orb_s;
    static bool                           initialized_s;

    static Alpine::DtcpPeerMgmt_var       dtcpPeerMgmtRef_s;
    static Alpine::DtcpStackMgmt_var      dtcpStackMgmtRef_s;
    static Alpine::DtcpStackStatus_var    dtcpStackStatusRef_s;
    static Alpine::AlpineStackMgmt_var    alpineStackMgmtRef_s;
    static Alpine::AlpineStackStatus_var  alpineStackStatusRef_s;
    static Alpine::AlpineGroupMgmt_var    alpineGroupMgmtRef_s;
    static Alpine::AlpinePeerMgmt_var     alpinePeerMgmtRef_s;
    static Alpine::AlpineQuery_var        alpineQueryRef_s;
    static Alpine::AlpineModuleMgmt_var   alpineModuleMgmtRef_s;

    static ReadWriteSem                   dataLock_s;


    friend class DtcpPeerMgmt;
    friend class DtcpStackMgmt;
    friend class DtcpStackStatus;
    friend class AlpineStackMgmt;
    friend class AlpineStackStatus;
    friend class AlpineGroupMgmt;
    friend class AlpinePeerMgmt;
    friend class AlpineQuery;
    friend class AlpineModuleMgmt;

};

