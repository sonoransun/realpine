/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <CorbaServant.h>
#include <AlpineCorbaClient.h>
#include <CosNamingUtils.h>
#include <ReadLock.h>
#include <WriteLock.h>
#include <Log.h>
#include <StringUtils.h>



CORBA::ORB_var            CorbaServant::orb_s;
bool                      CorbaServant::initialized_s = false;
bool                      CorbaServant::clientInterfaceActive_s = false;
ReadWriteSem              CorbaServant::dataLock_s;



bool  
CorbaServant::initialize (int      argc,
                          char **  argv)
{
#ifdef _VERBOSE
    Log::Debug ("CorbaServant::initialize invoked.");
#endif

    WriteLock  lock(dataLock_s);


    bool status;

    // Initialize ORB
    //
    status = OrbUtils::initialize (argc, argv);

    if (!status) {
        Log::Error ("OrbUtils initialization failed in CorbaServant::initialize!");
        return false;
    }

    status = OrbUtils::getOrb (orb_s);

    if (!status) {
        Log::Error ("Getting ORB reference failed in CorbaServant::initialize!");
        return false;
    }


    // Initialize CosNamingUtils
    //
    status = CosNamingUtils::initialize (orb_s);

    if (!status) {
        Log::Error ("CosNamingUtils initialization failed in CorbaServant::initialize!");
        return 1;
    }

    initialized_s = true;


    return true;
}



bool  
CorbaServant::intializeAlpineClientInterface (const string &  namingContextPath)
{
#ifdef _VERBOSE
    Log::Debug ("CorbaServant::intializeAlpineClientInterface invoked.");
#endif

    CORBA::ORB_var           orb;

    // scope lock
    {
        ReadLock  lock(dataLock_s);

        if (!initialized_s) {
            Log::Error ("Attempt to init client corba interface without initialization in "
                                 "CorbaServant::intializeAlpineClientInterface.");

            return false;
        }

        if (clientInterfaceActive_s) {
            Log::Error ("Corba interface already active in "
                                 "CorbaServant::intializeAlpineClientInterface.");

            return false;
        }

        // make temp copy of ORB reference so we can release lock
        //
        orb     = orb_s;
    }


    // Verify trailing '/' in naming context path
    //
    int lastChar = namingContextPath.size () - 1;
    string  fullContextPath;

    if (namingContextPath[lastChar] != '/') {
        fullContextPath = namingContextPath + "/";
    }
    else {
        fullContextPath = namingContextPath;
    }


    // Resolve interfaces in the naming service
    //
    string currentBinding;
    bool   status;

    // DtcpPeerMgmt
    //
    Alpine::DtcpPeerMgmt_var  dtcpPeerMgmtReference;
    currentBinding = fullContextPath + "DtcpPeerMgmtInterface";
    status = resolveDtcpPeerMgmtReference (currentBinding,
                                           dtcpPeerMgmtReference);

    if (!status) {
        return status;
    }

    // DtcpStackMgmt
    //
    Alpine::DtcpStackMgmt_var  dtcpStackMgmtReference;
    currentBinding = fullContextPath + "DtcpStackMgmtInterface";
    status = resolveDtcpStackMgmtReference (currentBinding,
                                            dtcpStackMgmtReference);

    if (!status) {
        return status;
    }

    // DtcpStackStatus
    //
    Alpine::DtcpStackStatus_var  dtcpStackStatusReference;
    currentBinding = fullContextPath + "DtcpStackStatusInterface";
    status = resolveDtcpStackStatusReference (currentBinding,
                                              dtcpStackStatusReference);

    if (!status) {
        return status;
    }

    // AlpineStackMgmt
    //
    Alpine::AlpineStackMgmt_var  alpineStackMgmtReference;
    currentBinding = fullContextPath + "AlpineStackMgmtInterface";
    status = resolveAlpineStackMgmtReference (currentBinding,
                                              alpineStackMgmtReference);

    if (!status) {
        return status;
    }

    // AlpineStackStatus
    //
    Alpine::AlpineStackStatus_var  alpineStackStatusReference;
    currentBinding = fullContextPath + "AlpineStackStatusInterface";
    status = resolveAlpineStackStatusReference (currentBinding,
                                                alpineStackStatusReference);

    if (!status) {
        return status;
    }

    // AlpineGroupMgmt
    //
    Alpine::AlpineGroupMgmt_var  alpineGroupMgmtReference;
    currentBinding = fullContextPath + "AlpineGroupMgmtInterface";
    status = resolveAlpineGroupMgmtReference (currentBinding,
                                              alpineGroupMgmtReference);

    if (!status) {
        return status;
    }

    // AlpinePeerMgmt
    //
    Alpine::AlpinePeerMgmt_var  alpinePeerMgmtReference;
    currentBinding = fullContextPath + "AlpinePeerMgmtInterface";
    status = resolveAlpinePeerMgmtReference (currentBinding,
                                             alpinePeerMgmtReference);

    if (!status) {
        return status;
    }

    // AlpineQuery
    //
    Alpine::AlpineQuery_var  alpineQueryReference;
    currentBinding = fullContextPath + "AlpineQueryInterface";
    status = resolveAlpineQueryReference (currentBinding,
                                          alpineQueryReference);

    if (!status) {
        return status;
    }

    // AlpineModuleMgmt
    //
    Alpine::AlpineModuleMgmt_var  alpineModuleMgmtReference;
    currentBinding = fullContextPath + "AlpineModuleMgmtInterface";
    status = resolveAlpineModuleMgmtReference (currentBinding,
                                               alpineModuleMgmtReference);

    if (!status) {
        return status;
    } 




    // Create Alpine client interface instance and activate
    //
    status = AlpineCorbaClient::initialize (orb, 
                                            dtcpPeerMgmtReference,
                                            dtcpStackMgmtReference,
                                            dtcpStackStatusReference,
                                            alpineStackMgmtReference,
                                            alpineStackStatusReference,
                                            alpineGroupMgmtReference,
                                            alpinePeerMgmtReference,
                                            alpineQueryReference,
                                            alpineModuleMgmtReference);

    if (!status) {
        Log::Error ("Attempt to initialize AlpineCorbaClient failed in "
                             "CorbaServant::intializeAlpineClientInterface.");

        return false;
    }


    // Finished...
    //
    {
        WriteLock  lock(dataLock_s);
        clientInterfaceActive_s = true;
    }


    return true;
}



