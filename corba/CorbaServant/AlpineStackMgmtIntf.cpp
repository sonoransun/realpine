/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AlpineStackMgmtIntf.h>
#include <Log.h>
#include <StringUtils.h>



bool  
AlpineStackMgmtIntf::setTotalTransferLimit (ulong limit)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineStackMgmtIntf::setTotalTransferLimit invoked.");
#endif

    ExNewEnv;

    ExTry {

        AlpineCorbaClient::AlpineStackMgmt::setTotalTransferLimit (limit, ExTryEnv);

        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {
        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " from AlpineCorbaClient::AlpineStackMgmt::setTotalTransferLimit in call to "
                             "AlpineStackMgmtIntf::setTotalTransferLimit.");
        return false;
    }
    ExCatchAny {
        Log::Error ("Caught unknown exception from "
                             "AlpineCorbaClient::AlpineStackMgmt::setTotalTransferLimit in call to "
                             "AlpineStackMgmtIntf::setTotalTransferLimit.");
        return false;
    }
    ExEndTry;
    ExCheck;


    return true;
}



bool  
AlpineStackMgmtIntf::getTotalTransferLimit (ulong & limit)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineStackMgmtIntf::getTotalTransferLimit invoked.");
#endif

    ExNewEnv;

    ExTry {

        limit = AlpineCorbaClient::AlpineStackMgmt::getTotalTransferLimit (ExTryEnv);

        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {
        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " from AlpineCorbaClient::AlpineStackMgmt::getTotalTransferLimit in call to "
                             "AlpineStackMgmtIntf::getTotalTransferLimit.");
        return false;
    }
    ExCatchAny {
        Log::Error ("Caught unknown exception from "
                             "AlpineCorbaClient::AlpineStackMgmt::getTotalTransferLimit in call to "
                             "AlpineStackMgmtIntf::getTotalTransferLimit.");
        return false;
    }
    ExEndTry;
    ExCheck;


    return true;
}



bool  
AlpineStackMgmtIntf::setPeerTransferLimit (ulong limit)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineStackMgmtIntf::setPeerTransferLimit invoked.");
#endif

    ExNewEnv;

    ExTry {

        AlpineCorbaClient::AlpineStackMgmt::setPeerTransferLimit (limit, ExTryEnv);

        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {
        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " from AlpineCorbaClient::AlpineStackMgmt::setPeerTransferLimit in call to "
                             "AlpineStackMgmtIntf::setPeerTransferLimit.");
        return false;
    }
    ExCatchAny {
        Log::Error ("Caught unknown exception from "
                             "AlpineCorbaClient::AlpineStackMgmt::setPeerTransferLimit in call to "
                             "AlpineStackMgmtIntf::setPeerTransferLimit.");
        return false;
    }
    ExEndTry;
    ExCheck;


    return true;
}



bool  
AlpineStackMgmtIntf::getPeerTransferLimit (ulong & limit)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineStackMgmtIntf::getPeerTransferLimit invoked.");
#endif

    ExNewEnv;

    ExTry {

        limit = AlpineCorbaClient::AlpineStackMgmt::getPeerTransferLimit (ExTryEnv);

        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {
        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " from AlpineCorbaClient::AlpineStackMgmt::getPeerTransferLimit in call to "
                             "AlpineStackMgmtIntf::getPeerTransferLimit.");
        return false;
    }
    ExCatchAny {
        Log::Error ("Caught unknown exception from "
                             "AlpineCorbaClient::AlpineStackMgmt::getPeerTransferLimit in call to "
                             "AlpineStackMgmtIntf::getPeerTransferLimit.");
        return false;
    }
    ExEndTry;
    ExCheck;


    return true;
}



