/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AlpineCorba_impl.h>
#include <Log.h>
#include <StringUtils.h>



AlpineCorba_impl::AlpineCorba_impl (const CORBA::ORB_var &          orb,
                                    const PortableServer::POA_var & poa)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl consturctor invoked.");
#endif

    orb_                    = orb;
    poa_                    = poa;
    dtcpPeerMgmtInterface_  = 0;
    initialized_            = false;
}



AlpineCorba_impl::~AlpineCorba_impl ()
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl destructor invoked.");
#endif

    delete dtcpPeerMgmtInterface_;

    delete dtcpStackMgmtInterface_;

    delete dtcpStackStatusInterface_;

    delete alpineStackMgmtInterface_;

    delete alpineStackStatusInterface_;

    delete alpineGroupMgmtInterface_;

    delete alpinePeerMgmtInterface_;

    delete alpineQueryInterface_;

    delete alpineModuleMgmtInterface_;
}



bool  
AlpineCorba_impl::initialize ()
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::initialize invoked.");
#endif

    ExNewEnv;

    ////
    //
    // Create implementation objects
    //
    PortableServer::ObjectId_var  objectId;
    CORBA::Object_var             object;


    // DtcpPeerMgmt
    // 
    ExTry {

        dtcpPeerMgmtInterface_ = new DtcpPeerMgmt (orb_, poa_, this);

        objectId = poa_->activate_object (dtcpPeerMgmtInterface_);

        object = dtcpPeerMgmtInterface_->_this ();

        if (CORBA::is_nil (object.in())) {
            Log::Error ("DtcpPeerMgmt activation returned a NIL object reference! "
                                 "AlpineCorba_impl::initialize failed.");
            return false;
        }
 
        dtcpPeerMgmtReference_ = Alpine::DtcpPeerMgmt::_narrow (object.in());

        if (CORBA::is_nil (dtcpPeerMgmtReference_.in())) {
            Log::Error ("DtcpPeerMgmt narrow returned a NIL reference! "
                                 "AlpineCorba_impl::initialize failed.");
            return false;
        }


        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {

        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " while creating DtcpPeerMgmt object.");
        return false;
    }
    ExCatchAny {
        
        Log::Error ("Caught unexpected exception while creating DtcpPeerMgmt object.");
        return false;
    }
    ExEndTry;
    ExCheck;



    // DtcpStackMgmt
    // 
    ExTry {

        PortableServer::ObjectId_var  objectId;

        dtcpStackMgmtInterface_ = new DtcpStackMgmt (orb_, poa_, this);

        objectId = poa_->activate_object (dtcpStackMgmtInterface_);

        object = dtcpStackMgmtInterface_->_this ();

        if (CORBA::is_nil (object.in())) {
            Log::Error ("DtcpStackMgmt activation returned a NIL object reference! "
                                 "AlpineCorba_impl::initialize failed.");
            return false;
        }
 
        dtcpStackMgmtReference_ = Alpine::DtcpStackMgmt::_narrow (object.in());

        if (CORBA::is_nil (dtcpStackMgmtReference_.in())) {
            Log::Error ("DtcpStackMgmt narrow returned a NIL reference! "
                                 "AlpineCorba_impl::initialize failed.");
            return false;
        }


        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {

        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " while creating DtcpStackMgmt object.");
        return false;
    }
    ExCatchAny {
        
        Log::Error ("Caught unexpected exception while creating DtcpStackMgmt object.");
        return false;
    }
    ExEndTry;
    ExCheck;



    // DtcpStackStatus
    // 
    ExTry {

        PortableServer::ObjectId_var  objectId;

        dtcpStackStatusInterface_ = new DtcpStackStatus (orb_, poa_, this);

        objectId = poa_->activate_object (dtcpStackStatusInterface_);

        object = dtcpStackStatusInterface_->_this ();

        if (CORBA::is_nil (object.in())) {
            Log::Error ("DtcpStackStatus activation returned a NIL object reference! "
                                 "AlpineCorba_impl::initialize failed.");
            return false;
        }
 
        dtcpStackStatusReference_ = Alpine::DtcpStackStatus::_narrow (object.in());

        if (CORBA::is_nil (dtcpStackStatusReference_.in())) {
            Log::Error ("DtcpStackStatus narrow returned a NIL reference! "
                                 "AlpineCorba_impl::initialize failed.");
            return false;
        }


        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {

        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " while creating DtcpStackStatus object.");
        return false;
    }
    ExCatchAny {
        
        Log::Error ("Caught unexpected exception while creating DtcpStackStatus object.");
        return false;
    }
    ExEndTry;
    ExCheck;



    // AlpineStackMgmt
    // 
    ExTry {

        PortableServer::ObjectId_var  objectId;

        alpineStackMgmtInterface_ = new AlpineStackMgmt (orb_, poa_, this);

        objectId = poa_->activate_object (alpineStackMgmtInterface_);

        object = alpineStackMgmtInterface_->_this ();

        if (CORBA::is_nil (object.in())) {
            Log::Error ("AlpineStackMgmt activation returned a NIL object reference! "
                                 "AlpineCorba_impl::initialize failed.");
            return false;
        }
 
        alpineStackMgmtReference_ = Alpine::AlpineStackMgmt::_narrow (object.in());

        if (CORBA::is_nil (alpineStackMgmtReference_.in())) {
            Log::Error ("AlpineStackMgmt narrow returned a NIL reference! "
                                 "AlpineCorba_impl::initialize failed.");
            return false;
        }


        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {

        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " while creating AlpineStackMgmt object.");
        return false;
    }
    ExCatchAny {
        
        Log::Error ("Caught unexpected exception while creating AlpineStackMgmt object.");
        return false;
    }
    ExEndTry;
    ExCheck;



    // AlpineStackStatus
    // 
    ExTry {

        PortableServer::ObjectId_var  objectId;

        alpineStackStatusInterface_ = new AlpineStackStatus (orb_, poa_, this);

        objectId = poa_->activate_object (alpineStackStatusInterface_);

        object = alpineStackStatusInterface_->_this ();

        if (CORBA::is_nil (object.in())) {
            Log::Error ("AlpineStackStatus activation returned a NIL object reference! "
                                 "AlpineCorba_impl::initialize failed.");
            return false;
        }
 
        alpineStackStatusReference_ = Alpine::AlpineStackStatus::_narrow (object.in());

        if (CORBA::is_nil (alpineStackStatusReference_.in())) {
            Log::Error ("AlpineStackStatus narrow returned a NIL reference! "
                                 "AlpineCorba_impl::initialize failed.");
            return false;
        }


        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {

        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " while creating AlpineStackStatus object.");
        return false;
    }
    ExCatchAny {
        
        Log::Error ("Caught unexpected exception while creating AlpineStackStatus object.");
        return false;
    }
    ExEndTry;
    ExCheck;



    // AlpineGroupMgmt
    // 
    ExTry {

        PortableServer::ObjectId_var  objectId;

        alpineGroupMgmtInterface_ = new AlpineGroupMgmt (orb_, poa_, this);

        objectId = poa_->activate_object (alpineGroupMgmtInterface_);

        object = alpineGroupMgmtInterface_->_this ();

        if (CORBA::is_nil (object.in())) {
            Log::Error ("AlpineGroupMgmt activation returned a NIL object reference! "
                                 "AlpineCorba_impl::initialize failed.");
            return false;
        }
 
        alpineGroupMgmtReference_ = Alpine::AlpineGroupMgmt::_narrow (object.in());

        if (CORBA::is_nil (alpineGroupMgmtReference_.in())) {
            Log::Error ("AlpineGroupMgmt narrow returned a NIL reference! "
                                 "AlpineCorba_impl::initialize failed.");
            return false;
        }


        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {

        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " while creating AlpineGroupMgmt object.");
        return false;
    }
    ExCatchAny {
        
        Log::Error ("Caught unexpected exception while creating AlpineGroupMgmt object.");
        return false;
    }
    ExEndTry;
    ExCheck;



    // AlpinePeerMgmt
    // 
    ExTry {

        PortableServer::ObjectId_var  objectId;

        alpinePeerMgmtInterface_ = new AlpinePeerMgmt (orb_, poa_, this);

        objectId = poa_->activate_object (alpinePeerMgmtInterface_);

        object = alpinePeerMgmtInterface_->_this ();

        if (CORBA::is_nil (object.in())) {
            Log::Error ("AlpinePeerMgmt activation returned a NIL object reference! "
                                 "AlpineCorba_impl::initialize failed.");
            return false;
        }
 
        alpinePeerMgmtReference_ = Alpine::AlpinePeerMgmt::_narrow (object.in());

        if (CORBA::is_nil (alpinePeerMgmtReference_.in())) {
            Log::Error ("AlpinePeerMgmt narrow returned a NIL reference! "
                                 "AlpineCorba_impl::initialize failed.");
            return false;
        }


        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {

        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " while creating AlpinePeerMgmt object.");
        return false;
    }
    ExCatchAny {
        
        Log::Error ("Caught unexpected exception while creating AlpinePeerMgmt object.");
        return false;
    }
    ExEndTry;
    ExCheck;



    // AlpineQuery
    // 
    ExTry {

        PortableServer::ObjectId_var  objectId;

        alpineQueryInterface_ = new AlpineQuery (orb_, poa_, this);

        objectId = poa_->activate_object (alpineQueryInterface_);

        object = alpineQueryInterface_->_this ();

        if (CORBA::is_nil (object.in())) {
            Log::Error ("AlpineQuery activation returned a NIL object reference! "
                                 "AlpineCorba_impl::initialize failed.");
            return false;
        }
 
        alpineQueryReference_ = Alpine::AlpineQuery::_narrow (object.in());

        if (CORBA::is_nil (alpineQueryReference_.in())) {
            Log::Error ("AlpineQuery narrow returned a NIL reference! "
                                 "AlpineCorba_impl::initialize failed.");
            return false;
        }


        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {

        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " while creating AlpineQuery object.");
        return false;
    }
    ExCatchAny {
        
        Log::Error ("Caught unexpected exception while creating AlpineQuery object.");
        return false;
    }
    ExEndTry;
    ExCheck;



    // AlpineModuleMgmt
    //
    ExTry {

        PortableServer::ObjectId_var  objectId;

        alpineModuleMgmtInterface_ = new AlpineModuleMgmt (orb_, poa_, this);

        objectId = poa_->activate_object (alpineModuleMgmtInterface_);

        object = alpineModuleMgmtInterface_->_this ();

        if (CORBA::is_nil (object.in())) {
            Log::Error ("AlpineModuleMgmt activation returned a NIL object reference! "
                                 "AlpineCorba_impl::initialize failed.");
            return false;
        }

        alpineModuleMgmtReference_ = Alpine::AlpineModuleMgmt::_narrow (object.in());

        if (CORBA::is_nil (alpineModuleMgmtReference_.in())) {
            Log::Error ("AlpineModuleMgmt narrow returned a NIL reference! "
                                 "AlpineCorba_impl::initialize failed.");
            return false;
        }


        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {

        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " while creating AlpineModuleMgmt object.");
        return false;
    }
    ExCatchAny {

        Log::Error ("Caught unexpected exception while creating AlpineModuleMgmt object.");
        return false;
    }
    ExEndTry;
    ExCheck;



    return true;
}



