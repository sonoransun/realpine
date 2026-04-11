/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <ConfigData.h>


class AlpineConfig
{
  public:
    AlpineConfig() = default;
    ~AlpineConfig() = default;


    static const string configFile_s;

    static void createConfigElements();

    static void getConfigElements(ConfigData::t_ConfigElementList *& configElements);

    // Shutdown drain
    static int getShutdownDrainSeconds();


  private:
    static ConfigData::t_ConfigElementList * configElements_s;

    static int getIntConfig(const string & name, int defaultValue, int minValue, int maxValue);
};
