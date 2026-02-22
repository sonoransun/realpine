/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AlpineCorbaServer.h>
#include <AlpineCorba_impl.h>
#include <ReadLock.h>
#include <WriteLock.h>
#include <Log.h>
#include <StringUtils.h>



AlpineCorba_impl *        AlpineCorbaServer::alpineCorba_impl_s = 0;
bool                      AlpineCorbaServer::initialized_s = false;
ReadWriteSem              AlpineCorbaServer::dataLock_s;



// Ctor defaulted in header


// Dtor defaulted in header



bool  
AlpineCorbaServer::initialize (const CORBA::ORB_var &           orb,
                               const PortableServer::POA_var &  poa)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaServer::initialize invoked.");
#endif

    WriteLock  lock(dataLock_s);

    if (initialized_s) {
        Log::Error ("Attempt to reinitialize AlpineCorbaServer!");
        return false;
    }


    alpineCorba_impl_s = new AlpineCorba_impl (orb, poa);

    bool status;
    status = alpineCorba_impl_s->initialize ();

    if (!status) {
        Log::Error ("Unable to initialize Alpine CORBA interface in "
                             "AlpineCorbaServer::initialize.");
        return false;
    }

    initialized_s = true;


    return true;
}


bool  
AlpineCorbaServer::getDtcpPeerMgmt (Alpine::DtcpPeerMgmt_var &  dtcpPeerMgmt)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaServer::getDtcpPeerMgmt invoked.");
#endif

    ReadLock  lock(dataLock_s);

    if (!initialized_s) {
        Log::Error ("AlpineCorbaServer::getDtcpPeerMgmt invoked before initialization!");
        return false;
    }


    bool status;
    status = alpineCorba_impl_s->getDtcpPeerMgmt (dtcpPeerMgmt);


    return status;
}



bool
AlpineCorbaServer::getDtcpStackMgmt (Alpine::DtcpStackMgmt_var &  dtcpStackMgmt)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaServer::getDtcpStackMgmt invoked.");
#endif

    ReadLock  lock(dataLock_s);

    if (!initialized_s) {
        Log::Error ("AlpineCorbaServer::getDtcpStackMgmt invoked before initialization!");
        return false;
    }


    bool status;
    status = alpineCorba_impl_s->getDtcpStackMgmt (dtcpStackMgmt);


    return status;
}



bool
AlpineCorbaServer::getDtcpStackStatus (Alpine::DtcpStackStatus_var &  dtcpStackStatus)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaServer::getDtcpStackStatus invoked.");
#endif

    ReadLock  lock(dataLock_s);

    if (!initialized_s) {
        Log::Error ("AlpineCorbaServer::getDtcpStackStatus invoked before initialization!");
        return false;
    }


    bool status;
    status = alpineCorba_impl_s->getDtcpStackStatus (dtcpStackStatus);


    return status;
}



bool
AlpineCorbaServer::getAlpineStackMgmt (Alpine::AlpineStackMgmt_var &  alpineStackMgmt)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaServer::getAlpineStackMgmt invoked.");
#endif

    ReadLock  lock(dataLock_s);

    if (!initialized_s) {
        Log::Error ("AlpineCorbaServer::getAlpineStackMgmt invoked before initialization!");
        return false;
    }


    bool status;
    status = alpineCorba_impl_s->getAlpineStackMgmt (alpineStackMgmt);


    return status;
}



bool
AlpineCorbaServer::getAlpineStackStatus (Alpine::AlpineStackStatus_var &  alpineStackStatus)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaServer::getAlpineStackStatus invoked.");
#endif

    ReadLock  lock(dataLock_s);

    if (!initialized_s) {
        Log::Error ("AlpineCorbaServer::getAlpineStackStatus invoked before initialization!");
        return false;
    }


    bool status;
    status = alpineCorba_impl_s->getAlpineStackStatus (alpineStackStatus);


    return status;
}



bool
AlpineCorbaServer::getAlpineGroupMgmt (Alpine::AlpineGroupMgmt_var &  alpineGroupMgmt)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaServer::getAlpineGroupMgmt invoked.");
#endif

    ReadLock  lock(dataLock_s);

    if (!initialized_s) {
        Log::Error ("AlpineCorbaServer::getAlpineGroupMgmt invoked before initialization!");
        return false;
    }


    bool status;
    status = alpineCorba_impl_s->getAlpineGroupMgmt (alpineGroupMgmt);


    return status;
}



bool
AlpineCorbaServer::getAlpinePeerMgmt (Alpine::AlpinePeerMgmt_var &  alpinePeerMgmt)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaServer::getAlpinePeerMgmt invoked.");
#endif

    ReadLock  lock(dataLock_s);

    if (!initialized_s) {
        Log::Error ("AlpineCorbaServer::getAlpinePeerMgmt invoked before initialization!");
        return false;
    }


    bool status;
    status = alpineCorba_impl_s->getAlpinePeerMgmt (alpinePeerMgmt);


    return status;
}



bool
AlpineCorbaServer::getAlpineQuery (Alpine::AlpineQuery_var &  alpineQuery)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaServer::getAlpineQuery invoked.");
#endif

    ReadLock  lock(dataLock_s);

    if (!initialized_s) {
        Log::Error ("AlpineCorbaServer::getAlpineQuery invoked before initialization!");
        return false;
    }


    bool status;
    status = alpineCorba_impl_s->getAlpineQuery (alpineQuery);


    return status;
}



bool
AlpineCorbaServer::getAlpineModuleMgmt (Alpine::AlpineModuleMgmt_var &  alpineModuleMgmt)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaServer::getAlpineModuleMgmt invoked.");
#endif

    ReadLock  lock(dataLock_s);

    if (!initialized_s) {
        Log::Error ("AlpineCorbaServer::getAlpineModuleMgmt invoked before initialization!");
        return false;
    }


    bool status;
    status = alpineCorba_impl_s->getAlpineModuleMgmt (alpineModuleMgmt);


    return status;
}



