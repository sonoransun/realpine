///////
///
///  Copyright (C) 2026  sonoransun
///
///  Permission is hereby granted, free of charge, to any person obtaining a copy
///  of this software and associated documentation files (the "Software"), to deal
///  in the Software without restriction, including without limitation the rights
///  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
///  copies of the Software, and to permit persons to whom the Software is
///  furnished to do so, subject to the following conditions:
///
///  The above copyright notice and this permission notice shall be included in all
///  copies or substantial portions of the Software.
///
///  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
///  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
///  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
///  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
///  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
///  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
///  SOFTWARE.
///
///////


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



