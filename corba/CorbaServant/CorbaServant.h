/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <AlpineC.h>
#include <CorbaUtils.h>
#include <OrbUtils.h>
#include <ReadWriteSem.h>


class CorbaServant
{
  public:


    // Initialize the OrbUtils and CosNamingUtils interfaces.
    // Start ORB thread(s)
    //
    static bool  initialize (int      argc,
                             char **  argv);



    // Methods specific to the CORBA interface implementations supported
    // This method initializes the CORBA interface to the server, resolving
    // refernces, etc.
    //
    static bool  intializeAlpineClientInterface (const string &  namingContextPath);



  private:

    static CORBA::ORB_var            orb_s;
    static bool                      initialized_s;
    static bool                      clientInterfaceActive_s;
    static ReadWriteSem              dataLock_s;


    static bool  resolveDtcpPeerMgmtReference (const string &              bindingPath,
                                               Alpine::DtcpPeerMgmt_var &  reference);

    static bool  resolveDtcpStackMgmtReference (const string &               bindingPath,
                                                Alpine::DtcpStackMgmt_var &  reference);

    static bool  resolveDtcpStackStatusReference (const string &                 bindingPath,
                                                  Alpine::DtcpStackStatus_var &  reference);

    static bool  resolveAlpineStackMgmtReference (const string &                 bindingPath,
                                                  Alpine::AlpineStackMgmt_var &  reference);

    static bool  resolveAlpineStackStatusReference (const string &                   bindingPath,
                                                    Alpine::AlpineStackStatus_var &  reference);

    static bool  resolveAlpineGroupMgmtReference (const string &                 bindingPath,
                                                  Alpine::AlpineGroupMgmt_var &  reference);

    static bool  resolveAlpinePeerMgmtReference (const string &                bindingPath,
                                                 Alpine::AlpinePeerMgmt_var &  reference);

    static bool  resolveAlpineQueryReference (const string &             bindingPath,
                                              Alpine::AlpineQuery_var &  reference);

    static bool  resolveAlpineModuleMgmtReference (const string &                  bindingPath,
                                                   Alpine::AlpineModuleMgmt_var &  reference);


};

