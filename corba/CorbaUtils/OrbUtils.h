/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <CorbaUtils.h>
#include <ReadWriteSem.h>
#include <tao/PortableServer/ORB_Manager.h>
#include <vector>


class OrbEventThread;


class OrbUtils 
{
  public:

    OrbUtils () = default;
    ~OrbUtils () = default;


    static bool  initialize (int     argc, 
                             char ** argv,
                             ushort  numThreads = 1);

    static bool  getOrb (CORBA::ORB_var &  orbVar);

    static bool  getRootPoa (PortableServer::POA_var &  poaVar);



    using t_OrbThreadList = vector<OrbEventThread *>;

  private:

    static TAO_ORB_Manager           orbManager_s;
    static CORBA::ORB_var            orb_s;
    static PortableServer::POA_var   rootPoa_s;
    static bool                      initialized_s;
    static ushort                    numThreads_s;
    static t_OrbThreadList           orbThreadList_s;
    static ReadWriteSem              dataLock_s;



    static void  runEventLoop  ();

    
    friend class OrbEventThread;

};