bool  
CorbaServant::resolveDtcpPeerMgmtReference (const string &              bindingPath,
                                            Alpine::DtcpPeerMgmt_var &  reference)
{
#ifdef _VERBOSE
    Log::Debug ("CorbaServant::resolveDtcpPeerMgmtReference invoked."
                         "\nBinding Path: " + bindingPath +
                "\n");
#endif

    bool               status;
    CosNamingUtils     namingUtil;
    CORBA::Object_var  object;

    if (!namingUtil.bindingExists (bindingPath)) {

        // Error, we must have a context to initialize client interface.
        Log::Error ("DtcpPeerMgmt interface binding does not exist in naming service. "
                             "CorbaServant::resolveDtcpPeerMgmtReference failed.");

        return false;
    }

    status = namingUtil.resolve (bindingPath, object);

    if (!status) {
        Log::Error ("Resolve DtcpPeerMgmt interface binding failed. "
                             "CorbaServant::resolveDtcpPeerMgmtReference failed.");

        return false;
    }

    if (CORBA::is_nil (object.in())) {
        Log::Error ("DtcpPeerMgmt interface binding in naming service is NIL. "
                             "CorbaServant::resolveDtcpPeerMgmtReference failed.");

        return false;
    }


    // Verify bound object
    //
    ExNewEnv;

    ExTry {

        reference = Alpine::DtcpPeerMgmt::_narrow (object.in ());

        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {

        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " when narrowing bound object in "
                             "CorbaServant::resolveDtcpPeerMgmtReference.");
        return false;
    }
    ExCatchAny {

        Log::Error ("Caught unexpected exception when narrowing bound object in "
                             "CorbaServant::resolveDtcpPeerMgmtReference.");
        return false;
    }
    ExEndTry;
    ExCheck;


    return true;
}



