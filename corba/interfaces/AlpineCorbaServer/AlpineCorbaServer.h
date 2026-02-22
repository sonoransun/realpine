/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <CorbaUtils.h>
#include <AlpineC.h>
#include <ReadWriteSem.h>


class AlpineCorba_impl;


class AlpineCorbaServer
{
  public:

    AlpineCorbaServer () = default;
    ~AlpineCorbaServer () = default;



    static bool  initialize (const CORBA::ORB_var &           orb,
                             const PortableServer::POA_var &  poa);


    static bool  getDtcpPeerMgmt (Alpine::DtcpPeerMgmt_var &  dtcpPeerMgmt);

    static bool  getDtcpStackMgmt (Alpine::DtcpStackMgmt_var &  dtcpStackMgmt);

    static bool  getDtcpStackStatus (Alpine::DtcpStackStatus_var &  dtcpStackStatus);

    static bool  getAlpineStackMgmt (Alpine::AlpineStackMgmt_var &  alpineStackMgmt);

    static bool  getAlpineStackStatus (Alpine::AlpineStackStatus_var &  alpineStackStatus);

    static bool  getAlpineGroupMgmt (Alpine::AlpineGroupMgmt_var &  alpineGroupMgmt);

    static bool  getAlpinePeerMgmt (Alpine::AlpinePeerMgmt_var &  alpinePeerMgmt);

    static bool  getAlpineQuery (Alpine::AlpineQuery_var &  alpineQuery);

    static bool  getAlpineModuleMgmt (Alpine::AlpineModuleMgmt_var &  alpineModuleMgmt);



  private:

    static AlpineCorba_impl *  alpineCorba_impl_s;
    static bool                initialized_s;
    static ReadWriteSem        dataLock_s;

};

