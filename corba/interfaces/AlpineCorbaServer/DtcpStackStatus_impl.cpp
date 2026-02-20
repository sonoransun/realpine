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



AlpineCorba_impl::DtcpStackStatus::DtcpStackStatus (const CORBA::ORB_var &          orb,
                                                   const PortableServer::POA_var & poa,
                                                   AlpineCorba_impl *              implParent)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::DtcpStackStatus constructor invoked.");
#endif

    orb_        = orb;
    poa_        = poa;
    implParent_ = implParent;
}



AlpineCorba_impl::DtcpStackStatus::~DtcpStackStatus ()
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::DtcpStackStatus destructor invoked.");
#endif
}



void  
AlpineCorba_impl::DtcpStackStatus::getBufferStats (Alpine::t_DtcpStackBufferStats_out   stats,
                                                   CORBA::Environment &ACE_TRY_ENV)
     ExThrowSpec ((CORBA::SystemException,
                   Alpine::e_DtcpStackError))
{
#ifdef _VERBOSE
    Log::Debug ("AlpineCorba_impl::DtcpStackStatus::getBufferStats invoked.");
#endif
}



