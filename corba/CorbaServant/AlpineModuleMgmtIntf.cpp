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


#include <AlpineModuleMgmtIntf.h>
#include <Log.h>
#include <StringUtils.h>



bool  
AlpineModuleMgmtIntf::registerModule (const string &  libraryPath,
                                      const string &  boostrapSymbol,
                                      ulong &         moduleId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineModuleMgmtIntf::registerModule invoked.");
#endif


    return true;
}



bool  
AlpineModuleMgmtIntf::unregisterModule (ulong  moduleId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineModuleMgmtIntf::unregisterModule invoked.");
#endif


    return true;
}



bool  
AlpineModuleMgmtIntf::setConfiguration (ulong         moduleId,
                                        ConfigData &  configData)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineModuleMgmtIntf::setConfiguration invoked.");
#endif


    return true;
}



bool  
AlpineModuleMgmtIntf::getConfiguration (ulong         moduleId,
                                        ConfigData &  configData)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineModuleMgmtIntf::getConfiguration invoked.");
#endif


    return true;
}



bool  
AlpineModuleMgmtIntf::getModuleInfo (ulong           moduleId,
                                     t_ModuleInfo &  moduleInfo)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineModuleMgmtIntf::getModuleInfo invoked.");
#endif


    return true;
}



bool  
AlpineModuleMgmtIntf::loadModule (ulong  moduleId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineModuleMgmtIntf::loadModule invoked.");
#endif


    return true;
}



bool  
AlpineModuleMgmtIntf::unloadModule (ulong  moduleId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineModuleMgmtIntf::unloadModule invoked.");
#endif


    return true;
}



bool  
AlpineModuleMgmtIntf::listActiveModules (t_IdList &  idList)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineModuleMgmtIntf::listActiveModules invoked.");
#endif


    return true;
}



bool  
AlpineModuleMgmtIntf::listAllModules (t_IdList &  idList)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineModuleMgmtIntf::listAllModules invoked.");
#endif


    return true;
}



