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


#include <AlpineConfig.h>


const string                            AlpineConfig::configFile_s ("test.cfg");
ConfigData::t_ConfigElementList *       AlpineConfig::configElements_s = nullptr;


// Ctor defaulted in header


// Dtor defaulted in header



void  
AlpineConfig::createConfigElements ()
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

    // CORBA interface naming context
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "Interface Context";
    currElement->argOptionName = "interfaceContext";
    currElement->envOptionName = "INTF_CONTEXT";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = true;
    configElements_s->push_back (currElement);

    // Covert channel enable
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "Covert Channel";
    currElement->argOptionName = "covertChannel";
    currElement->envOptionName = "COVERT_CHANNEL";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = false;
    configElements_s->push_back (currElement);

    // Covert channel pre-shared key
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "Covert Key";
    currElement->argOptionName = "covertKey";
    currElement->envOptionName = "COVERT_KEY";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = false;
    configElements_s->push_back (currElement);

    // WiFi Discovery Enabled
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "WiFi Discovery Enabled";
    currElement->argOptionName = "wifiDiscoveryEnabled";
    currElement->envOptionName = "WIFI_DISCOVERY_ENABLED";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = false;
    configElements_s->push_back (currElement);

    // WiFi Multicast Group
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "WiFi Multicast Group";
    currElement->argOptionName = "wifiMulticastGroup";
    currElement->envOptionName = "WIFI_MULTICAST_GROUP";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = false;
    configElements_s->push_back (currElement);

    // WiFi Multicast Port
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "WiFi Multicast Port";
    currElement->argOptionName = "wifiMulticastPort";
    currElement->envOptionName = "WIFI_MULTICAST_PORT";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = false;
    configElements_s->push_back (currElement);

    // WiFi Announce Interval
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "WiFi Announce Interval";
    currElement->argOptionName = "wifiAnnounceInterval";
    currElement->envOptionName = "WIFI_ANNOUNCE_INTERVAL";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = false;
    configElements_s->push_back (currElement);

    // WiFi Peer Timeout
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "WiFi Peer Timeout";
    currElement->argOptionName = "wifiPeerTimeout";
    currElement->envOptionName = "WIFI_PEER_TIMEOUT";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = false;
    configElements_s->push_back (currElement);

    // WiFi Interface
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "WiFi Interface";
    currElement->argOptionName = "wifiInterface";
    currElement->envOptionName = "WIFI_INTERFACE";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = false;
    configElements_s->push_back (currElement);

    // WiFi Beacon Interval
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "WiFi Beacon Interval";
    currElement->argOptionName = "wifiBeaconInterval";
    currElement->envOptionName = "WIFI_BEACON_INTERVAL";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = false;
    configElements_s->push_back (currElement);

    // RPC Port (JSON-RPC over HTTP)
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "RPC Port";
    currElement->argOptionName = "rpcPort";
    currElement->envOptionName = "RPC_PORT";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = false;
    configElements_s->push_back (currElement);

    // RPC Bind Address
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "RPC Bind Address";
    currElement->argOptionName = "rpcBindAddress";
    currElement->envOptionName = "RPC_BIND_ADDRESS";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = false;
    configElements_s->push_back (currElement);
}



void  
AlpineConfig::getConfigElements (ConfigData::t_ConfigElementList *& configElements)
{
    configElements = configElements_s;
}


