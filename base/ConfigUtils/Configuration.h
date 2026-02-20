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



    static bool  initialize (int                                argc,
                             char **                            argv,
                             ConfigData::t_ConfigElementList &  configElements,
                             const string &                     configFilePath);

    static bool  save ();

    static bool  getValue (const string & name,
                           string &       value);

    static bool  getValue (const string &             name,
                           ConfigData::t_ValueList &  valueList);

    static bool  setValue (const string & name,
                           string &       value);

    static bool  setValue (const string &             name,
                           ConfigData::t_ValueList &  valueList);



  private:

    static std::unique_ptr<ConfigData::t_ConfigElementList>  configElementList_s;
    static std::unique_ptr<ConfigData>                       configData_s;
    static std::unique_ptr<EnvironMap>                       environMap_s;
    static std::unique_ptr<ConfigFile>                       configFile_s;
    static std::unique_ptr<ArgumentMap>                      argumentMap_s;
    static bool                               initialized_s;
    static std::once_flag                     initFlag_s;

    static ReadWriteSem                       dataLock_s;


    static void  copyConfigElementList (ConfigData::t_ConfigElementList &  source,
                                        ConfigData::t_ConfigElementList &  dest);

    static bool  populateValues ();

    static bool  locateValue (ConfigData::t_ConfigElement *  element,
                              string &                       value);

    static void  parseValueList (const string &             values,
                                 ConfigData::t_ValueList &  valueList);

};

