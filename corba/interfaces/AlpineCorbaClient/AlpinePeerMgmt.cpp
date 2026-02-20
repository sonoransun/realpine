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


#include <AlpineCorbaClient.h>
#include <ReadLock.h>
#include <WriteLock.h>
#include <Log.h>
#include <StringUtils.h>



void  
AlpineCorbaClient::AlpinePeerMgmt::getExtendedPeerList (Alpine::t_DtcpPeerIdList_out  peerIdList,
                                                        CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_AlpineStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaClient::AlpinePeerMgmt::getExtendedPeerList invoked.");
#endif

    ReadLock  lock(AlpineCorbaClient::dataLock_s);

    if (AlpineCorbaClient::!initialized_s) {
        Log::Error ("AlpinePeerMgmt method invoked before AlpineCorbaClient initialization!");
        CORBA::BAD_INV_ORDER  ex;
        ExThrow (ex);
        return;
    }

    AlpineCorbaClient::alpinePeerMgmtRef_s->getExtendedPeerList (peerIdList, ExTryEnv);
}



void  
AlpineCorbaClient::AlpinePeerMgmt::getPeerInformation (Alpine::t_AlpinePeerInfo_out  peerInfo,
                                                       CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_InvalidPeerId,
                  Alpine::e_AlpineStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaClient::AlpinePeerMgmt::getPeerInformation invoked.");
#endif

    ReadLock  lock(AlpineCorbaClient::dataLock_s);

    if (AlpineCorbaClient::!initialized_s) {
        Log::Error ("AlpinePeerMgmt method invoked before AlpineCorbaClient initialization!");
        CORBA::BAD_INV_ORDER  ex;
        ExThrow (ex);
        return;
    }

    AlpineCorbaClient::alpinePeerMgmtRef_s->getPeerInformation (peerInfo, ExTryEnv);
}



void 
AlpineCorbaClient::AlpinePeerMgmt::updatePeerInformation (const Alpine::t_AlpinePeerInfo &  peerInfo,
                                                          CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_InvalidPeerId,
                  Alpine::e_AlpineStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorbaClient::AlpinePeerMgmt::updatePeerInformation invoked.");
#endif

    ReadLock  lock(AlpineCorbaClient::dataLock_s);

    if (AlpineCorbaClient::!initialized_s) {
        Log::Error ("AlpinePeerMgmt method invoked before AlpineCorbaClient initialization!");
        CORBA::BAD_INV_ORDER  ex;
        ExThrow (ex);
        return;
    }

    AlpineCorbaClient::alpinePeerMgmtRef_s->updatePeerInformation (peerInfo, ExTryEnv);
}



