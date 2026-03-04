/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <ConfigData.h>


class RestBridgeConfig
{
  public:

    RestBridgeConfig () = default;
    ~RestBridgeConfig () = default;


    static const string  configFile_s;

    static void  createConfigElements ();

    static void  getConfigElements (ConfigData::t_ConfigElementList *& configElements);

    // HTTP server pool/connection config helpers
    static int   getHttpMinThreads ();
    static int   getHttpMaxThreads ();
    static int   getHttpMaxConnections ();
    static int   getHttpMaxConnectionsPerIp ();
    static int   getHttpIdleTimeoutSeconds ();


  private:

    static ConfigData::t_ConfigElementList *  configElements_s;

    static int  getIntConfig (const string & name, int defaultValue);

};
