/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <CorbaAdmin.h>
#include <AlpineCorbaServer.h>
#include <CosNamingUtils.h>
#include <ReadLock.h>
#include <WriteLock.h>
#include <Log.h>
#include <StringUtils.h>



CORBA::ORB_var                  CorbaAdmin::orb_s;
PortableServer::POA_var         CorbaAdmin::rootPoa_s;
bool                            CorbaAdmin::initialized_s = false;
bool                            CorbaAdmin::alpineCorbaServerActive_s = false;
ReadWriteSem                    CorbaAdmin::dataLock_s;



// Ctor defaulted in header


// Dtor defaulted in header



bool  
CorbaAdmin::initialize (int      argc,
                        char **  argv)
{
#ifdef _VERBOSE
    Log::Debug ("CorbaAdmin::initialize invoked.");
#endif

    WriteLock  lock(dataLock_s);


    bool status;

    // Initialize ORB
    //
    status = OrbUtils::initialize (argc, argv);

    if (!status) {
        Log::Error ("OrbUtils initialization failed in CorbaAdmin::initialize!");
        return false;
    }
    status = OrbUtils::getOrb (orb_s);

    if (!status) {
        Log::Error ("Getting ORB reference failed in CorbaAdmin::initialize!");
        return false;
    }
    if (CORBA::is_nil (orb_s.in())) {
        Log::Error ("NIL ORB reference returned CorbaAdmin::initialize!");
        return false;
    }
    status = OrbUtils::getRootPoa (rootPoa_s);

    if (!status) {
        Log::Error ("Getting root POA reference failed in CorbaAdmin::initialize!");
        return false;
    }

    if (CORBA::is_nil (rootPoa_s.in())) {
        Log::Error ("NIL root POA reference returned CorbaAdmin::initialize!");
        return false;
    }


    // Initialize CosNamingUtils
    //
    status = CosNamingUtils::initialize (orb_s);

    if (!status) {
        Log::Error ("CosNamingUtils initialization failed in CorbaAdmin::initialize!");
        return false;
    }

    initialized_s = true;


    return true;
}


                             
bool  
CorbaAdmin::activateAlpineCorbaServer (const string &  namingContextPath)
{
#ifdef _VERBOSE
    Log::Debug ("CorbaAdmin::activateAlpineCorbaServer invoked.");
#endif

    CORBA::ORB_var           orb;
    PortableServer::POA_var  rootPoa;

    // scope lock
    {
        ReadLock  lock(dataLock_s);

        if (!initialized_s) {
            Log::Error ("Attempt to activate corba interface without initialization in "
                                 "CorbaAdmin::activateAlpineCorbaServer.");

            return false;
        }
        if (alpineCorbaServerActive_s) {
            Log::Error ("Corba interface already activated in "
                                 "CorbaAdmin::activateAlpineCorbaServer.");

            return false;
        }
        // make temp copy of ORB reference so we can release lock
        //
        orb     = orb_s;
        rootPoa = rootPoa_s;
    }


    // Create Alpine interface instance and activate
    //
    bool status;
    status = AlpineCorbaServer::initialize (orb, rootPoa);

    if (!status) {
        Log::Error ("Attempt to initialize AlpineCorbaServer failed in "
                             "CorbaAdmin::activateAlpineCorbaServer!");

        return false;
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


    // Register interfaces in the naming service
    //
    string currentBinding;

    // DtcpPeerMgmt
    //
    currentBinding = fullContextPath + "DtcpPeerMgmtInterface";
    status = bindDtcpPeerMgmtReference (currentBinding);

    if (!status) {
        return status;
    }

    // DtcpStackMgmt
    //
    currentBinding = fullContextPath + "DtcpStackMgmtInterface";
    status = bindDtcpStackMgmtReference (currentBinding);

    if (!status) {
        return status;
    }

    // DtcpStackStatus
    //
    currentBinding = fullContextPath + "DtcpStackStatusInterface";
    status = bindDtcpStackStatusReference (currentBinding);

    if (!status) {
        return status;
    }

    // AlpineStackMgmt
    //
    currentBinding = fullContextPath + "AlpineStackMgmtInterface";
    status = bindAlpineStackMgmtReference (currentBinding);

    if (!status) {
        return status;
    }

    // AlpineStackStatus
    //
    currentBinding = fullContextPath + "AlpineStackStatusInterface";
    status = bindAlpineStackStatusReference (currentBinding);

    if (!status) {
        return status;
    }

    // AlpineGroupMgmt
    //
    currentBinding = fullContextPath + "AlpineGroupMgmtInterface";
    status = bindAlpineGroupMgmtReference (currentBinding);

    if (!status) {
        return status;
    }

    // AlpinePeerMgmt
    //
    currentBinding = fullContextPath + "AlpinePeerMgmtInterface";
    status = bindAlpinePeerMgmtReference (currentBinding);

    if (!status) {
        return status;
    }

    // AlpineQuery
    //
    currentBinding = fullContextPath + "AlpineQueryInterface";
    status = bindAlpineQueryReference (currentBinding);

    if (!status) {
        return status;
    }

    // AlpineModuleMgmt
    //
    currentBinding = fullContextPath + "AlpineModuleMgmtInterface";
    status = bindAlpineModuleMgmtReference (currentBinding);

    if (!status) {
        return status;
    }



    // Finished...
    //
    {
        WriteLock  lock(dataLock_s);
        alpineCorbaServerActive_s = true;
    }


    return true;
}

                             

bool  
CorbaAdmin::bindDtcpPeerMgmtReference (const string & bindingPath)
{
#ifdef _VERBOSE
    Log::Debug ("CorbaAdmin::bindDtcpPeerMgmtReference invoked."
                         "\nBinding Path: " + bindingPath +
                "\n");
#endif

    Alpine::DtcpPeerMgmt_var  interfaceReference;
    bool status;

    status = AlpineCorbaServer::getDtcpPeerMgmt (interfaceReference);

    if (!status) {
        Log::Error ("Unable to retreive interface reference in "
                             "CorbaAdmin::bindDtcpPeerMgmtReference!");
        return false;
    }
    if (CORBA::is_nil (interfaceReference.in())) {
        Log::Error ("NIL interface reference returned in "
                             "CorbaAdmin::bindDtcpPeerMgmtReference!");
        return false;
    }
    CosNamingUtils  namingUtil;
    CORBA::Object_var  object;

    object = CORBA::Object::_duplicate (interfaceReference.in());

    if (CORBA::is_nil (object.in())) {
        Log::Error ("NIL upcast Object reference returned in "
                             "CorbaAdmin::bindDtcpPeerMgmtReference!");
        return false;
    }
    if (namingUtil.bindingExists (bindingPath)) {

        // binding exists, perform an update
        //
        status = namingUtil.updateBinding (bindingPath, object);
    }
    else {

        // binding does not exist, perform a create
        //
        status = namingUtil.addBinding (bindingPath, object);
    }

    if (!status) {
        Log::Error ("Naming service binding failed in "
                             "CorbaAdmin::bindDtcpPeerMgmtReference!");
        return false;
    }
    return true;
}



bool  
CorbaAdmin::bindDtcpStackMgmtReference (const string & bindingPath)
{
#ifdef _VERBOSE
    Log::Debug ("CorbaAdmin::bindDtcpStackMgmtReference invoked."
                         "\nBinding Path: " + bindingPath +
                "\n");
#endif

    Alpine::DtcpStackMgmt_var  interfaceReference;
    bool status;

    status = AlpineCorbaServer::getDtcpStackMgmt (interfaceReference);

    if (!status) {
        Log::Error ("Unable to retreive interface reference in "
                             "CorbaAdmin::bindDtcpStackMgmtReference!");
        return false;
    }
    if (CORBA::is_nil (interfaceReference.in())) {
        Log::Error ("NIL interface reference returned in "
                             "CorbaAdmin::bindDtcpStackMgmtReference!");
        return false;
    }
    CosNamingUtils  namingUtil;
    CORBA::Object_var  object;

    object = CORBA::Object::_duplicate (interfaceReference.in());

    if (CORBA::is_nil (object.in())) {
        Log::Error ("NIL upcast Object reference returned in "
                             "CorbaAdmin::bindDtcpStackMgmtReference!");
        return false;
    }
    if (namingUtil.bindingExists (bindingPath)) {

        // binding exists, perform an update
        //
        status = namingUtil.updateBinding (bindingPath, object);
    }
    else {

        // binding does not exist, perform a create
        //
        status = namingUtil.addBinding (bindingPath, object);
    }

    if (!status) {
        Log::Error ("Naming service binding failed in "
                             "CorbaAdmin::bindDtcpStackMgmtReference!");
        return false;
    }
    return true;
}



bool  
CorbaAdmin::bindDtcpStackStatusReference (const string & bindingPath)
{
#ifdef _VERBOSE
    Log::Debug ("CorbaAdmin::bindDtcpStackStatusReference invoked."
                         "\nBinding Path: " + bindingPath +
                "\n");
#endif

    Alpine::DtcpStackStatus_var  interfaceReference;
    bool status;

    status = AlpineCorbaServer::getDtcpStackStatus (interfaceReference);

    if (!status) {
        Log::Error ("Unable to retreive interface reference in "
                             "CorbaAdmin::bindDtcpStackStatusReference!");
        return false;
    }
    if (CORBA::is_nil (interfaceReference.in())) {
        Log::Error ("NIL interface reference returned in "
                             "CorbaAdmin::bindDtcpStackStatusReference!");
        return false;
    }
    CosNamingUtils  namingUtil;
    CORBA::Object_var  object;

    object = CORBA::Object::_duplicate (interfaceReference.in());

    if (CORBA::is_nil (object.in())) {
        Log::Error ("NIL upcast Object reference returned in "
                             "CorbaAdmin::bindDtcpStackStatusReference!");
        return false;
    }
    if (namingUtil.bindingExists (bindingPath)) {

        // binding exists, perform an update
        //
        status = namingUtil.updateBinding (bindingPath, object);
    }
    else {

        // binding does not exist, perform a create
        //
        status = namingUtil.addBinding (bindingPath, object);
    }

    if (!status) {
        Log::Error ("Naming service binding failed in "
                             "CorbaAdmin::bindDtcpStackStatusReference!");
        return false;
    }
    return true;
}



bool  
CorbaAdmin::bindAlpineStackMgmtReference (const string & bindingPath)
{
#ifdef _VERBOSE
    Log::Debug ("CorbaAdmin::bindAlpineStackMgmtReference invoked."
                         "\nBinding Path: " + bindingPath +
                "\n");
#endif

    Alpine::AlpineStackMgmt_var  interfaceReference;
    bool status;

    status = AlpineCorbaServer::getAlpineStackMgmt (interfaceReference);

    if (!status) {
        Log::Error ("Unable to retreive interface reference in "
                             "CorbaAdmin::bindAlpineStackMgmtReference!");
        return false;
    }
    if (CORBA::is_nil (interfaceReference.in())) {
        Log::Error ("NIL interface reference returned in "
                             "CorbaAdmin::bindAlpineStackMgmtReference!");
        return false;
    }
    CosNamingUtils  namingUtil;
    CORBA::Object_var  object;

    object = CORBA::Object::_duplicate (interfaceReference.in());

    if (CORBA::is_nil (object.in())) {
        Log::Error ("NIL upcast Object reference returned in "
                             "CorbaAdmin::bindAlpineStackMgmtReference!");
        return false;
    }
    if (namingUtil.bindingExists (bindingPath)) {

        // binding exists, perform an update
        //
        status = namingUtil.updateBinding (bindingPath, object);
    }
    else {

        // binding does not exist, perform a create
        //
        status = namingUtil.addBinding (bindingPath, object);
    }

    if (!status) {
        Log::Error ("Naming service binding failed in "
                             "CorbaAdmin::bindAlpineStackMgmtReference!");
        return false;
    }
    return true;
}



bool  
CorbaAdmin::bindAlpineStackStatusReference (const string & bindingPath)
{
#ifdef _VERBOSE
    Log::Debug ("CorbaAdmin::bindAlpineStackStatusReference invoked."
                         "\nBinding Path: " + bindingPath +
                "\n");
#endif

    Alpine::AlpineStackStatus_var  interfaceReference;
    bool status;

    status = AlpineCorbaServer::getAlpineStackStatus (interfaceReference);

    if (!status) {
        Log::Error ("Unable to retreive interface reference in "
                             "CorbaAdmin::bindAlpineStackStatusReference!");
        return false;
    }
    if (CORBA::is_nil (interfaceReference.in())) {
        Log::Error ("NIL interface reference returned in "
                             "CorbaAdmin::bindAlpineStackStatusReference!");
        return false;
    }
    CosNamingUtils  namingUtil;
    CORBA::Object_var  object;

    object = CORBA::Object::_duplicate (interfaceReference.in());

    if (CORBA::is_nil (object.in())) {
        Log::Error ("NIL upcast Object reference returned in "
                             "CorbaAdmin::bindAlpineStackStatusReference!");
        return false;
    }
    if (namingUtil.bindingExists (bindingPath)) {

        // binding exists, perform an update
        //
        status = namingUtil.updateBinding (bindingPath, object);
    }
    else {

        // binding does not exist, perform a create
        //
        status = namingUtil.addBinding (bindingPath, object);
    }

    if (!status) {
        Log::Error ("Naming service binding failed in "
                             "CorbaAdmin::bindAlpineStackStatusReference!");
        return false;
    }
    return true;
}



bool  
CorbaAdmin::bindAlpineGroupMgmtReference (const string & bindingPath)
{
#ifdef _VERBOSE
    Log::Debug ("CorbaAdmin::bindAlpineGroupMgmtReference invoked."
                         "\nBinding Path: " + bindingPath +
                "\n");
#endif

    Alpine::AlpineGroupMgmt_var  interfaceReference;
    bool status;

    status = AlpineCorbaServer::getAlpineGroupMgmt (interfaceReference);

    if (!status) {
        Log::Error ("Unable to retreive interface reference in "
                             "CorbaAdmin::bindAlpineGroupMgmtReference!");
        return false;
    }
    if (CORBA::is_nil (interfaceReference.in())) {
        Log::Error ("NIL interface reference returned in "
                             "CorbaAdmin::bindAlpineGroupMgmtReference!");
        return false;
    }
    CosNamingUtils  namingUtil;
    CORBA::Object_var  object;

    object = CORBA::Object::_duplicate (interfaceReference.in());

    if (CORBA::is_nil (object.in())) {
        Log::Error ("NIL upcast Object reference returned in "
                             "CorbaAdmin::bindAlpineGroupMgmtReference!");
        return false;
    }
    if (namingUtil.bindingExists (bindingPath)) {

        // binding exists, perform an update
        //
        status = namingUtil.updateBinding (bindingPath, object);
    }
    else {

        // binding does not exist, perform a create
        //
        status = namingUtil.addBinding (bindingPath, object);
    }

    if (!status) {
        Log::Error ("Naming service binding failed in "
                             "CorbaAdmin::bindAlpineGroupMgmtReference!");
        return false;
    }
    return true;
}



bool  
CorbaAdmin::bindAlpinePeerMgmtReference (const string & bindingPath)
{
#ifdef _VERBOSE
    Log::Debug ("CorbaAdmin::bindAlpinePeerMgmtReference invoked."
                         "\nBinding Path: " + bindingPath +
                "\n");
#endif

    Alpine::AlpinePeerMgmt_var  interfaceReference;
    bool status;

    status = AlpineCorbaServer::getAlpinePeerMgmt (interfaceReference);

    if (!status) {
        Log::Error ("Unable to retreive interface reference in "
                             "CorbaAdmin::bindAlpinePeerMgmtReference!");
        return false;
    }
    if (CORBA::is_nil (interfaceReference.in())) {
        Log::Error ("NIL interface reference returned in "
                             "CorbaAdmin::bindAlpinePeerMgmtReference!");
        return false;
    }
    CosNamingUtils  namingUtil;
    CORBA::Object_var  object;

    object = CORBA::Object::_duplicate (interfaceReference.in());

    if (CORBA::is_nil (object.in())) {
        Log::Error ("NIL upcast Object reference returned in "
                             "CorbaAdmin::bindAlpinePeerMgmtReference!");
        return false;
    }
    if (namingUtil.bindingExists (bindingPath)) {

        // binding exists, perform an update
        //
        status = namingUtil.updateBinding (bindingPath, object);
    }
    else {

        // binding does not exist, perform a create
        //
        status = namingUtil.addBinding (bindingPath, object);
    }

    if (!status) {
        Log::Error ("Naming service binding failed in "
                             "CorbaAdmin::bindAlpinePeerMgmtReference!");
        return false;
    }
    return true;
}



bool  
CorbaAdmin::bindAlpineQueryReference (const string & bindingPath)
{
#ifdef _VERBOSE
    Log::Debug ("CorbaAdmin::bindAlpineQueryReference invoked."
                         "\nBinding Path: " + bindingPath +
                "\n");
#endif

    Alpine::AlpineQuery_var  interfaceReference;
    bool status;

    status = AlpineCorbaServer::getAlpineQuery (interfaceReference);

    if (!status) {
        Log::Error ("Unable to retreive interface reference in "
                             "CorbaAdmin::bindAlpineQueryReference!");
        return false;
    }
    if (CORBA::is_nil (interfaceReference.in())) {
        Log::Error ("NIL interface reference returned in "
                             "CorbaAdmin::bindAlpineQueryReference!");
        return false;
    }
    CosNamingUtils  namingUtil;
    CORBA::Object_var  object;

    object = CORBA::Object::_duplicate (interfaceReference.in());

    if (CORBA::is_nil (object.in())) {
        Log::Error ("NIL upcast Object reference returned in "
                             "CorbaAdmin::bindAlpineQueryReference!");
        return false;
    }
    if (namingUtil.bindingExists (bindingPath)) {

        // binding exists, perform an update
        //
        status = namingUtil.updateBinding (bindingPath, object);
    }
    else {

        // binding does not exist, perform a create
        //
        status = namingUtil.addBinding (bindingPath, object);
    }

    if (!status) {
        Log::Error ("Naming service binding failed in "
                             "CorbaAdmin::bindAlpineQueryReference!");
        return false;
    }
    return true;
}



bool  
CorbaAdmin::bindAlpineModuleMgmtReference (const string & bindingPath)
{
#ifdef _VERBOSE
    Log::Debug ("CorbaAdmin::bindAlpineModuleMgmtReference invoked."
                         "\nBinding Path: " + bindingPath +
                "\n");
#endif

    Alpine::AlpineModuleMgmt_var  interfaceReference;
    bool status;

    status = AlpineCorbaServer::getAlpineModuleMgmt (interfaceReference);

    if (!status) {
        Log::Error ("Unable to retreive interface reference in "
                             "CorbaAdmin::bindAlpineModuleMgmtReference!");
        return false;
    }
    if (CORBA::is_nil (interfaceReference.in())) {
        Log::Error ("NIL interface reference returned in "
                             "CorbaAdmin::bindAlpineModuleMgmtReference!");
        return false;
    }
    CosNamingUtils  namingUtil;
    CORBA::Object_var  object;

    object = CORBA::Object::_duplicate (interfaceReference.in());

    if (CORBA::is_nil (object.in())) {
        Log::Error ("NIL upcast Object reference returned in "
                             "CorbaAdmin::bindAlpineModuleMgmtReference!");
        return false;
    }
    if (namingUtil.bindingExists (bindingPath)) {

        // binding exists, perform an update
        //
        status = namingUtil.updateBinding (bindingPath, object);
    }
    else {

        // binding does not exist, perform a create
        //
        status = namingUtil.addBinding (bindingPath, object);
    }

    if (!status) {
        Log::Error ("Naming service binding failed in "
                             "CorbaAdmin::bindAlpineModuleMgmtReference!");
        return false;
    }
    return true;
}



