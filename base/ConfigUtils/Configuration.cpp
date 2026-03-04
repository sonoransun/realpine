/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <Configuration.h>
#include <EnvironMap.h>
#include <ConfigFile.h>
#include <ArgumentMap.h>
#include <WriteLock.h>
#include <ReadLock.h>
#include <Log.h>
#include <StringUtils.h>
#include <format>



std::unique_ptr<ConfigData::t_ConfigElementList>      Configuration::configElementList_s = nullptr;
std::unique_ptr<ConfigData>                           Configuration::configData_s        = nullptr;
std::unique_ptr<EnvironMap>                           Configuration::environMap_s        = nullptr;
std::unique_ptr<ConfigFile>                           Configuration::configFile_s        = nullptr;
std::unique_ptr<ArgumentMap>                          Configuration::argumentMap_s       = nullptr;
bool                                   Configuration::initialized_s       = false;
std::once_flag                         Configuration::initFlag_s;

ReadWriteSem                           Configuration::dataLock_s;
string                                 Configuration::configFilePath_s;



// Ctor defaulted in header


// Dtor defaulted in header



bool
Configuration::initialize (int                                argc,
                           char **                            argv,
                           ConfigData::t_ConfigElementList &  configElements,
                           const string &                     configFilePath)
{
#ifdef _VERBOSE
    Log::Debug ("Configuration::initialize invoked.");
#endif

    bool result = false;

    std::call_once(initFlag_s, [&]() {

        WriteLock  lock(dataLock_s);

        // Allocate configuration utlity objects
        //
        if (!configElementList_s)
            configElementList_s = std::make_unique<ConfigData::t_ConfigElementList>();

        copyConfigElementList (configElements, *configElementList_s);


        if (!configData_s)
            configData_s  = std::make_unique<ConfigData>(configElementList_s.get());

        if (!environMap_s)
            environMap_s  = std::make_unique<EnvironMap>();

        if (!configFile_s)
            configFile_s  = std::make_unique<ConfigFile>();

        if (!argumentMap_s)
            argumentMap_s = std::make_unique<ArgumentMap>();


        bool status;

        status = environMap_s->load ();

        if (!status) {
            Log::Error ("Loading environment variables failed in Configuration::initialize.");
            return false;
        }


        status = configFile_s->initialize (configFilePath);

        if (!status) {
            Log::Error ("Loading config file failed in Configuration::initialize.");
            return false;
        }


        status = argumentMap_s->load (argc, argv);

        if (!status) {
            Log::Error ("Processing command line arguments failed in Configuration::initialize.");
            return false;
        }


        // Attempt to load all configurable values into ConfigData container.
        //
        status = populateValues ();

        if (!status) {
            Log::Error ("populateValues() failed in Configuration::initialize.");
            return false;
        }

        configFilePath_s = configFilePath;
        initialized_s = true;
        result = true;
    });


    return result;
}



bool  
Configuration::save ()
{
#ifdef _VERBOSE
    Log::Debug ("Configuration::save invoked.");
#endif


    return true;
}



bool  
Configuration::getValue (const string & name,
                         string &       value)
{
#ifdef _VERBOSE
    Log::Debug ("Configuration::getValue invoked.");
#endif

    ReadLock  lock(dataLock_s);

    bool status;

    status = configData_s->getValue (name, value);


    return status;
}



bool  
Configuration::getValue (const string &             name,
                         ConfigData::t_ValueList &  valueList)
{
#ifdef _VERBOSE
    Log::Debug ("Configuration::getValue invoked.");
#endif

    ReadLock  lock(dataLock_s);

    bool status; 
      
    status = configData_s->getValue (name, valueList);

    
    return status;
}



bool  
Configuration::setValue (const string & name,
                         string &       value)
{
#ifdef _VERBOSE
    Log::Debug ("Configuration::setValue invoked.");
#endif


    return true;
}



bool  
Configuration::setValue (const string &             name,
                         ConfigData::t_ValueList &  valueList)
{
#ifdef _VERBOSE
    Log::Debug ("Configuration::setValue invoked.");
#endif


    return true;
}



void  
Configuration::copyConfigElementList (ConfigData::t_ConfigElementList &  source,
                                      ConfigData::t_ConfigElementList &  dest)
{
#ifdef _VERBOSE
    Log::Debug ("Configuration::copyConfigElementList invoked.");
#endif

    ConfigData::t_ConfigElement *  currElement;
    ConfigData::t_ConfigElement *  newElement;

    for (const auto& item : source) {

        currElement = item;

        newElement = new ConfigData::t_ConfigElement;
        *newElement = *currElement;

        dest.push_back (newElement);
    }
}



