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