bool  
CorbaServant::resolveDtcpStackMgmtReference (const string &               bindingPath,
                                             Alpine::DtcpStackMgmt_var &  reference)
{
#ifdef _VERBOSE
    Log::Debug ("CorbaServant::resolveDtcpStackMgmtReference invoked."
                         "\nBinding Path: " + bindingPath +
                "\n");
#endif

    bool               status;
    CosNamingUtils     namingUtil;
    CORBA::Object_var  object;

    if (!namingUtil.bindingExists (bindingPath)) {

        // Error, we must have a context to initialize client interface.
        Log::Error ("DtcpStackMgmt interface binding does not exist in naming service. "
                             "CorbaServant::resolveDtcpStackMgmtReference failed.");

        return false;
    }

    status = namingUtil.resolve (bindingPath, object);

    if (!status) {
        Log::Error ("Resolve DtcpStackMgmt interface binding failed. "
                             "CorbaServant::resolveDtcpStackMgmtReference failed.");

        return false;
    }

    if (CORBA::is_nil (object.in())) {
        Log::Error ("DtcpStackMgmt interface binding in naming service is NIL. "
                             "CorbaServant::resolveDtcpStackMgmtReference failed.");

        return false;
    }


    // Verify bound object
    //
    ExNewEnv;

    ExTry {

        reference = Alpine::DtcpStackMgmt::_narrow (object.in ());

        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {

        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " when narrowing bound object in "
                             "CorbaServant::resolveDtcpStackMgmtReference.");
        return false;
    }
    ExCatchAny {

        Log::Error ("Caught unexpected exception when narrowing bound object in "
                             "CorbaServant::resolveDtcpStackMgmtReference.");
        return false;
    }
    ExEndTry;
    ExCheck;


    return true;
}



bool  
CorbaServant::resolveDtcpStackStatusReference (const string &                 bindingPath,
                                               Alpine::DtcpStackStatus_var &  reference)
{
#ifdef _VERBOSE
    Log::Debug ("CorbaServant::resolveDtcpStackStatusReference invoked."
                         "\nBinding Path: " + bindingPath +
                "\n");
#endif

    bool               status;
    CosNamingUtils     namingUtil;
    CORBA::Object_var  object;

    if (!namingUtil.bindingExists (bindingPath)) {

        // Error, we must have a context to initialize client interface.
        Log::Error ("DtcpStackStatus interface binding does not exist in naming service. "
                             "CorbaServant::resolveDtcpStackStatusReference failed.");

        return false;
    }

    status = namingUtil.resolve (bindingPath, object);

    if (!status) {
        Log::Error ("Resolve DtcpStackStatus interface binding failed. "
                             "CorbaServant::resolveDtcpStackStatusReference failed.");

        return false;
    }

    if (CORBA::is_nil (object.in())) {
        Log::Error ("DtcpStackStatus interface binding in naming service is NIL. "
                             "CorbaServant::resolveDtcpStackStatusReference failed.");

        return false;
    }


    // Verify bound object
    //
    ExNewEnv;

    ExTry {

        reference = Alpine::DtcpStackStatus::_narrow (object.in ());

        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {

        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " when narrowing bound object in "
                             "CorbaServant::resolveDtcpStackStatusReference.");
        return false;
    }
    ExCatchAny {

        Log::Error ("Caught unexpected exception when narrowing bound object in "
                             "CorbaServant::resolveDtcpStackStatusReference.");
        return false;
    }
    ExEndTry;
    ExCheck;


    return true;
}



bool  
CorbaServant::resolveAlpineStackMgmtReference (const string &                 bindingPath,
                                               Alpine::AlpineStackMgmt_var &  reference)
{
#ifdef _VERBOSE
    Log::Debug ("CorbaServant::resolveAlpineStackMgmtReference invoked."
                         "\nBinding Path: " + bindingPath +
                "\n");
#endif

    bool               status;
    CosNamingUtils     namingUtil;
    CORBA::Object_var  object;

    if (!namingUtil.bindingExists (bindingPath)) {

        // Error, we must have a context to initialize client interface.
        Log::Error ("AlpineStackMgmt interface binding does not exist in naming service. "
                             "CorbaServant::resolveAlpineStackMgmtReference failed.");

        return false;
    }

    status = namingUtil.resolve (bindingPath, object);

    if (!status) {
        Log::Error ("Resolve AlpineStackMgmt interface binding failed. "
                             "CorbaServant::resolveAlpineStackMgmtReference failed.");

        return false;
    }

    if (CORBA::is_nil (object.in())) {
        Log::Error ("AlpineStackMgmt interface binding in naming service is NIL. "
                             "CorbaServant::resolveAlpineStackMgmtReference failed.");

        return false;
    }


    // Verify bound object
    //
    ExNewEnv;

    ExTry {

        reference = Alpine::AlpineStackMgmt::_narrow (object.in ());

        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {

        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " when narrowing bound object in "
                             "CorbaServant::resolveAlpineStackMgmtReference.");
        return false;
    }
    ExCatchAny {

        Log::Error ("Caught unexpected exception when narrowing bound object in "
                             "CorbaServant::resolveAlpineStackMgmtReference.");
        return false;
    }
    ExEndTry;
    ExCheck;


    return true;
}



