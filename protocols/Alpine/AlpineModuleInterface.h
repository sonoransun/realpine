/// Copyright (C) 2026 sonoransun — see LICENCE.txt


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

