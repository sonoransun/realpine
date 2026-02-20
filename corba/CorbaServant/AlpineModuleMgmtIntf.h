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
#include <AlpineCorbaClient.h>
#include <CorbaUtils.h>
#include <ConfigData.h>
#include <string>
#include <vector>


class AlpineModuleMgmtIntf
{
  public:


    // Public types
    //
    struct t_ModuleInfo {
        ulong    moduleId;
        string   moduleName;
        string   description;
        string   version;
        string   libraryPath;
        string   bootstrapSymbol;
        ulong    activeTime;
    };

    using t_IdList = vector<ulong>;



    // Supported interface operations
    //
    static bool  registerModule (const string &  libraryPath,
                                 const string &  boostrapSymbol,
                                 ulong &         moduleId);

    static bool  unregisterModule (ulong  moduleId);

    static bool  setConfiguration (ulong         moduleId,
                                   ConfigData &  configData);

    static bool  getConfiguration (ulong         moduleId,
                                   ConfigData &  configData);

    static bool  getModuleInfo (ulong           moduleId,
                                t_ModuleInfo &  moduleInfo);

    static bool  loadModule (ulong  moduleId);

    static bool  unloadModule (ulong  moduleId);

    static bool  listActiveModules (t_IdList &  idList);

    static bool  listAllModules (t_IdList &  idList);



  private:

};

