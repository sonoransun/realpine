/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <ServerTestConfig.h>


const string ServerTestConfig::configFile_s("test.cfg");
ConfigData::t_ConfigElementList * ServerTestConfig::configElements_s = nullptr;


// Ctor defaulted in header


// Dtor defaulted in header


void
ServerTestConfig::createConfigElements()
{
    if (configElements_s)
        return;

    configElements_s = new ConfigData::t_ConfigElementList;

    ConfigData::t_ConfigElement * currElement;


    // add configuration elements for test application
    //

    // IP Address
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName = "IP Address";
    currElement->argOptionName = "ipaddress";
    currElement->envOptionName = "IP_ADDRESS";
    currElement->optionType = ConfigData::t_ElementType::String;
    currElement->required = true;
    configElements_s->push_back(currElement);

    // Port
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName = "Port";
    currElement->argOptionName = "port";
    currElement->envOptionName = "PORT";
    currElement->optionType = ConfigData::t_ElementType::String;
    currElement->required = true;
    configElements_s->push_back(currElement);
}


void
ServerTestConfig::getConfigElements(ConfigData::t_ConfigElementList *& configElements)
{
    configElements = configElements_s;
}
