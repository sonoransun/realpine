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


#include <TcpClientConfig.h>



const string                            TcpClientConfig::configFile_s ("client.cfg");
ConfigData::t_ConfigElementList *       TcpClientConfig::configElements_s = nullptr;



void  
TcpClientConfig::createConfigElements ()
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

    // Num TCP transport to create
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "Transports";
    currElement->argOptionName = "transports";
    currElement->envOptionName = "TRANSPORTS";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = true;
    configElements_s->push_back (currElement);

    // Connector style
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "Method";
    currElement->argOptionName = "method";
    currElement->envOptionName = "METHOD";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = true;
    configElements_s->push_back (currElement);
}



void  
TcpClientConfig::getConfigElements (ConfigData::t_ConfigElementList *& configElements)
{
    if (!configElements_s) {
        createConfigElements ();
        return;
    }

    configElements = configElements_s;
}