bool  
AlpineCorba_impl::getDtcpPeerMgmt (Alpine::DtcpPeerMgmt_var &  dtcpPeerMgmt)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::getDtcpPeerMgmt invoked.");
#endif

    dtcpPeerMgmt = dtcpPeerMgmtReference_;

    return true;
}



bool
AlpineCorba_impl::getDtcpStackMgmt (Alpine::DtcpStackMgmt_var &  dtcpStackMgmt)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::getDtcpStackMgmt invoked.");
#endif

    dtcpStackMgmt = dtcpStackMgmtReference_;

    return true;
}



bool
AlpineCorba_impl::getDtcpStackStatus (Alpine::DtcpStackStatus_var &  dtcpStackStatus)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::getDtcpStackStatus invoked.");
#endif

    dtcpStackStatus = dtcpStackStatusReference_;

    return true;
}



bool
AlpineCorba_impl::getAlpineStackMgmt (Alpine::AlpineStackMgmt_var &  alpineStackMgmt)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::getAlpineStackMgmt invoked.");
#endif

    alpineStackMgmt = alpineStackMgmtReference_;

    return true;
}



bool
AlpineCorba_impl::getAlpineStackStatus (Alpine::AlpineStackStatus_var &  alpineStackStatus)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::getAlpineStackStatus invoked.");
#endif

    alpineStackStatus = alpineStackStatusReference_;

    return true;
}



bool
AlpineCorba_impl::getAlpineGroupMgmt (Alpine::AlpineGroupMgmt_var &  alpineGroupMgmt)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::getAlpineGroupMgmt invoked.");
#endif

    alpineGroupMgmt = alpineGroupMgmtReference_;

    return true;
}



bool
AlpineCorba_impl::getAlpinePeerMgmt (Alpine::AlpinePeerMgmt_var &  alpinePeerMgmt)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::getAlpinePeerMgmt invoked.");
#endif

    alpinePeerMgmt = alpinePeerMgmtReference_;

    return true;
}



bool
AlpineCorba_impl::getAlpineQuery (Alpine::AlpineQuery_var &  alpineQuery)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::getAlpineQuery invoked.");
#endif

    alpineQuery = alpineQueryReference_;

    return true;
}



bool
AlpineCorba_impl::getAlpineModuleMgmt (Alpine::AlpineModuleMgmt_var &  alpineModuleMgmt)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::getAlpineModuleMgmt invoked.");
#endif

    alpineModuleMgmt = alpineModuleMgmtReference_;

    return true;
}



