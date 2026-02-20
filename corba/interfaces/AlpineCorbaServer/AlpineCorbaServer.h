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


#pragma once
#include <Common.h>
#include <CorbaUtils.h>
#include <AlpineC.h>
#include <ReadWriteSem.h>


class AlpineCorba_impl;


class AlpineCorbaServer
{
  public:

    AlpineCorbaServer () = default;
    ~AlpineCorbaServer () = default;



    static bool  initialize (const CORBA::ORB_var &           orb,
                             const PortableServer::POA_var &  poa);


    static bool  getDtcpPeerMgmt (Alpine::DtcpPeerMgmt_var &  dtcpPeerMgmt);

    static bool  getDtcpStackMgmt (Alpine::DtcpStackMgmt_var &  dtcpStackMgmt);

    static bool  getDtcpStackStatus (Alpine::DtcpStackStatus_var &  dtcpStackStatus);

    static bool  getAlpineStackMgmt (Alpine::AlpineStackMgmt_var &  alpineStackMgmt);

    static bool  getAlpineStackStatus (Alpine::AlpineStackStatus_var &  alpineStackStatus);

    static bool  getAlpineGroupMgmt (Alpine::AlpineGroupMgmt_var &  alpineGroupMgmt);

    static bool  getAlpinePeerMgmt (Alpine::AlpinePeerMgmt_var &  alpinePeerMgmt);

    static bool  getAlpineQuery (Alpine::AlpineQuery_var &  alpineQuery);

    static bool  getAlpineModuleMgmt (Alpine::AlpineModuleMgmt_var &  alpineModuleMgmt);



  private:

    static AlpineCorba_impl *  alpineCorba_impl_s;
    static bool                initialized_s;
    static ReadWriteSem        dataLock_s;

};

