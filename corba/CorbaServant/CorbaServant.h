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
#include <AlpineC.h>
#include <CorbaUtils.h>
#include <OrbUtils.h>
#include <ReadWriteSem.h>


class CorbaServant
{
  public:


    // Initialize the OrbUtils and CosNamingUtils interfaces.
    // Start ORB thread(s)
    //
    static bool  initialize (int      argc,
                             char **  argv);



    // Methods specific to the CORBA interface implementations supported
    // This method initializes the CORBA interface to the server, resolving
    // refernces, etc.
    //
    static bool  intializeAlpineClientInterface (const string &  namingContextPath);



  private:

    static CORBA::ORB_var            orb_s;
    static bool                      initialized_s;
    static bool                      clientInterfaceActive_s;
    static ReadWriteSem              dataLock_s;


    static bool  resolveDtcpPeerMgmtReference (const string &              bindingPath,
                                               Alpine::DtcpPeerMgmt_var &  reference);

    static bool  resolveDtcpStackMgmtReference (const string &               bindingPath,
                                                Alpine::DtcpStackMgmt_var &  reference);

    static bool  resolveDtcpStackStatusReference (const string &                 bindingPath,
                                                  Alpine::DtcpStackStatus_var &  reference);

    static bool  resolveAlpineStackMgmtReference (const string &                 bindingPath,
                                                  Alpine::AlpineStackMgmt_var &  reference);

    static bool  resolveAlpineStackStatusReference (const string &                   bindingPath,
                                                    Alpine::AlpineStackStatus_var &  reference);

    static bool  resolveAlpineGroupMgmtReference (const string &                 bindingPath,
                                                  Alpine::AlpineGroupMgmt_var &  reference);

    static bool  resolveAlpinePeerMgmtReference (const string &                bindingPath,
                                                 Alpine::AlpinePeerMgmt_var &  reference);

    static bool  resolveAlpineQueryReference (const string &             bindingPath,
                                              Alpine::AlpineQuery_var &  reference);

    static bool  resolveAlpineModuleMgmtReference (const string &                  bindingPath,
                                                   Alpine::AlpineModuleMgmt_var &  reference);


};