bool  
CorbaServant::resolveAlpineStackStatusReference (const string &                   bindingPath,
                                                 Alpine::AlpineStackStatus_var &  reference)
{
#ifdef _VERBOSE
    Log::Debug ("CorbaServant::resolveAlpineStackStatusReference invoked."
                         "\nBinding Path: " + bindingPath +
                "\n");
#endif

    bool               status;
    CosNamingUtils     namingUtil;
    CORBA::Object_var  object;

    if (!namingUtil.bindingExists (bindingPath)) {

        // Error, we must have a context to initialize client interface.
        Log::Error ("AlpineStackStatus interface binding does not exist in naming service. "
                             "CorbaServant::resolveAlpineStackStatusReference failed.");

        return false;
    }

    status = namingUtil.resolve (bindingPath, object);

    if (!status) {
        Log::Error ("Resolve AlpineStackStatus interface binding failed. "
                             "CorbaServant::resolveAlpineStackStatusReference failed.");

        return false;
    }

    if (CORBA::is_nil (object.in())) {
        Log::Error ("AlpineStackStatus interface binding in naming service is NIL. "
                             "CorbaServant::resolveAlpineStackStatusReference failed.");

        return false;
    }


    // Verify bound object
    //
    ExNewEnv;

    ExTry {

        reference = Alpine::AlpineStackStatus::_narrow (object.in ());

        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {

        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " when narrowing bound object in "
                             "CorbaServant::resolveAlpineStackStatusReference.");
        return false;
    }
    ExCatchAny {

        Log::Error ("Caught unexpected exception when narrowing bound object in "
                             "CorbaServant::resolveAlpineStackStatusReference.");
        return false;
    }
    ExEndTry;
    ExCheck;


    return true;
}



bool  
CorbaServant::resolveAlpineGroupMgmtReference (const string &                 bindingPath,
                                               Alpine::AlpineGroupMgmt_var &  reference)
{
#ifdef _VERBOSE
    Log::Debug ("CorbaServant::resolveAlpineGroupMgmtReference invoked."
                         "\nBinding Path: " + bindingPath +
                "\n");
#endif

    bool               status;
    CosNamingUtils     namingUtil;
    CORBA::Object_var  object;

    if (!namingUtil.bindingExists (bindingPath)) {

        // Error, we must have a context to initialize client interface.
        Log::Error ("AlpineGroupMgmt interface binding does not exist in naming service. "
                             "CorbaServant::resolveAlpineGroupMgmtReference failed.");

        return false;
    }

    status = namingUtil.resolve (bindingPath, object);

    if (!status) {
        Log::Error ("Resolve AlpineGroupMgmt interface binding failed. "
                             "CorbaServant::resolveAlpineGroupMgmtReference failed.");

        return false;
    }

    if (CORBA::is_nil (object.in())) {
        Log::Error ("AlpineGroupMgmt interface binding in naming service is NIL. "
                             "CorbaServant::resolveAlpineGroupMgmtReference failed.");

        return false;
    }


    // Verify bound object
    //
    ExNewEnv;

    ExTry {

        reference = Alpine::AlpineGroupMgmt::_narrow (object.in ());

        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {

        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " when narrowing bound object in "
                             "CorbaServant::resolveAlpineGroupMgmtReference.");
        return false;
    }
    ExCatchAny {

        Log::Error ("Caught unexpected exception when narrowing bound object in "
                             "CorbaServant::resolveAlpineGroupMgmtReference.");
        return false;
    }
    ExEndTry;
    ExCheck;


    return true;
}



