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
#include <ReadWriteSem.h>
#include <vector>



class CosNamingUtils
{
  public:

    CosNamingUtils ();
    ~CosNamingUtils ();


    using t_ContextList = vector<string>;
    using t_BindingList = vector<string>;

 
 
    static bool  initialize (CORBA::ORB_var &  orb);

    static bool  getRootContext (CosNaming::NamingContext_var &  context);

    bool  getContextList (t_ContextList &  contextList);

    bool  getBindingList (t_BindingList &  bindingList);
 
    bool  setCurrentContext (const string & contextName); 

    bool  getCurrentContext (string & contextName);

    bool  createContext (const string & contextName);

    bool  bindingExists (const string &  name);

    bool  addBinding (const string &             name,
                      const CORBA::Object_var &  object);

    bool  updateBinding (const string &             name,
                         const CORBA::Object_var &  object);

    bool  addContext (const string &                        name,
                      const CosNaming::NamingContext_var &  context);

    bool  resolve (const string &       name,
                   CORBA::Object_var &  object);

    bool  removeBinding (const string & name);



    using t_CosNameList = vector<CosNaming::Name_var>;


  private:
 
    static  bool                          initialized_s;
    static  CORBA::ORB_var                orb_s;
    static  CosNaming::NamingContext_var  rootContext_s;
    static  TAO_Naming_Client *           namingClient_s;  
    static  ReadWriteSem                  dataLock_s;

    CosNaming::NamingContext_var  rootContext_;
    CosNaming::NamingContext_var  currentContext_;
    t_ContextList                 contextPath_;
    string                        contextName_;



    static bool  parseContextPath (const string &   contextPath,
                                   t_ContextList &  contextList);

    static bool  generateContextPath (t_ContextList &  contextList,
                                      string &         contextPath);

    static bool  generateCosName (t_ContextList &        contextList,
                                  CosNaming::Name_var &  cosName);

    static bool  generateCosNameList (t_ContextList &  contextList,
                                      t_CosNameList &  cosNameList);

    static bool  generateContextPath (CosNaming::Name_var &  cosName,
                                      t_ContextList &        contextList);
    

};


