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


#include <ClientTestConfig.h>


const string                            ClientTestConfig::configFile_s ("test.cfg");
ConfigData::t_ConfigElementList *       ClientTestConfig::configElements_s = nullptr;


// Ctor defaulted in header


// Dtor defaulted in header



void  
ClientTestConfig::createConfigElements ()
{
    if (configElements_s)
        return;

    configElements_s = new ConfigData::t_ConfigElementList;

    ConfigData::t_ConfigElement *  currElement;


    // add configuration elements for test application
    //

    // Server IP Address
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "Server IP Address";
    currElement->argOptionName = "server-ip";
    currElement->envOptionName = "SERVER_IP";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = true;
    configElements_s->push_back (currElement);

    // Server Port
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "Server Port";
    currElement->argOptionName = "server-port";
    currElement->envOptionName = "SERVER_PORT";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = true;
    configElements_s->push_back (currElement);

    // Client IP Address
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "Client IP Address";
    currElement->argOptionName = "client-ip";
    currElement->envOptionName = "CLIENT_IP";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = true;
    configElements_s->push_back (currElement);

    // Client Port
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "Client Port";
    currElement->argOptionName = "client-port";
    currElement->envOptionName = "CLIENT_PORT";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = true;
    configElements_s->push_back (currElement);

    // Client Create Count
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "Client Create Count";
    currElement->argOptionName = "num-creates";
    currElement->envOptionName = "NUM_CREATES";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = true;
    configElements_s->push_back (currElement);
}



void  
ClientTestConfig::getConfigElements (ConfigData::t_ConfigElementList *& configElements)
{
    configElements = configElements_s;
}