bool  
CorbaServant::resolveAlpinePeerMgmtReference (const string &                bindingPath,
                                              Alpine::AlpinePeerMgmt_var &  reference)
{
#ifdef _VERBOSE
    Log::Debug ("CorbaServant::resolveAlpinePeerMgmtReference invoked."
                         "\nBinding Path: " + bindingPath +
                "\n");
#endif

    bool               status;
    CosNamingUtils     namingUtil;
    CORBA::Object_var  object;

    if (!namingUtil.bindingExists (bindingPath)) {

        // Error, we must have a context to initialize client interface.
        Log::Error ("AlpinePeerMgmt interface binding does not exist in naming service. "
                             "CorbaServant::resolveAlpinePeerMgmtReference failed.");

        return false;
    }

    status = namingUtil.resolve (bindingPath, object);

    if (!status) {
        Log::Error ("Resolve AlpinePeerMgmt interface binding failed. "
                             "CorbaServant::resolveAlpinePeerMgmtReference failed.");

        return false;
    }

    if (CORBA::is_nil (object.in())) {
        Log::Error ("AlpinePeerMgmt interface binding in naming service is NIL. "
                             "CorbaServant::resolveAlpinePeerMgmtReference failed.");

        return false;
    }


    // Verify bound object
    //
    ExNewEnv;

    ExTry {

        reference = Alpine::AlpinePeerMgmt::_narrow (object.in ());

        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {

        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " when narrowing bound object in "
                             "CorbaServant::resolveAlpinePeerMgmtReference.");
        return false;
    }
    ExCatchAny {

        Log::Error ("Caught unexpected exception when narrowing bound object in "
                             "CorbaServant::resolveAlpinePeerMgmtReference.");
        return false;
    }
    ExEndTry;
    ExCheck;


    return true;
}



bool  
CorbaServant::resolveAlpineQueryReference (const string &                bindingPath,
                                           Alpine::AlpineQuery_var &  reference)
{
#ifdef _VERBOSE
    Log::Debug ("CorbaServant::resolveAlpineQueryReference invoked."
                         "\nBinding Path: " + bindingPath +
                "\n");
#endif

    bool               status;
    CosNamingUtils     namingUtil;
    CORBA::Object_var  object;

    if (!namingUtil.bindingExists (bindingPath)) {

        // Error, we must have a context to initialize client interface.
        Log::Error ("AlpineQuery interface binding does not exist in naming service. "
                             "CorbaServant::resolveAlpineQueryReference failed.");

        return false;
    }

    status = namingUtil.resolve (bindingPath, object);

    if (!status) {
        Log::Error ("Resolve AlpineQuery interface binding failed. "
                             "CorbaServant::resolveAlpineQueryReference failed.");

        return false;
    }

    if (CORBA::is_nil (object.in())) {
        Log::Error ("AlpineQuery interface binding in naming service is NIL. "
                             "CorbaServant::resolveAlpineQueryReference failed.");

        return false;
    }


    // Verify bound object
    //
    ExNewEnv;

    ExTry {

        reference = Alpine::AlpineQuery::_narrow (object.in ());

        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {

        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " when narrowing bound object in "
                             "CorbaServant::resolveAlpineQueryReference.");
        return false;
    }
    ExCatchAny {

        Log::Error ("Caught unexpected exception when narrowing bound object in "
                             "CorbaServant::resolveAlpineQueryReference.");
        return false;
    }
    ExEndTry;
    ExCheck;


    return true;
}



bool  
CorbaServant::resolveAlpineModuleMgmtReference (const string &                  bindingPath,
                                                Alpine::AlpineModuleMgmt_var &  reference)
{
#ifdef _VERBOSE
    Log::Debug ("CorbaServant::resolveAlpineModuleMgmtReference invoked."
                         "\nBinding Path: " + bindingPath +
                "\n");
#endif

    bool               status;
    CosNamingUtils     namingUtil;
    CORBA::Object_var  object;

    if (!namingUtil.bindingExists (bindingPath)) {

        // Error, we must have a context to initialize client interface.
        Log::Error ("AlpineModuleMgmt interface binding does not exist in naming service. "
                             "CorbaServant::resolveAlpineModuleMgmtReference failed.");

        return false;
    }

    status = namingUtil.resolve (bindingPath, object);

    if (!status) {
        Log::Error ("Resolve AlpineModuleMgmt interface binding failed. "
                             "CorbaServant::resolveAlpineModuleMgmtReference failed.");

        return false;
    }

    if (CORBA::is_nil (object.in())) {
        Log::Error ("AlpineModuleMgmt interface binding in naming service is NIL. "
                             "CorbaServant::resolveAlpineModuleMgmtReference failed.");

        return false;
    }


    // Verify bound object
    //
    ExNewEnv;

    ExTry {

        reference = Alpine::AlpineModuleMgmt::_narrow (object.in ());

        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {

        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " when narrowing bound object in "
                             "CorbaServant::resolveAlpineModuleMgmtReference.");
        return false;
    }
    ExCatchAny {

        Log::Error ("Caught unexpected exception when narrowing bound object in "
                             "CorbaServant::resolveAlpineModuleMgmtReference.");
        return false;
    }
    ExEndTry;
    ExCheck;


    return true;
}



