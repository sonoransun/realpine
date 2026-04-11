/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AlpineConfig.h>
#include <Configuration.h>
#include <Log.h>
#include <SafeParse.h>


const string AlpineConfig::configFile_s("test.cfg");
ConfigData::t_ConfigElementList * AlpineConfig::configElements_s = nullptr;


// Ctor defaulted in header


// Dtor defaulted in header


void
AlpineConfig::createConfigElements()
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
    currElement->argOptionName = "ipAddress";
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

    // CORBA interface naming context
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName = "Interface Context";
    currElement->argOptionName = "interfaceContext";
    currElement->envOptionName = "INTF_CONTEXT";
    currElement->optionType = ConfigData::t_ElementType::String;
    currElement->required = true;
    configElements_s->push_back(currElement);

    // Covert channel enable
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName = "Covert Channel";
    currElement->argOptionName = "covertChannel";
    currElement->envOptionName = "COVERT_CHANNEL";
    currElement->optionType = ConfigData::t_ElementType::String;
    currElement->required = false;
    configElements_s->push_back(currElement);

    // Covert channel pre-shared key
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName = "Covert Key";
    currElement->argOptionName = "covertKey";
    currElement->envOptionName = "COVERT_KEY";
    currElement->optionType = ConfigData::t_ElementType::String;
    currElement->required = false;
    configElements_s->push_back(currElement);

    // WiFi Discovery Enabled
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName = "WiFi Discovery Enabled";
    currElement->argOptionName = "wifiDiscoveryEnabled";
    currElement->envOptionName = "WIFI_DISCOVERY_ENABLED";
    currElement->optionType = ConfigData::t_ElementType::String;
    currElement->required = false;
    configElements_s->push_back(currElement);

    // WiFi Multicast Group
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName = "WiFi Multicast Group";
    currElement->argOptionName = "wifiMulticastGroup";
    currElement->envOptionName = "WIFI_MULTICAST_GROUP";
    currElement->optionType = ConfigData::t_ElementType::String;
    currElement->required = false;
    configElements_s->push_back(currElement);

    // WiFi Multicast Port
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName = "WiFi Multicast Port";
    currElement->argOptionName = "wifiMulticastPort";
    currElement->envOptionName = "WIFI_MULTICAST_PORT";
    currElement->optionType = ConfigData::t_ElementType::String;
    currElement->required = false;
    configElements_s->push_back(currElement);

    // WiFi Announce Interval
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName = "WiFi Announce Interval";
    currElement->argOptionName = "wifiAnnounceInterval";
    currElement->envOptionName = "WIFI_ANNOUNCE_INTERVAL";
    currElement->optionType = ConfigData::t_ElementType::String;
    currElement->required = false;
    configElements_s->push_back(currElement);

    // WiFi Peer Timeout
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName = "WiFi Peer Timeout";
    currElement->argOptionName = "wifiPeerTimeout";
    currElement->envOptionName = "WIFI_PEER_TIMEOUT";
    currElement->optionType = ConfigData::t_ElementType::String;
    currElement->required = false;
    configElements_s->push_back(currElement);

    // WiFi Interface
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName = "WiFi Interface";
    currElement->argOptionName = "wifiInterface";
    currElement->envOptionName = "WIFI_INTERFACE";
    currElement->optionType = ConfigData::t_ElementType::String;
    currElement->required = false;
    configElements_s->push_back(currElement);

    // WiFi Beacon Interval
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName = "WiFi Beacon Interval";
    currElement->argOptionName = "wifiBeaconInterval";
    currElement->envOptionName = "WIFI_BEACON_INTERVAL";
    currElement->optionType = ConfigData::t_ElementType::String;
    currElement->required = false;
    configElements_s->push_back(currElement);

    // RPC Port (JSON-RPC over HTTP)
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName = "RPC Port";
    currElement->argOptionName = "rpcPort";
    currElement->envOptionName = "RPC_PORT";
    currElement->optionType = ConfigData::t_ElementType::String;
    currElement->required = false;
    configElements_s->push_back(currElement);

    // RPC Bind Address
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName = "RPC Bind Address";
    currElement->argOptionName = "rpcBindAddress";
    currElement->envOptionName = "RPC_BIND_ADDRESS";
    currElement->optionType = ConfigData::t_ElementType::String;
    currElement->required = false;
    configElements_s->push_back(currElement);

    // UPnP Enabled
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName = "UPnP Enabled";
    currElement->argOptionName = "upnpEnabled";
    currElement->envOptionName = "UPNP_ENABLED";
    currElement->optionType = ConfigData::t_ElementType::String;
    currElement->required = false;
    configElements_s->push_back(currElement);

    // UPnP Lease Duration
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName = "UPnP Lease Duration";
    currElement->argOptionName = "upnpLeaseDuration";
    currElement->envOptionName = "UPNP_LEASE_DURATION";
    currElement->optionType = ConfigData::t_ElementType::String;
    currElement->required = false;
    configElements_s->push_back(currElement);

    // VPN Interface
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName = "VPN Interface";
    currElement->argOptionName = "vpnInterface";
    currElement->envOptionName = "VPN_INTERFACE";
    currElement->optionType = ConfigData::t_ElementType::String;
    currElement->required = false;
    configElements_s->push_back(currElement);

    // VPN External Address
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName = "VPN External Address";
    currElement->argOptionName = "vpnExternalAddress";
    currElement->envOptionName = "VPN_EXTERNAL_ADDRESS";
    currElement->optionType = ConfigData::t_ElementType::String;
    currElement->required = false;
    configElements_s->push_back(currElement);

    // VPN Auto Detect
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName = "VPN Auto Detect";
    currElement->argOptionName = "vpnAutoDetect";
    currElement->envOptionName = "VPN_AUTO_DETECT";
    currElement->optionType = ConfigData::t_ElementType::String;
    currElement->required = false;
    configElements_s->push_back(currElement);

    // Tor Enabled
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName = "Tor Enabled";
    currElement->argOptionName = "torEnabled";
    currElement->envOptionName = "TOR_ENABLED";
    currElement->optionType = ConfigData::t_ElementType::String;
    currElement->required = false;
    configElements_s->push_back(currElement);

    // Tor Control Port
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName = "Tor Control Port";
    currElement->argOptionName = "torControlPort";
    currElement->envOptionName = "TOR_CONTROL_PORT";
    currElement->optionType = ConfigData::t_ElementType::String;
    currElement->required = false;
    configElements_s->push_back(currElement);

    // Tor SOCKS Port
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName = "Tor SOCKS Port";
    currElement->argOptionName = "torSocksPort";
    currElement->envOptionName = "TOR_SOCKS_PORT";
    currElement->optionType = ConfigData::t_ElementType::String;
    currElement->required = false;
    configElements_s->push_back(currElement);

    // Tor Control Auth
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName = "Tor Control Auth";
    currElement->argOptionName = "torControlAuth";
    currElement->envOptionName = "TOR_CONTROL_AUTH";
    currElement->optionType = ConfigData::t_ElementType::String;
    currElement->required = false;
    configElements_s->push_back(currElement);

    // FUSE Enabled
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName = "FUSE Enabled";
    currElement->argOptionName = "fuseEnabled";
    currElement->envOptionName = "FUSE_ENABLED";
    currElement->optionType = ConfigData::t_ElementType::String;
    currElement->required = false;
    configElements_s->push_back(currElement);

    // FUSE Mount Point
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName = "FUSE Mount Point";
    currElement->argOptionName = "fuseMountPoint";
    currElement->envOptionName = "FUSE_MOUNT_POINT";
    currElement->optionType = ConfigData::t_ElementType::String;
    currElement->required = false;
    configElements_s->push_back(currElement);

    // FUSE Cache TTL
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName = "FUSE Cache TTL";
    currElement->argOptionName = "fuseCacheTtl";
    currElement->envOptionName = "FUSE_CACHE_TTL";
    currElement->optionType = ConfigData::t_ElementType::String;
    currElement->required = false;
    configElements_s->push_back(currElement);

    // FUSE Feedback Threshold
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName = "FUSE Feedback Threshold";
    currElement->argOptionName = "fuseFeedbackThreshold";
    currElement->envOptionName = "FUSE_FEEDBACK_THRESHOLD";
    currElement->optionType = ConfigData::t_ElementType::String;
    currElement->required = false;
    configElements_s->push_back(currElement);

    // Shutdown Drain Seconds
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName = "Shutdown Drain Seconds";
    currElement->argOptionName = "shutdownDrainSeconds";
    currElement->envOptionName = "SHUTDOWN_DRAIN_SECONDS";
    currElement->optionType = ConfigData::t_ElementType::String;
    currElement->required = false;
    configElements_s->push_back(currElement);
}


void
AlpineConfig::getConfigElements(ConfigData::t_ConfigElementList *& configElements)
{
    configElements = configElements_s;
}


int
AlpineConfig::getIntConfig(const string & name, int defaultValue, int minValue, int maxValue)
{
    string value;
    if (Configuration::getValue(name, value)) {
        auto parsed = parseInt(value);
        if (!parsed) {
            Log::Error("Config '"s + name + "' has invalid value '"s + value + "', using default "s +
                       std::to_string(defaultValue));
            return defaultValue;
        }
        if (*parsed < minValue || *parsed > maxValue) {
            Log::Error("Config '"s + name + "' value "s + std::to_string(*parsed) + " outside range ["s +
                       std::to_string(minValue) + ", "s + std::to_string(maxValue) + "], using default "s +
                       std::to_string(defaultValue));
            return defaultValue;
        }
        return *parsed;
    }
    return defaultValue;
}


int
AlpineConfig::getShutdownDrainSeconds()
{
    return getIntConfig("Shutdown Drain Seconds"s, 5, 1, 60);
}
