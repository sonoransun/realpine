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


#include <AlpineCorba_impl.h>
#include <NetUtils.h>
#include <Log.h>
#include <StringUtils.h>



AlpineCorba_impl::DtcpStackMgmt::DtcpStackMgmt (const CORBA::ORB_var &          orb,
                                                const PortableServer::POA_var & poa,
                                                AlpineCorba_impl *              implParent)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::DtcpStackMgmt constructor invoked.");
#endif

    orb_        = orb;
    poa_        = poa;
    implParent_ = implParent;
}



AlpineCorba_impl::DtcpStackMgmt::~DtcpStackMgmt ()
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::DtcpStackMgmt destructor invoked.");
#endif
}



void  
AlpineCorba_impl::DtcpStackMgmt::natDiscovery (CORBA::Boolean  isRequired,
                                               CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_DtcpStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::DtcpStackMgmt::natDiscovery invoked.");
#endif


}



CORBA::Boolean  
AlpineCorba_impl::DtcpStackMgmt::natDiscoveryRequired (CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_DtcpStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::DtcpStackMgmt::natDiscoveryRequired invoked.");
#endif

    CORBA::Boolean  retVal = 0;

    return retVal;
}



void  
AlpineCorba_impl::DtcpStackMgmt::setDataSendingLimit (uint  bpsLimit,
                                                      CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_DtcpStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::DtcpStackMgmt::setDataSendingLimit invoked.");
#endif


}



uint  
AlpineCorba_impl::DtcpStackMgmt::getDataSendingLimit (CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_DtcpStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::DtcpStackMgmt::getDataSendingLimit invoked.");
#endif

    uint  retVal = 0;

    return retVal;
}



void  
AlpineCorba_impl::DtcpStackMgmt::setStackThreadLimit (uint  threadLimit,
                                                      CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_DtcpStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::DtcpStackMgmt::setStackThreadLimit invoked.");
#endif


}



uint  
AlpineCorba_impl::DtcpStackMgmt::getStackThreadLimit (CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_DtcpStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::DtcpStackMgmt::getStackThreadLimit invoked.");
#endif

    uint  retVal = 0;

    return retVal;
}



void  
AlpineCorba_impl::DtcpStackMgmt::setReceiveBufferLimit (uint  recvBufferLimit,
                                                        CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_InvalidValue,
                  Alpine::e_DtcpStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::DtcpStackMgmt::setReceiveBufferLimit invoked.");
#endif


}



uint  
AlpineCorba_impl::DtcpStackMgmt::getReceiveBufferLimit (CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_DtcpStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::DtcpStackMgmt::getReceiveBufferLimit invoked.");
#endif

    uint  retVal = 0;

    return retVal;
}



void  
AlpineCorba_impl::DtcpStackMgmt::setSendBufferLimit (uint  sendBufferLimit,
                                                     CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_InvalidValue,
                  Alpine::e_DtcpStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::DtcpStackMgmt::setSendBufferLimit invoked.");
#endif


}



uint  
AlpineCorba_impl::DtcpStackMgmt::getSendBufferLimit (CORBA::Environment &ACE_TRY_ENV)
    ExThrowSpec ((CORBA::SystemException,
                  Alpine::e_DtcpStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::DtcpStackMgmt::getSendBufferLimit invoked.");
#endif

    uint  retVal = 0;

    return retVal;
}



