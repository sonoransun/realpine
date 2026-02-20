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
#include <ConfigData.h>
#include <AlpineQueryModule.h>
#include <AlpineExtensionModule.h>
#include <AlpineTransportClientModule.h>
#include <AlpineTransportServerModule.h>


class AlpineModuleInterface
{
  public:

    AlpineModuleInterface () {};
    virtual ~AlpineModuleInterface () {};


    // General module information
    //
    virtual bool  getModuleInfo (string &  moduleName,
                                 string &  description,
                                 string &  version) = 0;


    // Configuration
    //
    virtual bool  setConfiguration (ConfigData &  configData) = 0;

    virtual bool  getConfiguration (ConfigData &  configData) = 0;


    // Control
    // NOTE: These apply to all components within a module.  Individual components can be started
    // and stopped.  However, stopping a module stops all member components.
    //
    virtual bool  start () = 0;

    virtual bool  isActive () = 0;

    virtual bool  stop () = 0;


    // Supported module interfaces
    //
    virtual bool  getQueryInterface (AlpineQueryModule *&  queryInterface) = 0;

    virtual bool  getExtensionInterface (AlpineExtensionModule *&  extensionInterface) = 0;

    virtual bool  getTransportClientInterface (AlpineTransportClientModule *&  transportClientInterface) = 0;

    virtual bool  getTransportServerInterface (AlpineTransportServerModule *&  transportServerInterface) = 0;


};

