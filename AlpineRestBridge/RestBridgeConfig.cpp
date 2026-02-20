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


#include <RestBridgeConfig.h>


const string                            RestBridgeConfig::configFile_s ("bridge.cfg");
ConfigData::t_ConfigElementList *       RestBridgeConfig::configElements_s = nullptr;


// Ctor defaulted in header


// Dtor defaulted in header



void
RestBridgeConfig::createConfigElements ()
{
    if (configElements_s)
        return;

    configElements_s = new ConfigData::t_ConfigElementList;

    ConfigData::t_ConfigElement *  currElement;


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

    // REST Port
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "REST Port";
    currElement->argOptionName = "restPort";
    currElement->envOptionName = "REST_PORT";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = true;
    configElements_s->push_back (currElement);

    // REST Bind Address
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "REST Bind Address";
    currElement->argOptionName = "restBindAddress";
    currElement->envOptionName = "REST_BIND_ADDRESS";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = false;
    configElements_s->push_back (currElement);

    // Beacon Port
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "Beacon Port";
    currElement->argOptionName = "beaconPort";
    currElement->envOptionName = "BEACON_PORT";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = false;
    configElements_s->push_back (currElement);

    // Beacon Enabled
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "Beacon Enabled";
    currElement->argOptionName = "beaconEnabled";
    currElement->envOptionName = "BEACON_ENABLED";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = false;
    configElements_s->push_back (currElement);

    // Broadcast Handler Enabled
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "Broadcast Handler Enabled";
    currElement->argOptionName = "broadcastEnabled";
    currElement->envOptionName = "BROADCAST_ENABLED";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = false;
    configElements_s->push_back (currElement);

    // Broadcast Port
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "Broadcast Port";
    currElement->argOptionName = "broadcastPort";
    currElement->envOptionName = "BROADCAST_PORT";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = false;
    configElements_s->push_back (currElement);

    // Tor Enabled
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "Tor Enabled";
    currElement->argOptionName = "torEnabled";
    currElement->envOptionName = "TOR_ENABLED";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = false;
    configElements_s->push_back (currElement);

    // Tor Control Port
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "Tor Control Port";
    currElement->argOptionName = "torControlPort";
    currElement->envOptionName = "TOR_CONTROL_PORT";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = false;
    configElements_s->push_back (currElement);

    // Tor SOCKS Port
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "Tor SOCKS Port";
    currElement->argOptionName = "torSocksPort";
    currElement->envOptionName = "TOR_SOCKS_PORT";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = false;
    configElements_s->push_back (currElement);

    // Tor Control Auth
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "Tor Control Auth";
    currElement->argOptionName = "torControlAuth";
    currElement->envOptionName = "TOR_CONTROL_AUTH";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = false;
    configElements_s->push_back (currElement);

    // Tor Listen Port
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "Tor Listen Port";
    currElement->argOptionName = "torListenPort";
    currElement->envOptionName = "TOR_LISTEN_PORT";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = false;
    configElements_s->push_back (currElement);

    // Tor Peers
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "Tor Peers";
    currElement->argOptionName = "torPeers";
    currElement->envOptionName = "TOR_PEERS";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = false;
    configElements_s->push_back (currElement);

    // DLNA Enabled
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "DLNA Enabled";
    currElement->argOptionName = "dlnaEnabled";
    currElement->envOptionName = "DLNA_ENABLED";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = false;
    configElements_s->push_back (currElement);

    // DLNA Port
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "DLNA Port";
    currElement->argOptionName = "dlnaPort";
    currElement->envOptionName = "DLNA_PORT";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = false;
    configElements_s->push_back (currElement);

    // Media Directory
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "Media Directory";
    currElement->argOptionName = "mediaDirectory";
    currElement->envOptionName = "MEDIA_DIRECTORY";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = false;
    configElements_s->push_back (currElement);

    // DLNA Server Name
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "DLNA Server Name";
    currElement->argOptionName = "dlnaServerName";
    currElement->envOptionName = "DLNA_SERVER_NAME";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = false;
    configElements_s->push_back (currElement);

    // Transcode Enabled
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "Transcode Enabled";
    currElement->argOptionName = "transcodeEnabled";
    currElement->envOptionName = "TRANSCODE_ENABLED";
    currElement->optionType    = ConfigData::t_ElementType::String;
    currElement->required      = false;
    configElements_s->push_back (currElement);

    // DLNA Host Address
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName   = "DLNA Host Address";
    currElement->argOptionName = "dlnaHostAddress";
    currElement->envOptionName = "DLNA_HOST_ADDRESS";
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
}



void
RestBridgeConfig::getConfigElements (ConfigData::t_ConfigElementList *& configElements)
{
    configElements = configElements_s;
}
