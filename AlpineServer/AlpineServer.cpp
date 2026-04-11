/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <Platform.h>
#include <stdlib.h>

#include <Log.h>
#include <NetUtils.h>
#include <SafeParse.h>
#include <StringUtils.h>

#include <ApplCore.h>
#include <Configuration.h>

#ifdef ALPINE_ENABLE_CORBA
#include <CorbaAdmin.h>
#endif

#include <ApiKeyAuth.h>
#include <HttpRouter.h>
#include <HttpServer.h>
#include <JsonRpcHandler.h>

#include <AlpineConfig.h>
#include <AlpineStack.h>
#include <AlpineStackConfig.h>
#include <CovertChannel.h>
#include <InterfaceEnumerator.h>
#include <TorService.h>
#include <UpnpPortMapper.h>
#include <WifiDiscovery.h>

#include <ServerSigMethods.h>
#include <chrono>
#include <thread>

#ifdef ALPINE_FUSE_ENABLED
#include <AlpineFuse.h>
#endif


int
main(int argc, char * argv[])
{
    bool status;

    // Initialize application core
    //
    status = ApplCore::initialize(argc, argv);

    if (!status) {
        Log::Error("Unable to initialize application core.  Exiting.");
        exit(1);
    }


    // Initialize configuration
    //
    ConfigData::t_ConfigElementList * configElements;

    AlpineConfig::createConfigElements();
    AlpineConfig::getConfigElements(configElements);

    status = Configuration::initialize(argc, argv, *configElements, AlpineConfig::configFile_s);

    if (!status) {
        Log::Error("Error initializing configuration.  Exiting.");
        return 1;
    }


    // Load configuration settings
    //
    string ipAddressStr;
    string portStr;
    ulong ipAddress;
    int port;

    status = Configuration::getValue("IP Address", ipAddressStr);

    if (!status) {
        Log::Error("No IP Address value.  Exiting.");
        return 1;
    }

    if (!NetUtils::stringIpToLong(ipAddressStr, ipAddress)) {
        Log::Error("Invalid IP Address.  Exiting.");
        return 1;
    }

    status = Configuration::getValue("Port", portStr);

    if (!status) {
        Log::Error("No Port value.  Exiting.");
        return 1;
    }

    auto parsedPort = parseInt(portStr);
    if (!parsedPort) {
        Log::Error("Invalid Port value.  Exiting.");
        return 1;
    }
    port = htons(*parsedPort);


    // VPN interface detection and binding (non-fatal)
    //
    string vpnInterfaceStr;
    string vpnExternalAddressStr;
    string vpnAutoDetectStr;

    if (Configuration::getValue("VPN Interface", vpnInterfaceStr) && !vpnInterfaceStr.empty()) {
        string vpnAddr;
        if (InterfaceEnumerator::getInterfaceAddress(vpnInterfaceStr, vpnAddr)) {
            ipAddressStr = vpnAddr;
            NetUtils::stringIpToLong(ipAddressStr, ipAddress);
            Log::Info("VPN interface "s + vpnInterfaceStr + " detected, binding to " + ipAddressStr);
        } else {
            Log::Error("VPN interface "s + vpnInterfaceStr + " not found (continuing with configured IP).");
        }
    } else if (Configuration::getValue("VPN Auto Detect", vpnAutoDetectStr) &&
               (vpnAutoDetectStr == "true" || vpnAutoDetectStr == "1")) {
        std::vector<InterfaceEnumerator::InterfaceInfo> vpnInterfaces;
        if (InterfaceEnumerator::findVpnInterfaces(vpnInterfaces) && !vpnInterfaces.empty()) {
            ipAddressStr = vpnInterfaces[0].ipAddress;
            NetUtils::stringIpToLong(ipAddressStr, ipAddress);
            Log::Info("VPN auto-detected interface "s + vpnInterfaces[0].name + ", binding to " + ipAddressStr);
        } else {
            Log::Info("VPN auto-detect enabled but no VPN interfaces found.");
        }
    }

    if (Configuration::getValue("VPN External Address", vpnExternalAddressStr) && !vpnExternalAddressStr.empty()) {
        Log::Info("VPN external address configured: "s + vpnExternalAddressStr);
    }


    string interfaceContext;
    status = Configuration::getValue("Interface Context", interfaceContext);

    if (!status) {
        Log::Error("No Interface Context value.  Exiting.");
        return 1;
    }


    // Initialize covert channel if configured
    //
    string covertChannelFlag;
    string covertKey;

    if (Configuration::getValue("Covert Channel", covertChannelFlag)) {
        if (Configuration::getValue("Covert Key", covertKey)) {
            CovertChannel::initialize(covertKey);
            Log::Info("Covert channel enabled.");
        } else {
            Log::Error("Covert channel requested but no key provided.  Ignoring.");
        }
    }


    // Initialize WiFi discovery if configured (non-fatal)
    //
    string wifiDiscoveryEnabledStr;
    string wifiMulticastGroupStr;
    string wifiMulticastPortStr;
    string wifiAnnounceIntervalStr;
    string wifiPeerTimeoutStr;
    string wifiInterfaceStr;
    string wifiBeaconIntervalStr;
    bool wifiDiscoveryEnabled = false;

    status = Configuration::getValue("WiFi Discovery Enabled", wifiDiscoveryEnabledStr);
    if (status && (wifiDiscoveryEnabledStr == "true" || wifiDiscoveryEnabledStr == "1"))
        wifiDiscoveryEnabled = true;

    if (wifiDiscoveryEnabled) {
        status = Configuration::getValue("WiFi Multicast Group", wifiMulticastGroupStr);
        if (status && !wifiMulticastGroupStr.empty())
            WifiDiscovery::setMulticastGroup(wifiMulticastGroupStr);

        status = Configuration::getValue("WiFi Multicast Port", wifiMulticastPortStr);
        if (status && !wifiMulticastPortStr.empty())
            WifiDiscovery::setMulticastPort(parseUshort(wifiMulticastPortStr).value_or(0));

        status = Configuration::getValue("WiFi Announce Interval", wifiAnnounceIntervalStr);
        if (status && !wifiAnnounceIntervalStr.empty())
            WifiDiscovery::setAnnounceInterval(parseInt(wifiAnnounceIntervalStr).value_or(0));

        status = Configuration::getValue("WiFi Peer Timeout", wifiPeerTimeoutStr);
        if (status && !wifiPeerTimeoutStr.empty())
            WifiDiscovery::setPeerTimeout(parseInt(wifiPeerTimeoutStr).value_or(0));

        status = Configuration::getValue("WiFi Interface", wifiInterfaceStr);
        if (status && !wifiInterfaceStr.empty())
            WifiDiscovery::setWifiInterface(wifiInterfaceStr);

        status = Configuration::getValue("WiFi Beacon Interval", wifiBeaconIntervalStr);
        if (status && !wifiBeaconIntervalStr.empty())
            WifiDiscovery::setBeaconInterval(parseInt(wifiBeaconIntervalStr).value_or(0));

        char hostname[256];
        gethostname(hostname, sizeof(hostname));
        string peerId = "server-"s + std::to_string(std::hash<string>{}(string(hostname)) & 0xFFFFFF);

        uint wifiCaps = WifiDiscovery::CAP_QUERY | WifiDiscovery::CAP_TRANSFER;

        if (WifiDiscovery::initialize(peerId, ipAddressStr, ntohs(port), 0, wifiCaps)) {
            Log::Info("WiFi discovery started (peerId: "s + peerId + ").");
        } else {
            Log::Error("WiFi discovery failed to initialize (continuing without WiFi discovery).");
        }
    }


    Log::Info("Starting ALPINE server-"s + "\nIP: "s + ipAddressStr + "\nPort: "s + portStr + "\nInterface Context: "s +
              interfaceContext + "\n");


    // Initialize Alpine stack
    //
    AlpineStackConfig config;
    config.setLocalEndpoint(ipAddress, port);
    config.setMaxConcurrentQueries(10);  // MRP_TEMP load from config

    status = AlpineStack::initialize(config);

    if (!status) {
        Log::Error("Initializing AlpineStack failed.  Exiting.");
        return 1;
    }


    // Initialize UPnP port mapping if configured (non-fatal)
    //
    string upnpEnabledStr;
    bool upnpEnabled = false;

    status = Configuration::getValue("UPnP Enabled", upnpEnabledStr);
    if (status && (upnpEnabledStr == "true" || upnpEnabledStr == "1"))
        upnpEnabled = true;

    if (upnpEnabled) {
        if (UpnpPortMapper::initialize()) {
            UpnpPortMapper::addMapping(ntohs(port), ntohs(port), "UDP", "Alpine Protocol UDP");
            UpnpPortMapper::addMapping(ntohs(port), ntohs(port), "TCP", "Alpine Protocol TCP");

            string externalIp = UpnpPortMapper::getExternalIpAddress();
            if (!externalIp.empty())
                Log::Info("UPnP external IP: "s + externalIp);
        } else {
            Log::Error("UPnP initialization failed (continuing without UPnP).");
        }
    } else {
        Log::Info("UPnP port mapping disabled by configuration.");
    }


    // Initialize Tor hidden service if configured (non-fatal)
    //
    string torEnabledStr;
    string torControlPortStr;
    string torControlAuthStr;
    bool torEnabled = false;
    ushort torControlPort = 9051;

    TorService * torService = nullptr;

    status = Configuration::getValue("Tor Enabled", torEnabledStr);
    if (status && (torEnabledStr == "true" || torEnabledStr == "1"))
        torEnabled = true;

    if (torEnabled) {
        status = Configuration::getValue("Tor Control Port", torControlPortStr);
        if (status && !torControlPortStr.empty())
            torControlPort = parseUshort(torControlPortStr).value_or(torControlPort);

        Configuration::getValue("Tor Control Auth", torControlAuthStr);

        torService = new TorService();

        if (torService->initialize(torControlPort, torControlAuthStr)) {
            torService->addPortMapping(ntohs(port), ntohs(port));
            // createService() deferred until RPC port is known
        } else {
            Log::Error("Tor hidden service failed to initialize (continuing without Tor).");
            delete torService;
            torService = nullptr;
        }
    } else {
        Log::Info("Tor hidden service disabled by configuration.");
    }


    // Initialize FUSE virtual filesystem (non-fatal)
    //
#ifdef ALPINE_FUSE_ENABLED
    string fuseEnabledStr;
    string fuseMountPointStr;
    string fuseCacheTtlStr;
    string fuseFeedbackThresholdStr;
    bool fuseEnabled = false;
    string fuseMountPoint = alpine_temp_dir() + "/alpine"s;
    ulong fuseCacheTtl = 60;
    ulong fuseFeedbackThreshold = 5;

    status = Configuration::getValue("FUSE Enabled", fuseEnabledStr);
    if (status && (fuseEnabledStr == "true" || fuseEnabledStr == "1"))
        fuseEnabled = true;

    if (fuseEnabled) {
        if (Configuration::getValue("FUSE Mount Point", fuseMountPointStr) && !fuseMountPointStr.empty())
            fuseMountPoint = fuseMountPointStr;

        if (Configuration::getValue("FUSE Cache TTL", fuseCacheTtlStr) && !fuseCacheTtlStr.empty())
            fuseCacheTtl = parseUlong(fuseCacheTtlStr).value_or(fuseCacheTtl);

        if (Configuration::getValue("FUSE Feedback Threshold", fuseFeedbackThresholdStr) &&
            !fuseFeedbackThresholdStr.empty())
            fuseFeedbackThreshold = parseUlong(fuseFeedbackThresholdStr).value_or(fuseFeedbackThreshold);

        if (AlpineFuse::initialize(fuseMountPoint, fuseCacheTtl, fuseFeedbackThreshold)) {
            if (AlpineFuse::run())
                Log::Info("FUSE virtual filesystem mounted at "s + fuseMountPoint);
            else
                Log::Error("FUSE virtual filesystem failed to start (continuing without VFS).");
        } else {
            Log::Error("FUSE virtual filesystem failed to initialize (continuing without VFS).");
        }
    } else {
        Log::Info("FUSE virtual filesystem disabled by configuration.");
    }
#endif


    // Initialize tester
    //
    ServerSigMethods::initialize();


#ifdef ALPINE_ENABLE_CORBA
    // start CORBA interface
    //
    status = CorbaAdmin::initialize(argc, argv);

    if (!status) {
        Log::Error("Initializing CorbaAdmin failed.  Exiting.");
        return 1;
    }

    status = CorbaAdmin::activateAlpineCorbaServer(interfaceContext);

    if (!status) {
        Log::Error("Activation for AlpineCorbaServer interface failed.  Exiting.");
        return 1;
    }
#endif


    // Load RPC configuration
    //
    string rpcPortStr;
    string rpcBindAddressStr;
    ushort rpcPort = 8600;
    ulong rpcBindAddress = 0;  // all interfaces

    if (Configuration::getValue("RPC Port", rpcPortStr))
        rpcPort = parseUshort(rpcPortStr).value_or(rpcPort);

    if (Configuration::getValue("RPC Bind Address", rpcBindAddressStr)) {
        if (!NetUtils::stringIpToLong(rpcBindAddressStr, rpcBindAddress)) {
            Log::Error("Invalid RPC Bind Address.  Using all interfaces.");
            rpcBindAddress = 0;
        }
    }

    // Add UPnP mapping for RPC port
    if (upnpEnabled && UpnpPortMapper::isAvailable())
        UpnpPortMapper::addMapping(rpcPort, rpcPort, "TCP", "Alpine RPC");

    // Finalize Tor hidden service with both protocol and RPC ports
    if (torService) {
        torService->addPortMapping(rpcPort, rpcPort);
        if (torService->createService()) {
            Log::Info("Tor hidden service started at "s + torService->onionAddress());
        } else {
            Log::Error("Tor hidden service creation failed (continuing without Tor).");
            delete torService;
            torService = nullptr;
        }
    }


    // Start JSON-RPC server (blocking main loop)
    //
    HttpRouter rpcRouter;
    JsonRpcHandler::registerRoutes(rpcRouter);

    // Initialize API key authentication
    string apiKeyConfig;
    if (Configuration::getValue("API Key", apiKeyConfig))
        setenv("ALPINE_API_KEY", apiKeyConfig.c_str(), 0);
    ApiKeyAuth::initialize();
    rpcRouter.setAuthMiddleware(ApiKeyAuth::validate);
    Log::Info("API key authentication enabled (key: "s + ApiKeyAuth::getKey().substr(0, 8) + "...)");

    HttpServer rpcServer(rpcRouter);

    Log::Info("Starting JSON-RPC server on port "s + std::to_string(rpcPort));

    rpcServer.start(rpcBindAddress, rpcPort);


    // Wait for shutdown signal
    //
    Log::Info("Server running.  Waiting for shutdown signal..."s);

    while (!ApplCore::isShutdownRequested()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }

    Log::Info("Shutdown requested (signal "s + std::to_string(ApplCore::getShutdownSignal()) +
              "), beginning graceful shutdown..."s);


    // Stop accepting new HTTP connections
    rpcServer.stop();

    // Configurable drain period for in-flight requests
    int drainSeconds = AlpineConfig::getShutdownDrainSeconds();
    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(drainSeconds);
    while (rpcServer.getActiveConnections() > 0 && std::chrono::steady_clock::now() < deadline) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    if (rpcServer.getActiveConnections() > 0) {
        Log::Info("Drain timeout reached with "s + std::to_string(rpcServer.getActiveConnections()) +
                  " connections still active"s);
    }


    // Shutdown Alpine stack (stops event loop, cancels queries, persists ratings, shuts down transports)
    AlpineStack::requestShutdown();
    AlpineStack::cleanUp();

    // Shutdown optional services in reverse initialization order
#ifdef ALPINE_FUSE_ENABLED
    if (AlpineFuse::isRunning())
        AlpineFuse::shutdown();
#endif

    if (torService) {
        torService->shutdown();
        delete torService;
    }

    UpnpPortMapper::shutdown();
    WifiDiscovery::shutdown();

    Log::Info("Server finished.  Exiting.");


    return 0;
}