bool
Configuration::populateValues ()
{
#ifdef _VERBOSE
    Log::Debug ("Configuration::populateValues invoked.");
#endif


    Log::Debug ("Loading configuration settings.");

    // Iterate through each configure element and attempt to locate
    // a value.  Use precendence for various sources of config values.
    //
    ConfigData::t_ConfigElement *  currElement;
    string                         currValue;
    ConfigData::t_ValueList        currValueList;
    
    bool status;

    for (const auto& item : *configElementList_s) {

        currElement = item;

        status = locateValue (currElement, currValue);

        if (!status) {
            // Could not locate value.  If this is a required parameter,
            // return error.
            //
            if (currElement->required) {
                Log::Error ("Could not locate required configuration element: "s +
                             currElement->elementName);

                return false;

            }
            continue;
        }


        // If this element is a list value, parse into individual values.
        // Otherwise, just insert into ConfigData container.
        //
        if ( (currElement->optionType == ConfigData::t_ElementType::String) ||
             (currElement->optionType == ConfigData::t_ElementType::Number) )  {

            status = configData_s->setValue (currElement->elementName, currValue);
        }
        else {
            currValueList.clear ();
            parseValueList (currValue, currValueList);

            status = configData_s->setValue (currElement->elementName, currValueList);
        }

        if (!status) {
            Log::Error ("Error assigning value for configuration element: "s +
                        currElement->elementName);

            if (currElement->required) {
                // Fail if this is a required configuration element.
                //
                return false;

            }
            continue;
        }
    }


    return true;
}



bool  
Configuration::locateValue (ConfigData::t_ConfigElement *  element,
                            string &                       value)
{
#ifdef _VERBOSE
    Log::Debug ("Configuration::locateValue invoked.");
#endif            

    // Locate element based on order of precedence.
    // 1st - ConfigFile
    // 2nd - Arguments
    // 3rd - Env Variables
    //

#ifdef _VERBOSE
    Log::Debug ("Locating value for config element name: "s + element->elementName);
#endif

    bool status = false;

    // Check config file first.
    //
    if (!element->fileOptionName.empty())
        status = configFile_s->get (element->fileOptionName, value);

    if (status) {
#ifdef _VERBOSE
        Log::Debug ("Located in config file, value: "s + value);
#endif
        return true;
    }


    if (!status) {
        // Try arguments
        //
        if (!element->argOptionName.empty())
            status = argumentMap_s->get (element->argOptionName, value);

        if (status) {
#ifdef _VERBOSE
            Log::Debug ("Located in config file, value: "s + value);
#endif
            return true;
        }
        return false;
    }


    if (!status) {
        // Try program environment
        //
        if (!element->envOptionName.empty())
            status = environMap_s->get (element->envOptionName, value);

        if (status) {
#ifdef _VERBOSE
            Log::Debug ("Located in environment, value: "s + value);
#endif
            return true;
        }
        return false;
    }


    if (!status) {
        // No value given in any location...
        //
        Log::Error ("No value found for element option name: "s +
                    element->elementName);

        return false;
    }
    return true;
}



void
Configuration::parseValueList (const string &             values,
                               ConfigData::t_ValueList &  valueList)
{
#ifdef _VERBOSE
    Log::Debug ("Configuration::parseValueList invoked.");
#endif
}



bool
Configuration::reload ()
{
#ifdef _VERBOSE
    Log::Debug ("Configuration::reload invoked.");
#endif

    if (!initialized_s || configFilePath_s.empty()) {
        Log::Error("Configuration::reload called before initialize.");
        return false;
    }

    WriteLock lock(dataLock_s);

    bool status = configFile_s->initialize(configFilePath_s);

    if (!status) {
        Log::Error("Configuration::reload — failed to re-read config file.");
        return false;
    }

    status = populateValues();

    if (!status) {
        Log::Error("Configuration::reload — populateValues() failed.");
        return false;
    }

    Log::Info("Configuration reloaded successfully."s);
    return true;
}



bool
Configuration::enableAutoReload (const string & configFilePath)
{
#ifdef _VERBOSE
    Log::Debug ("Configuration::enableAutoReload invoked.");
#endif

    if (!initialized_s) {
        Log::Error("Configuration::enableAutoReload called before initialize.");
        return false;
    }

    {
        WriteLock lock(dataLock_s);
        configFilePath_s = configFilePath;
    }

    Log::Info("Configuration auto-reload enabled for: "s + configFilePath);
    return true;
}
