/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <OrbUtils.h>
#include <OrbEventThread.h>
#include <Log.h>
#include <StringUtils.h>
#include <ReadLock.h>
#include <WriteLock.h>



TAO_ORB_Manager               OrbUtils::orbManager_s;
CORBA::ORB_var                OrbUtils::orb_s;
PortableServer::POA_var       OrbUtils::rootPoa_s;
bool                          OrbUtils::initialized_s;
ushort                        OrbUtils::numThreads_s;
OrbUtils::t_OrbThreadList     OrbUtils::orbThreadList_s;
ReadWriteSem                  OrbUtils::dataLock_s;



// Ctor defaulted in header


// Dtor defaulted in header



bool
OrbUtils::initialize (int     argc, 
                      char ** argv,
                      ushort  numThreads)
{
#ifdef _VERBOSE
    Log::Debug ("OrbUtils::initialize invoked.  Thread pool size: "s + std::to_string(numThreads));
#endif

    WriteLock  lock (dataLock_s);

    if (initialized_s) {
        Log::Error ("Attempt to re-initialize OrbUtils.  Ignoring.");
        return false;
    }


    int retVal;

    // Initialize ORB
    //
    CORBA::Environment ExTryEnv = TAO_default_environment ();


    ExTry {

        retVal = orbManager_s.init (argc, argv, ExTryEnv);

        if (retVal < 0) {
            Log::Error ("Failure initializing ORB Manager in OrbUtils::initialize.");
            return false;
        }

        orb_s     = orbManager_s.orb ();
        rootPoa_s = orbManager_s.root_poa ();
    }

    ExCatchAny {
        Log::Error ("Exception caught during ORB Manager initialization in OrbUtils::initialize.");
        return false;
    }
    ExEndTry;
    ExCheck;


    // Start ORB event threads
    //
    numThreads_s = numThreads;

    int i;
    OrbEventThread * currThread;

    for (i = 0; i < numThreads; i++) {
        currThread = new OrbEventThread;
        currThread->run ();
        orbThreadList_s.push_back (currThread);
    }

    initialized_s = true;


    return(true);
}



bool  
OrbUtils::getOrb (CORBA::ORB_var &  orbVar)
{
    ReadLock  lock(dataLock_s);

    if (!initialized_s) {
        return false;
    }

    orbVar = orbManager_s.orb ();


    return true;
}



bool  
OrbUtils::getRootPoa (PortableServer::POA_var &  poaVar)
{
    ReadLock  lock(dataLock_s);

    if (!initialized_s) {
        return false;
    }

    poaVar = orbManager_s.root_poa ();


    return true;
}



void  
OrbUtils::runEventLoop  ()
{
    int retVal;

    CORBA::Environment ExTryEnv = TAO_default_environment ();

    ExTry {

        retVal = orbManager_s.run (ExTryEnv);
    }
    ExCatchAll {
        Log::Error ("Exception caught during ORB run loop in OrbUtils::runEventLoop.");
    }
    ExEndTry;
    ExCheck;

    if (retVal < 0) {
        Log::Error ("Exception caught during ORB run loop in OrbUtils::runEventLoop.");
    }
}







#if 0
bool
OrbUtils::stringToObject (CORBA::Object_var  &object, CORBA::Environment &ExTryEnv)
{

    if (!ior_) {
        return false;
    }

    if (!orbInit_) {
        return false;
    }

    ExTry {

        object = orb_->string_to_object (ior_, ExTryEnv);
 
        ExTryCheck;
    }

    ExCatchAny {
        return(false);
    }
    ExEndTry;
    ExCheck;


    return true;
}
#endif

