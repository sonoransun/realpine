/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <CorbaUtils.h>
#include <OrbUtils.h>
#include <ReadWriteSem.h>


class CorbaAdmin
{
  public:

    CorbaAdmin () = default;
    ~CorbaAdmin () = default;


    // Initialize the OrbUtils and CosNamingUtils interfaces.  
    // Start ORB thread(s)
    //
    static bool  initialize (int      argc,
                             char **  argv);
                             


    // Methods specific to the CORBA interface implementations supported
    // activate creates the interface and registers its reference in the
    // naming service server for use by external entities.
    //
    static bool  activateAlpineCorbaServer (const string &  namingContextPath);
                             


  private:

    static CORBA::ORB_var            orb_s;
    static PortableServer::POA_var   rootPoa_s;
    static bool                      initialized_s;
    static bool                      alpineCorbaServerActive_s;
    static ReadWriteSem              dataLock_s;


    static bool  bindDtcpPeerMgmtReference (const string & bindingPath);

    static bool  bindDtcpStackMgmtReference (const string & bindingPath);

    static bool  bindDtcpStackStatusReference (const string & bindingPath);

    static bool  bindAlpineStackMgmtReference (const string & bindingPath);

    static bool  bindAlpineStackStatusReference (const string & bindingPath);

    static bool  bindAlpineGroupMgmtReference (const string & bindingPath);

    static bool  bindAlpinePeerMgmtReference (const string & bindingPath);

    static bool  bindAlpineQueryReference (const string & bindingPath);

    static bool  bindAlpineModuleMgmtReference (const string & bindingPath);

};

