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
#include <OrbUtils.h>
#include <ReadWriteSem.h>


class CorbaAdmin
{
  public:

    CorbaAdmin () = default;
    ~CorbaAdmin () = default;


    // Initialize the OrbUtils and CosNamingUtils interfaces.  
    // Start ORB thread(s)
    //
    static bool  initialize (int      argc,
                             char **  argv);
                             


    // Methods specific to the CORBA interface implementations supported
    // activate creates the interface and registers its reference in the
    // naming service server for use by external entities.
    //
    static bool  activateAlpineCorbaServer (const string &  namingContextPath);
                             


  private:

    static CORBA::ORB_var            orb_s;
    static PortableServer::POA_var   rootPoa_s;
    static bool                      initialized_s;
    static bool                      alpineCorbaServerActive_s;
    static ReadWriteSem              dataLock_s;


    static bool  bindDtcpPeerMgmtReference (const string & bindingPath);

    static bool  bindDtcpStackMgmtReference (const string & bindingPath);

    static bool  bindDtcpStackStatusReference (const string & bindingPath);

    static bool  bindAlpineStackMgmtReference (const string & bindingPath);

    static bool  bindAlpineStackStatusReference (const string & bindingPath);

    static bool  bindAlpineGroupMgmtReference (const string & bindingPath);

    static bool  bindAlpinePeerMgmtReference (const string & bindingPath);

    static bool  bindAlpineQueryReference (const string & bindingPath);

    static bool  bindAlpineModuleMgmtReference (const string & bindingPath);

};

