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
#include <CorbaUtils.h>
#include <ReadWriteSem.h>
#include <tao/PortableServer/ORB_Manager.h>
#include <vector>


class OrbEventThread;


class OrbUtils 
{
  public:

    OrbUtils () = default;
    ~OrbUtils () = default;


    static bool  initialize (int     argc, 
                             char ** argv,
                             ushort  numThreads = 1);

    static bool  getOrb (CORBA::ORB_var &  orbVar);

    static bool  getRootPoa (PortableServer::POA_var &  poaVar);



    using t_OrbThreadList = vector<OrbEventThread *>;

  private:

    static TAO_ORB_Manager           orbManager_s;
    static CORBA::ORB_var            orb_s;
    static PortableServer::POA_var   rootPoa_s;
    static bool                      initialized_s;
    static ushort                    numThreads_s;
    static t_OrbThreadList           orbThreadList_s;
    static ReadWriteSem              dataLock_s;



    static void  runEventLoop  ();

    
    friend class OrbEventThread;

};

