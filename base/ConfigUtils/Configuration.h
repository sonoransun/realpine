/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <memory>
#include <mutex>
#include <ConfigData.h>
#include <ReadWriteSem.h>


class EnvironMap;
class ConfigFile;
class ArgumentMap;


class Configuration
{
  public:

    Configuration () = default;
    ~Configuration () = default;



    [[nodiscard]] static bool  initialize (int                                argc,
                             char **                            argv,
                             ConfigData::t_ConfigElementList &  configElements,
                             const string &                     configFilePath);

    [[nodiscard]] static bool  save ();

    [[nodiscard]] static bool  getValue (const string & name,
                           string &       value);

    [[nodiscard]] static bool  getValue (const string &             name,
                           ConfigData::t_ValueList &  valueList);

    [[nodiscard]] static bool  setValue (const string & name,
                           string &       value);

    [[nodiscard]] static bool  setValue (const string &             name,
                           ConfigData::t_ValueList &  valueList);

    static bool  reload ();

    static bool  enableAutoReload (const string & configFilePath);


  private:

    static std::unique_ptr<ConfigData::t_ConfigElementList>  configElementList_s;
    static std::unique_ptr<ConfigData>                       configData_s;
    static std::unique_ptr<EnvironMap>                       environMap_s;
    static std::unique_ptr<ConfigFile>                       configFile_s;
    static std::unique_ptr<ArgumentMap>                      argumentMap_s;
    static bool                               initialized_s;
    static std::once_flag                     initFlag_s;

    static ReadWriteSem                       dataLock_s;
    static string                             configFilePath_s;


    static void  copyConfigElementList (ConfigData::t_ConfigElementList &  source,
                                        ConfigData::t_ConfigElementList &  dest);

    static bool  populateValues ();

    static bool  locateValue (ConfigData::t_ConfigElement *  element,
                              string &                       value);

    static void  parseValueList (const string &             values,
                                 ConfigData::t_ValueList &  valueList);

};

