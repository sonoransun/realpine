/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <AlpineS.h>
#include <CorbaUtils.h>


class AlpineCorba_impl
{
  public:

    AlpineCorba_impl (const CORBA::ORB_var &          orb,
                      const PortableServer::POA_var & poa);

    virtual ~AlpineCorba_impl ();



    bool  initialize ();

    bool  getDtcpPeerMgmt (Alpine::DtcpPeerMgmt_var &  dtcpPeerMgmt);

    bool  getDtcpStackMgmt (Alpine::DtcpStackMgmt_var &  dtcpStackMgmt);

    bool  getDtcpStackStatus (Alpine::DtcpStackStatus_var &  dtcpStackStatus);

    bool  getAlpineStackMgmt (Alpine::AlpineStackMgmt_var &  alpineStackMgmt);

    bool  getAlpineStackStatus (Alpine::AlpineStackStatus_var &  alpineStackStatus);

    bool  getAlpineGroupMgmt (Alpine::AlpineGroupMgmt_var &  alpineGroupMgmt);

    bool  getAlpinePeerMgmt (Alpine::AlpinePeerMgmt_var &  alpinePeerMgmt);

    bool  getAlpineQuery (Alpine::AlpineQuery_var &  alpineQuery);

    bool  getAlpineModuleMgmt (Alpine::AlpineModuleMgmt_var &  alpineModuleMgmt);




    // DtcpPeerMgmt Interface member class
    //
    class DtcpPeerMgmt : public POA_Alpine::DtcpPeerMgmt
    {
      public:

        DtcpPeerMgmt (const CORBA::ORB_var &          orb,
                      const PortableServer::POA_var & poa,
                      AlpineCorba_impl *              implParent);

        virtual ~DtcpPeerMgmt ();



        // Add Dtcp Peer
        //
        virtual void  addDtcpPeer (const char *  ipAddress,
                                   ushort        port,
                                   CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_InvalidAddress,
                          Alpine::e_PeerAlreadyExists,
                          Alpine::e_DtcpStackError));



