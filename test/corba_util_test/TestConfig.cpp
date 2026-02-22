/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <TestConfig.h>


const string                            TestConfig::configFile_s ("test.cfg");
ConfigData::t_ConfigElementList *       TestConfig::configElements_s = nullptr;


// Ctor defaulted in header


// Dtor defaulted in header



void  
TestConfig::createConfigElements ()
{
    if (configElements_s)
        return;

    configElements_s = new ConfigData::t_ConfigElementList;

    ConfigData::t_ConfigElement *  currElement;


    // add configuration elements for test application
    //

    // Test Context
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "Test Context";
    currElement->argOptionName = "context";
    currElement->envOptionName = "CONTEXT";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = true;
    configElements_s->push_back (currElement);

    // Test Binding
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "Test Binding";
    currElement->argOptionName = "binding";
    currElement->envOptionName = "BINDING";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = true;
    configElements_s->push_back (currElement);
}



void  
TestConfig::getConfigElements (ConfigData::t_ConfigElementList *& configElements)
{
    configElements = configElements_s;
}


