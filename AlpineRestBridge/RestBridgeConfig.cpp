/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <Configuration.h>
#include <Log.h>
#include <RestBridgeConfig.h>
#include <SafeParse.h>


const string RestBridgeConfig::configFile_s("bridge.cfg");
ConfigData::t_ConfigElementList * RestBridgeConfig::configElements_s = nullptr;


// Ctor defaulted in header


// Dtor defaulted in header


void
RestBridgeConfig::createConfigElements()
{
    if (configElements_s)
        return;

    configElements_s = new ConfigData::t_ConfigElementList;

    ConfigData::t_ConfigElement * currElement;


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

    // REST Port
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName = "REST Port";
    currElement->argOptionName = "restPort";
    currElement->envOptionName = "REST_PORT";
    currElement->optionType = ConfigData::t_ElementType::String;
    currElement->required = true;
    configElements_s->push_back(currElement);

    // REST Bind Address
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName = "REST Bind Address";
    currElement->argOptionName = "restBindAddress";
    currElement->envOptionName = "REST_BIND_ADDRESS";
    currElement->optionType = ConfigData::t_ElementType::String;
    currElement->required = false;
    configElements_s->push_back(currElement);

    // Beacon Port
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName = "Beacon Port";
    currElement->argOptionName = "beaconPort";
    currElement->envOptionName = "BEACON_PORT";
    currElement->optionType = ConfigData::t_ElementType::String;
    currElement->required = false;
    configElements_s->push_back(currElement);

    // Beacon Enabled
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName = "Beacon Enabled";
    currElement->argOptionName = "beaconEnabled";
    currElement->envOptionName = "BEACON_ENABLED";
    currElement->optionType = ConfigData::t_ElementType::String;
    currElement->required = false;
    configElements_s->push_back(currElement);

    // Broadcast Handler Enabled
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName = "Broadcast Handler Enabled";
    currElement->argOptionName = "broadcastEnabled";
    currElement->envOptionName = "BROADCAST_ENABLED";
    currElement->optionType = ConfigData::t_ElementType::String;
    currElement->required = false;
    configElements_s->push_back(currElement);

    // Broadcast Port
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName = "Broadcast Port";
    currElement->argOptionName = "broadcastPort";
    currElement->envOptionName = "BROADCAST_PORT";
    currElement->optionType = ConfigData::t_ElementType::String;
    currElement->required = false;
    configElements_s->push_back(currElement);

    // No New Privs
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName = "No New Privs";
    currElement->argOptionName = "noNewPrivs";
    currElement->envOptionName = "NO_NEW_PRIVS";
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

    // Tor Listen Port
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName = "Tor Listen Port";
    currElement->argOptionName = "torListenPort";
    currElement->envOptionName = "TOR_LISTEN_PORT";
    currElement->optionType = ConfigData::t_ElementType::String;
    currElement->required = false;
    configElements_s->push_back(currElement);

    // Tor Peers
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName = "Tor Peers";
    currElement->argOptionName = "torPeers";
    currElement->envOptionName = "TOR_PEERS";
    currElement->optionType = ConfigData::t_ElementType::String;
    currElement->required = false;
    configElements_s->push_back(currElement);

    // DLNA Enabled
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName = "DLNA Enabled";
    currElement->argOptionName = "dlnaEnabled";
    currElement->envOptionName = "DLNA_ENABLED";
    currElement->optionType = ConfigData::t_ElementType::String;
    currElement->required = false;
    configElements_s->push_back(currElement);

    // DLNA Port
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName = "DLNA Port";
    currElement->argOptionName = "dlnaPort";
    currElement->envOptionName = "DLNA_PORT";
    currElement->optionType = ConfigData::t_ElementType::String;
    currElement->required = false;
    configElements_s->push_back(currElement);

    // Media Directory
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName = "Media Directory";
    currElement->argOptionName = "mediaDirectory";
    currElement->envOptionName = "MEDIA_DIRECTORY";
    currElement->optionType = ConfigData::t_ElementType::String;
    currElement->required = false;
    configElements_s->push_back(currElement);

    // DLNA Server Name
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName = "DLNA Server Name";
    currElement->argOptionName = "dlnaServerName";
    currElement->envOptionName = "DLNA_SERVER_NAME";
    currElement->optionType = ConfigData::t_ElementType::String;
    currElement->required = false;
    configElements_s->push_back(currElement);

    // Transcode Enabled
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName = "Transcode Enabled";
    currElement->argOptionName = "transcodeEnabled";
    currElement->envOptionName = "TRANSCODE_ENABLED";
    currElement->optionType = ConfigData::t_ElementType::String;
    currElement->required = false;
    configElements_s->push_back(currElement);

    // DLNA Host Address
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName = "DLNA Host Address";
    currElement->argOptionName = "dlnaHostAddress";
    currElement->envOptionName = "DLNA_HOST_ADDRESS";
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

    // API Key
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName = "API Key";
    currElement->argOptionName = "apiKey";
    currElement->envOptionName = "ALPINE_API_KEY";
    currElement->optionType = ConfigData::t_ElementType::String;
    currElement->required = false;
    configElements_s->push_back(currElement);

    // CORS Origin
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName = "CORS Origin";
    currElement->argOptionName = "corsOrigin";
    currElement->envOptionName = "CORS_ORIGIN";
    currElement->optionType = ConfigData::t_ElementType::String;
    currElement->required = false;
    configElements_s->push_back(currElement);

    // Module Registration Enabled
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName = "Module Registration Enabled";
    currElement->argOptionName = "moduleRegistration";
    currElement->envOptionName = "MODULE_REGISTRATION";
    currElement->optionType = ConfigData::t_ElementType::String;
    currElement->required = false;
    configElements_s->push_back(currElement);

    // Module Directory
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName = "Module Directory";
    currElement->argOptionName = "moduleDirectory";
    currElement->envOptionName = "MODULE_DIRECTORY";
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

    // HTTP Min Threads
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName = "HTTP Min Threads";
    currElement->argOptionName = "httpMinThreads";
    currElement->envOptionName = "HTTP_MIN_THREADS";
    currElement->optionType = ConfigData::t_ElementType::String;
    currElement->required = false;
    configElements_s->push_back(currElement);

    // HTTP Max Threads
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName = "HTTP Max Threads";
    currElement->argOptionName = "httpMaxThreads";
    currElement->envOptionName = "HTTP_MAX_THREADS";
    currElement->optionType = ConfigData::t_ElementType::String;
    currElement->required = false;
    configElements_s->push_back(currElement);

    // HTTP Max Connections
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName = "HTTP Max Connections";
    currElement->argOptionName = "httpMaxConnections";
    currElement->envOptionName = "HTTP_MAX_CONNECTIONS";
    currElement->optionType = ConfigData::t_ElementType::String;
    currElement->required = false;
    configElements_s->push_back(currElement);

    // HTTP Max Connections Per IP
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName = "HTTP Max Connections Per IP";
    currElement->argOptionName = "httpMaxConnectionsPerIp";
    currElement->envOptionName = "HTTP_MAX_CONNECTIONS_PER_IP";
    currElement->optionType = ConfigData::t_ElementType::String;
    currElement->required = false;
    configElements_s->push_back(currElement);

    // HTTP Idle Timeout Seconds
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName = "HTTP Idle Timeout Seconds";
    currElement->argOptionName = "httpIdleTimeoutSeconds";
    currElement->envOptionName = "HTTP_IDLE_TIMEOUT_SECONDS";
    currElement->optionType = ConfigData::t_ElementType::String;
    currElement->required = false;
    configElements_s->push_back(currElement);

    // HTTP Keep-Alive Max Requests
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName = "HTTP Keep-Alive Max Requests";
    currElement->argOptionName = "httpKeepAliveMaxRequests";
    currElement->envOptionName = "HTTP_KEEPALIVE_MAX_REQUESTS";
    currElement->optionType = ConfigData::t_ElementType::String;
    currElement->required = false;
    configElements_s->push_back(currElement);

    // HTTP Write Timeout Seconds
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName = "HTTP Write Timeout Seconds";
    currElement->argOptionName = "httpWriteTimeoutSeconds";
    currElement->envOptionName = "HTTP_WRITE_TIMEOUT_SECONDS";
    currElement->optionType = ConfigData::t_ElementType::String;
    currElement->required = false;
    configElements_s->push_back(currElement);

    // Tracing Enabled
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName = "Tracing Enabled";
    currElement->argOptionName = "tracingEnabled";
    currElement->envOptionName = "TRACING_ENABLED";
    currElement->optionType = ConfigData::t_ElementType::String;
    currElement->required = false;
    configElements_s->push_back(currElement);

    // OTLP Endpoint
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName = "OTLP Endpoint";
    currElement->argOptionName = "otlpEndpoint";
    currElement->envOptionName = "OTLP_ENDPOINT";
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

    // Webhook Secret
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName = "Webhook Secret";
    currElement->argOptionName = "webhookSecret";
    currElement->envOptionName = "WEBHOOK_SECRET";
    currElement->optionType = ConfigData::t_ElementType::String;
    currElement->required = false;
    configElements_s->push_back(currElement);

    // Webhook Max Retries
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName = "Webhook Max Retries";
    currElement->argOptionName = "webhookMaxRetries";
    currElement->envOptionName = "WEBHOOK_MAX_RETRIES";
    currElement->optionType = ConfigData::t_ElementType::String;
    currElement->required = false;
    configElements_s->push_back(currElement);

    // Webhook Timeout Seconds
    currElement = new ConfigData::t_ConfigElement;
    currElement->elementName = "Webhook Timeout Seconds";
    currElement->argOptionName = "webhookTimeoutSeconds";
    currElement->envOptionName = "WEBHOOK_TIMEOUT_SECONDS";
    currElement->optionType = ConfigData::t_ElementType::String;
    currElement->required = false;
    configElements_s->push_back(currElement);
}


