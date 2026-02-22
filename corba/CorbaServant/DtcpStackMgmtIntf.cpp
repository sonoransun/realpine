/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <DtcpStackMgmtIntf.h>
#include <Log.h>
#include <StringUtils.h>



bool  
DtcpStackMgmtIntf::natDiscovery (bool isRequired)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpStackMgmtIntf::natDiscovery invoked.");
#endif

    ExNewEnv;

    CORBA::Boolean  corbaRequired;
    isRequired ? corbaRequired = 1 : corbaRequired = 0;

    ExTry {

        AlpineCorbaClient::DtcpStackMgmt::natDiscovery (corbaRequired, ExTryEnv);

        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {
        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " from AlpineCorbaClient::DtcpStackMgmt::natDiscovery in call to "
                             "DtcpStackMgmtIntf::natDiscovery.");
        return false;
    }
    ExCatchAny {
        Log::Error ("Caught unknown exception from "
                             "AlpineCorbaClient::DtcpStackMgmt::natDiscovery in call to "
                             "DtcpStackMgmtIntf::natDiscovery.");
        return false;
    }
    ExEndTry;
    ExCheck;


    return true;
}



bool  
DtcpStackMgmtIntf::natDiscoveryRequired ()
{
#ifdef _VERBOSE
    Log::Debug ("DtcpStackMgmtIntf::natDiscoveryRequired invoked.");
#endif

    ExNewEnv;

    CORBA::Boolean  retVal;

    ExTry {

        retVal = AlpineCorbaClient::DtcpStackMgmt::natDiscoveryRequired (ExTryEnv);

        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {
        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " from AlpineCorbaClient::DtcpStackMgmt::natDiscoveryRequired in call to "
                             "DtcpStackMgmtIntf::natDiscoveryRequired.");
        return false;
    }
    ExCatchAny {
        Log::Error ("Caught unknown exception from "
                             "AlpineCorbaClient::DtcpStackMgmt::natDiscoveryRequired in call to "
                             "DtcpStackMgmtIntf::natDiscoveryRequired.");
        return false;
    }
    ExEndTry;
    ExCheck;

    if (retVal == 0) {
        return false;
    }


    return true;
}



bool  
DtcpStackMgmtIntf::setDataSendingLimit (ulong limit)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpStackMgmtIntf::setDataSendingLimit invoked.");
#endif

    ExNewEnv;

    ExTry {

        AlpineCorbaClient::DtcpStackMgmt::setDataSendingLimit (limit, ExTryEnv);

        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {
        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " from AlpineCorbaClient::DtcpStackMgmt::setDataSendingLimit in call to "
                             "DtcpStackMgmtIntf::setDataSendingLimit.");
        return false;
    }
    ExCatchAny {
        Log::Error ("Caught unknown exception from "
                             "AlpineCorbaClient::DtcpStackMgmt::setDataSendingLimit in call to "
                             "DtcpStackMgmtIntf::setDataSendingLimit.");
        return false;
    }
    ExEndTry;
    ExCheck;


    return true;
}



bool  
DtcpStackMgmtIntf::getDataSendingLimit (ulong & limit)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpStackMgmtIntf::getDataSendingLimit invoked.");
#endif

    ExNewEnv;

    ExTry {

        limit = AlpineCorbaClient::DtcpStackMgmt::getDataSendingLimit (ExTryEnv);

        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {
        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " from AlpineCorbaClient::DtcpStackMgmt::getDataSendingLimit in call to "
                             "DtcpStackMgmtIntf::getDataSendingLimit.");
        return false;
    }
    ExCatchAny {
        Log::Error ("Caught unknown exception from "
                             "AlpineCorbaClient::DtcpStackMgmt::getDataSendingLimit in call to "
                             "DtcpStackMgmtIntf::getDataSendingLimit.");
        return false;
    }
    ExEndTry;
    ExCheck;


    return true;
}



bool  
DtcpStackMgmtIntf::setStackThreadLimit (ulong limit)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpStackMgmtIntf::setStackThreadLimit invoked.");
#endif

    ExNewEnv;

    ExTry {

        AlpineCorbaClient::DtcpStackMgmt::setStackThreadLimit (limit, ExTryEnv);

        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {
        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " from AlpineCorbaClient::DtcpStackMgmt::setStackThreadLimit in call to "
                             "DtcpStackMgmtIntf::setStackThreadLimit.");
        return false;
    }
    ExCatchAny {
        Log::Error ("Caught unknown exception from "
                             "AlpineCorbaClient::DtcpStackMgmt::setStackThreadLimit in call to "
                             "DtcpStackMgmtIntf::setStackThreadLimit.");
        return false;
    }
    ExEndTry;
    ExCheck;


    return true;
}



