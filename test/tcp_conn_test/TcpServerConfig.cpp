/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <TcpServerConfig.h>



const string                            TcpServerConfig::configFile_s ("server.cfg");
ConfigData::t_ConfigElementList *       TcpServerConfig::configElements_s = nullptr;



void  
TcpServerConfig::createConfigElements ()
{
    if (configElements_s)
        return;

    configElements_s = new ConfigData::t_ConfigElementList;

    ConfigData::t_ConfigElement *  currElement;


    // add configuration elements for test application
    //

    // IP Address
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "IP Address";
    currElement->argOptionName = "ipAddress";
    currElement->envOptionName = "IP_ADDRESS";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = true;
    configElements_s->push_back (currElement);

    // Port
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "Port";
    currElement->argOptionName = "port";
    currElement->envOptionName = "PORT";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = true;
    configElements_s->push_back (currElement);
}



void  
TcpServerConfig::getConfigElements (ConfigData::t_ConfigElementList *& configElements)
{
    if (!configElements_s) {
        createConfigElements ();
        return;
    }

    configElements = configElements_s;
}