        // Get Dtcp Peer ID
        //
        virtual uint  getDtcpPeerId (const char *  ipAddress,
                                     ushort        port,
                                     CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_InvalidAddress,
                          Alpine::e_PeerDoesNotExists,
                          Alpine::e_DtcpStackError));



        // Get Dtcp Peer Status
        //
        virtual void  getDtcpPeerStatus (uint                            peerId,
                                         Alpine::t_DtcpPeerStatus_out    peerStatus,
                                         CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_PeerDoesNotExists,
                          Alpine::e_DtcpStackError));



        // Get All Dtcp Peer IDs
        //
        virtual void  getAllDtcpPeerIds (Alpine::t_DtcpPeerIdList_out      peerIdList,
                                         CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_DtcpStackError));



        // Activate Dtcp Peer
        //
        virtual void  activateDtcpPeer (uint   peerId,
                                        CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_DtcpStackError));



        // Deactivate Dtcp Peer
        //
        virtual void  deactivateDtcpPeer (uint   peerId,
                                          CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_DtcpStackError));



        // Ping Peer
        //
        virtual CORBA::Boolean  pingDtcpPeer (uint   peerId,
                                              CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_PeerDoesNotExists,
                          Alpine::e_DtcpStackError));



        // Exclude Host
        //
        virtual void  excludeHost (const char *  ipAddress,
                                   CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_InvalidAddress,
                          Alpine::e_DtcpStackError));



        // Exclude a subnet / filter
        //
        virtual void  excludeSubnet (const char *  subnetIpAddress,
                                     const char *  subnetMask,
                                     CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_InvalidAddress,
                          Alpine::e_DtcpStackError));



        // Allow Host
        //
        virtual void  allowHost (const char *  ipAddress,
                                 CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_InvalidAddress,
                          Alpine::e_DtcpStackError));



        // Remove ban on subnet
        //
        virtual void  allowSubnet (const char *  subnetIpAddress,
                                   CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_InvalidAddress,
                          Alpine::e_DtcpStackError));



        // List All Excluded Hosts
        //
        virtual void  listExcludedHosts (Alpine::t_IpAddressList_out       ipAddressList,
                                         CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_DtcpStackError));



        // List all excluded subnets
        //
        virtual void  listExcludedSubnets (Alpine::t_SubnetAddressList_out  subnetAddressList,
                                           CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_DtcpStackError));



      private:

        CORBA::ORB_var            orb_;
        PortableServer::POA_var   poa_;
        AlpineCorba_impl *        implParent_;

    };




    // DtcpStackMgmt Interface member class
    //
    class DtcpStackMgmt : public POA_Alpine::DtcpStackMgmt
    {
      public:

        DtcpStackMgmt (const CORBA::ORB_var &          orb,
                       const PortableServer::POA_var & poa,
                       AlpineCorba_impl *              implParent);

        virtual ~DtcpStackMgmt ();



        // Perform NAT dicovery and adaptation
        //
        virtual void  natDiscovery (CORBA::Boolean  isRequired,
                                    CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_DtcpStackError));



        // Check for NAT discovery
        //
        virtual CORBA::Boolean natDiscoveryRequired (CORBA::Environment &ACE_TRY_ENV = TAO_default_environment())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_DtcpStackError));



        // Outgoing data throttling (_bits_ / sec)  (0 == no limit)
        //
        virtual void  setDataSendingLimit (uint  bpsLimit,
                                           CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_DtcpStackError));


        virtual uint  getDataSendingLimit (CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_DtcpStackError));



        // Threading limits
        //
        virtual void  setStackThreadLimit (uint  threadLimit,
                                           CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_DtcpStackError));


        virtual uint  getStackThreadLimit (CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_DtcpStackError));



        // Receive buffer limit (number of buffers available for receiving data)
        //
        virtual void  setReceiveBufferLimit (uint  recvBufferLimit,
                                             CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_InvalidValue,
                          Alpine::e_DtcpStackError));


        virtual uint  getReceiveBufferLimit (CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_DtcpStackError));



        // Send buffer limit (number of buffers available for sending data)
        //
        virtual void  setSendBufferLimit (uint  sendBufferLimit,
                                          CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_InvalidValue,
                          Alpine::e_DtcpStackError));


        virtual uint  getSendBufferLimit (CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_DtcpStackError));



      private:

        CORBA::ORB_var            orb_;
        PortableServer::POA_var   poa_;
        AlpineCorba_impl *        implParent_;

    };




    // DtcpStackStatus Interface member class
    //
    class DtcpStackStatus : public POA_Alpine::DtcpStackStatus
    {
      public:

        DtcpStackStatus (const CORBA::ORB_var &          orb,
                         const PortableServer::POA_var & poa,
                         AlpineCorba_impl *              implParent);

        virtual ~DtcpStackStatus ();



        // Get transfer buffer stats
        //
        virtual void  getBufferStats (Alpine::t_DtcpStackBufferStats_out   stats,
                                      CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

             ExThrowSpec ((CORBA::SystemException,
                           Alpine::e_DtcpStackError));



      private:

        CORBA::ORB_var            orb_;
        PortableServer::POA_var   poa_;
        AlpineCorba_impl *        implParent_;

    };




    // AlpineStackMgmt Interface member class
    //
    class AlpineStackMgmt : public POA_Alpine::AlpineStackMgmt
    {
      public:

        AlpineStackMgmt (const CORBA::ORB_var &          orb,
                         const PortableServer::POA_var & poa,
                         AlpineCorba_impl *              implParent);

        virtual ~AlpineStackMgmt ();



        // Transaction data transfer limits
        //
        virtual void  setTotalTransferLimit (uint  transferLimit,
                                             CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

             ExThrowSpec ((CORBA::SystemException,
                           Alpine::e_InvalidValue,
                           Alpine::e_AlpineStackError));


        virtual uint  getTotalTransferLimit (CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_AlpineStackError));



        // Per user concurrent transaction data transfer limits (i.e. num per user)
        //
        virtual void  setPeerTransferLimit (uint  transferLimit,
                                            CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_InvalidValue,
                          Alpine::e_AlpineStackError));


        virtual uint  getPeerTransferLimit (CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_AlpineStackError));



      private:

        CORBA::ORB_var            orb_;
        PortableServer::POA_var   poa_;
        AlpineCorba_impl *        implParent_;

    };




    // AlpineStackStatus Interface member class
    //
    class AlpineStackStatus : public POA_Alpine::AlpineStackStatus
    {
      public:

        AlpineStackStatus (const CORBA::ORB_var &          orb,
                           const PortableServer::POA_var & poa,
                           AlpineCorba_impl *              implParent);

        virtual ~AlpineStackStatus ();



        // Get Alpine transfer stats
        //
        virtual void  getTransferStats (Alpine::t_AlpineStackTransferStats_out  stats,
                                        CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_AlpineStackError));



      private:

        CORBA::ORB_var            orb_;
        PortableServer::POA_var   poa_;
        AlpineCorba_impl *        implParent_;

    };




    // AlpineGroupMgmt Interface member class
    //
    class AlpineGroupMgmt : public POA_Alpine::AlpineGroupMgmt
    {
      public:

        AlpineGroupMgmt (const CORBA::ORB_var &          orb,
                         const PortableServer::POA_var & poa,
                         AlpineCorba_impl *              implParent);

        virtual ~AlpineGroupMgmt ();



        // List user defined groups
        //
        virtual void  getUserGroupList (Alpine::t_AlpineGroupInfoList_out  groupInfoList,
                                        CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_AlpineStackError));



        // Create new group (returns created group ID)
        //
        virtual uint  createUserGroup (const char *  groupName,
                                       const char *  description,
                                       CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_GroupAlreadyExists,
                          Alpine::e_AlpineStackError));




        // Destroy user group
        //
        virtual void  destroyUserGroup (uint  groupId,
                                        CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_InvalidGroupId,
                          Alpine::e_AlpineStackError));



        // List groups peer is a member of
        //
        virtual void  getPeerUserGroupList (uint                               peerId,
                                            Alpine::t_AlpineGroupInfoList_out  groupInfoList,
                                            CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_InvalidPeerId,
                          Alpine::e_AlpineStackError));



        // Add peer to specific group
        //
        virtual void  addPeerToGroup (uint  peerId,
                                      uint  groupId,
                                      CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_InvalidPeerId,
                          Alpine::e_InvalidGroupId,
                          Alpine::e_AlpineStackError));



        // Remove peer from specific group
        //
        virtual void  removePeerFromGroup (uint  peerId,
                                           uint  groupId,
                                           CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_InvalidPeerId,
                          Alpine::e_InvalidGroupId,
                          Alpine::e_AlpineStackError));



      private:

        CORBA::ORB_var            orb_;
        PortableServer::POA_var   poa_;
        AlpineCorba_impl *        implParent_;

    };




    // AlpinePeerMgmt Interface member class
    //
    class AlpinePeerMgmt : public POA_Alpine::AlpinePeerMgmt
    {
      public:

        AlpinePeerMgmt (const CORBA::ORB_var &          orb,
                        const PortableServer::POA_var & poa,
                        AlpineCorba_impl *              implParent);

        virtual ~AlpinePeerMgmt ();



        // List peers with extended information
        //
        virtual void  getExtendedPeerList (Alpine::t_DtcpPeerIdList_out  peerIdList,
                                           CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_AlpineStackError));



        // Get information for a specific peer
        //
        virtual void  getPeerInformation (Alpine::t_AlpinePeerInfo_out  peerInfo,
                                          CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_InvalidPeerId,
                          Alpine::e_AlpineStackError));



        // Update information for a specific peer
        //
        virtual void updatePeerInformation (const Alpine::t_AlpinePeerInfo &  peerInfo,
                                            CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_InvalidPeerId,
                          Alpine::e_AlpineStackError));



      private:

        CORBA::ORB_var            orb_;
        PortableServer::POA_var   poa_;
        AlpineCorba_impl *        implParent_;

    };




    // AlpineQuery Interface member class
    //
    class AlpineQuery : public POA_Alpine::AlpineQuery
    {
      public:

        AlpineQuery (const CORBA::ORB_var &          orb,
                     const PortableServer::POA_var & poa,
                     AlpineCorba_impl *              implParent);

        virtual ~AlpineQuery ();



        // Get default options for queries
        //
        virtual void  getDefaultOptions (Alpine::t_QueryOptions_out  options,
                                         CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_AlpineStackError));



        // Set default options for queries
        //
        virtual void  setDefaultOptions (const Alpine::t_QueryOptions &  options,
                                         CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_AlpineStackError,
                          Alpine::e_InvalidQueryOptions));



        // Create query
        //
        virtual void  startQuery (const Alpine::t_QueryOptions &  options,
                                  const char *                    queryString,
                                  uint &                          queryId,
                                  CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_AlpineStackError,
                          Alpine::e_InvalidQueryOptions));



        // Check query in progress
        //
        virtual CORBA::Boolean  inProgress (uint  queryId,
                                            CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_InvalidQueryId));



        // Get query status information
        //
        virtual void  getQueryStatus (uint                       queryId,
                                      Alpine::t_QueryStatus_out  queryStatus,
                                      CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_AlpineStackError,
                          Alpine::e_InvalidQueryId,
                          Alpine::e_InvalidQueryOperation));



        // Pause active query
        //
        virtual void  pauseQuery (uint  queryId,
                                  CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_AlpineStackError,
                          Alpine::e_InvalidQueryId,
                          Alpine::e_InvalidQueryOperation));



        // Resume paused query
        //
        virtual void  resumeQuery (uint  queryId,
                                   CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_AlpineStackError,
                          Alpine::e_InvalidQueryId,
                          Alpine::e_InvalidQueryOperation));



        // Cancel query
        //
        virtual void  cancelQuery (uint  queryId,
                                   CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_AlpineStackError,
                          Alpine::e_InvalidQueryId,
                          Alpine::e_InvalidQueryOperation));



        // Get query results
        //
        virtual void  getQueryResults (uint        queryId,
                                       Alpine::t_PeerResourcesList_out  results,
                                       CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_AlpineStackError,
                          Alpine::e_InvalidQueryId,
                          Alpine::e_InvalidQueryOperation));



        // Get a list of active query IDs
        //
        virtual void  getActiveQueryIdList (Alpine::t_QueryIdList_out  queryIdList,
                                            CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_AlpineStackError));



        // Get a list of past query IDs
        //
        virtual void  getPastQueryIdList (Alpine::t_QueryIdList_out  queryIdList,
                                          CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_AlpineStackError));



      private:

        CORBA::ORB_var            orb_;
        PortableServer::POA_var   poa_;
        AlpineCorba_impl *        implParent_;

    };




    // AlpineModuleMgmt Interface member class
    //
    class AlpineModuleMgmt : public POA_Alpine::AlpineModuleMgmt
    {
      public:

        AlpineModuleMgmt (const CORBA::ORB_var &          orb,
                          const PortableServer::POA_var & poa,
                          AlpineCorba_impl *              implParent);

        virtual ~AlpineModuleMgmt ();



        // Register a new ALPINE Server module
        //
        virtual void  registerModule (const char *   libraryPath,
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
        virtual void  unregisterModule (uint  moduleId,
                                        CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_InvalidModuleId,
                          Alpine::e_ModuleActive,
                          Alpine::e_ModuleError));



        // Set configuration values for registered module
        //
        virtual void  setConfiguration (uint                          moduleId,
                                        const Alpine::t_ConfigData &  configData,
                                        CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_InvalidModuleId,
                          Alpine::e_ModuleError));



        // Get current configuration values for module
        //
        virtual void  getConfiguration (uint                       moduleId,
                                        Alpine::t_ConfigData_out   configData,
                                        CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_InvalidModuleId,
                          Alpine::e_ModuleError));



        // Get module information
        //
        virtual void  getModuleInfo (uint                            moduleId,
                                     Alpine::t_AlpineModuleInfo_out  moduleInfo,
                                     CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_InvalidModuleId,
                          Alpine::e_ModuleError));



        // Load a registered module (i.e. activate it)
        //
        virtual void  loadModule (uint  moduleId,
                                  CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_InvalidModuleId,
                          Alpine::e_ModuleActive,
                          Alpine::e_ModuleError));



        // Unload module (deactivate)
        //
        virtual void  unloadModule (uint  moduleId,
                                    CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_InvalidModuleId,
                          Alpine::e_ModuleNotActive,
                          Alpine::e_ModuleError));



        // List all active/loaded modules
        //
        virtual void  listActiveModules (Alpine::t_ModuleIdList_out  idList,
                                         CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_ModuleError));



        // List all modules
        //
        virtual void  listAllModules (Alpine::t_ModuleIdList_out  idList,
                                      CORBA::Environment &ACE_TRY_ENV = TAO_default_environment ())

            ExThrowSpec ((CORBA::SystemException,
                          Alpine::e_ModuleError));



      private:

        CORBA::ORB_var            orb_;
        PortableServer::POA_var   poa_;
        AlpineCorba_impl *        implParent_;

    };





  private:

    CORBA::ORB_var           orb_;
    PortableServer::POA_var  poa_;
    bool                     initialized_;


    // Interface implementations and references
    //
    DtcpPeerMgmt *                 dtcpPeerMgmtInterface_;
    Alpine::DtcpPeerMgmt_var       dtcpPeerMgmtReference_;

    DtcpStackMgmt *                dtcpStackMgmtInterface_;
    Alpine::DtcpStackMgmt_var      dtcpStackMgmtReference_;

    DtcpStackStatus *              dtcpStackStatusInterface_;
    Alpine::DtcpStackStatus_var    dtcpStackStatusReference_;

    AlpineStackMgmt *              alpineStackMgmtInterface_;
    Alpine::AlpineStackMgmt_var    alpineStackMgmtReference_;

    AlpineStackStatus *            alpineStackStatusInterface_;
    Alpine::AlpineStackStatus_var  alpineStackStatusReference_;

    AlpineGroupMgmt *              alpineGroupMgmtInterface_;
    Alpine::AlpineGroupMgmt_var    alpineGroupMgmtReference_;

    AlpinePeerMgmt *               alpinePeerMgmtInterface_;
    Alpine::AlpinePeerMgmt_var     alpinePeerMgmtReference_;

    AlpineQuery *                  alpineQueryInterface_;
    Alpine::AlpineQuery_var        alpineQueryReference_;

    AlpineModuleMgmt *             alpineModuleMgmtInterface_;
    Alpine::AlpineModuleMgmt_var   alpineModuleMgmtReference_;
    


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