void
RestBridgeConfig::getConfigElements(ConfigData::t_ConfigElementList *& configElements)
{
    configElements = configElements_s;
}


int
RestBridgeConfig::getIntConfig(const string & name, int defaultValue, int minValue, int maxValue)
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
RestBridgeConfig::getHttpMinThreads()
{
    return getIntConfig("HTTP Min Threads"s, 4, 1, 256);
}
int
RestBridgeConfig::getHttpMaxThreads()
{
    return getIntConfig("HTTP Max Threads"s, 32, 1, 1024);
}
int
RestBridgeConfig::getHttpMaxConnections()
{
    return getIntConfig("HTTP Max Connections"s, 512, 1, 65535);
}
int
RestBridgeConfig::getHttpMaxConnectionsPerIp()
{
    return getIntConfig("HTTP Max Connections Per IP"s, 16, 1, 1024);
}
int
RestBridgeConfig::getHttpIdleTimeoutSeconds()
{
    return getIntConfig("HTTP Idle Timeout Seconds"s, 60, 1, 3600);
}
int
RestBridgeConfig::getHttpKeepAliveMaxRequests()
{
    return getIntConfig("HTTP Keep-Alive Max Requests"s, 100, 1, 10000);
}
int
RestBridgeConfig::getHttpWriteTimeoutSeconds()
{
    return getIntConfig("HTTP Write Timeout Seconds"s, 10, 1, 300);
}
int
RestBridgeConfig::getShutdownDrainSeconds()
{
    return getIntConfig("Shutdown Drain Seconds"s, 5, 1, 60);
}

string
RestBridgeConfig::getWebhookSecret()
{
    string value;
    Configuration::getValue("Webhook Secret"s, value);
    return value;
}

int
RestBridgeConfig::getWebhookMaxRetries()
{
    return getIntConfig("Webhook Max Retries"s, 3, 0, 20);
}
int
RestBridgeConfig::getWebhookTimeoutSeconds()
{
    return getIntConfig("Webhook Timeout Seconds"s, 10, 1, 120);
}