bool  
DtcpStackMgmtIntf::getStackThreadLimit (ulong & limit)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpStackMgmtIntf::getStackThreadLimit invoked.");
#endif

    ExNewEnv;

    ExTry {

        limit = AlpineCorbaClient::DtcpStackMgmt::getStackThreadLimit (ExTryEnv);

        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {
        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " from AlpineCorbaClient::DtcpStackMgmt::getStackThreadLimit in call to "
                             "DtcpStackMgmtIntf::getStackThreadLimit.");
        return false;
    }
    ExCatchAny {
        Log::Error ("Caught unknown exception from "
                             "AlpineCorbaClient::DtcpStackMgmt::getStackThreadLimit in call to "
                             "DtcpStackMgmtIntf::getStackThreadLimit.");
        return false;
    }
    ExEndTry;
    ExCheck;


    return true;
}



bool  
DtcpStackMgmtIntf::setReceiveBufferLimit (ulong limit)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpStackMgmtIntf::setReceiveBufferLimit invoked.");
#endif

    ExNewEnv;

    ExTry {

        AlpineCorbaClient::DtcpStackMgmt::setReceiveBufferLimit (limit, ExTryEnv);

        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {
        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " from AlpineCorbaClient::DtcpStackMgmt::setReceiveBufferLimit in call to "
                             "DtcpStackMgmtIntf::setReceiveBufferLimit.");
        return false;
    }
    ExCatchAny {
        Log::Error ("Caught unknown exception from "
                             "AlpineCorbaClient::DtcpStackMgmt::setReceiveBufferLimit in call to "
                             "DtcpStackMgmtIntf::setReceiveBufferLimit.");
        return false;
    }
    ExEndTry;
    ExCheck;


    return true;
}



bool  
DtcpStackMgmtIntf::getReceiveBufferLimit (ulong & limit)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpStackMgmtIntf::getReceiveBufferLimit invoked.");
#endif

    ExNewEnv;

    ExTry {

        limit = AlpineCorbaClient::DtcpStackMgmt::getReceiveBufferLimit (ExTryEnv);

        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {
        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " from AlpineCorbaClient::DtcpStackMgmt::getReceiveBufferLimit in call to "
                             "DtcpStackMgmtIntf::getReceiveBufferLimit.");
        return false;
    }
    ExCatchAny {
        Log::Error ("Caught unknown exception from "
                             "AlpineCorbaClient::DtcpStackMgmt::getReceiveBufferLimit in call to "
                             "DtcpStackMgmtIntf::getReceiveBufferLimit.");
        return false;
    }
    ExEndTry;
    ExCheck;


    return true;
}



bool  
DtcpStackMgmtIntf::setSendBufferLimit (ulong limit)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpStackMgmtIntf::setSendBufferLimit invoked.");
#endif

    ExNewEnv;

    ExTry {

        AlpineCorbaClient::DtcpStackMgmt::setSendBufferLimit (limit, ExTryEnv);

        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {
        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " from AlpineCorbaClient::DtcpStackMgmt::setSendBufferLimit in call to "
                             "DtcpStackMgmtIntf::setSendBufferLimit.");
        return false;
    }
    ExCatchAny {
        Log::Error ("Caught unknown exception from "
                             "AlpineCorbaClient::DtcpStackMgmt::setSendBufferLimit in call to "
                             "DtcpStackMgmtIntf::setSendBufferLimit.");
        return false;
    }
    ExEndTry;
    ExCheck;


    return true;
}



bool  
DtcpStackMgmtIntf::getSendBufferLimit (ulong & limit)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpStackMgmtIntf::getSendBufferLimit invoked.");
#endif

    ExNewEnv;

    ExTry {

        limit = AlpineCorbaClient::DtcpStackMgmt::getSendBufferLimit (ExTryEnv);

        ExTryCheck;
    }
    ExCatch (CORBA::Exception, ex) {
        Log::Error ("Caught corba exception: "s + std::string(ex._id()) +
                    " from AlpineCorbaClient::DtcpStackMgmt::getSendBufferLimit in call to "
                             "DtcpStackMgmtIntf::getSendBufferLimit.");
        return false;
    }
    ExCatchAny {
        Log::Error ("Caught unknown exception from "
                             "AlpineCorbaClient::DtcpStackMgmt::getSendBufferLimit in call to "
                             "DtcpStackMgmtIntf::getSendBufferLimit.");
        return false;
    }
    ExEndTry;
    ExCheck;


    return true;
}



