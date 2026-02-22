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


  private:

    static ConfigData::t_ConfigElementList *  configElements_s;

};
