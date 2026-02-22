/// Copyright (C) 2026 sonoransun — see LICENCE.txt



#include <stdlib.h>

#include <Log.h>
#include <StringUtils.h>
#include <NetUtils.h>

#include <ApplCore.h>
#include <Configuration.h>

#include <RestBridgeConfig.h>
#include <AlpineStackConfig.h>
#include <AlpineStack.h>

#include <HttpRouter.h>
#include <HttpServer.h>
#include <QueryHandler.h>
#include <PeerHandler.h>
#include <StatusHandler.h>
#include <DiscoveryBeacon.h>
#include <BroadcastQueryHandler.h>
#include <TorHiddenService.h>
#include <TorTunnel.h>
#include <ContentStore.h>
#include <DlnaServer.h>
#include <SsdpService.h>
#include <MdnsService.h>
#include <WifiDiscovery.h>




int
main (int argc, char *argv[])
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

    RestBridgeConfig::createConfigElements();
    RestBridgeConfig::getConfigElements(configElements);

    status = Configuration::initialize(argc,
                                       argv,
                                       *configElements,
                                       RestBridgeConfig::configFile_s);

    if (!status) {
        Log::Error("Error initializing configuration.  Exiting.");
        return 1;
    }


    // Load configuration settings
    //
    string  ipAddressStr;
    string  portStr;
    ulong   ipAddress;
    int     port;

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

    port = atoi(portStr.c_str());
    port = htons(port);


    // Load REST server configuration
    //
    string restPortStr;
    string restBindStr;
    ushort restPort = 8080;
    ulong  restBindAddress = 0;  // 0 == all interfaces

    status = Configuration::getValue("REST Port", restPortStr);

    if (status)
        restPort = (ushort)atoi(restPortStr.c_str());

    status = Configuration::getValue("REST Bind Address", restBindStr);

    if (status && !restBindStr.empty())
        NetUtils::stringIpToLong(restBindStr, restBindAddress);


    Log::Info("Starting ALPINE REST Bridge-\nIP: "s + ipAddressStr +
              "\nPort: " + portStr + "\nREST Port: " + restPortStr + "\n");


    // Initialize Alpine stack
    //
    AlpineStackConfig  config;
    config.setLocalEndpoint(ipAddress, port);
    config.setMaxConcurrentQueries(10);

    status = AlpineStack::initialize(config);

    if (!status) {
        Log::Error("Initializing AlpineStack failed.  Exiting.");
        return 1;
    }


    // Set up REST API routes
    //
    HttpRouter router;

    QueryHandler::registerRoutes(router);
    PeerHandler::registerRoutes(router);
    StatusHandler::registerRoutes(router);


    // Start discovery beacon (non-fatal if it fails)
    //
    string beaconPortStr;
    string beaconEnabledStr;
    ushort beaconPort = 8089;
    bool   beaconEnabled = true;

    status = Configuration::getValue("Beacon Port", beaconPortStr);
    if (status && !beaconPortStr.empty())
        beaconPort = (ushort)atoi(beaconPortStr.c_str());

    status = Configuration::getValue("Beacon Enabled", beaconEnabledStr);
    if (status && (beaconEnabledStr == "false" || beaconEnabledStr == "0"))
        beaconEnabled = false;

    DiscoveryBeacon * beacon = nullptr;

    if (beaconEnabled) {
        beacon = new DiscoveryBeacon();

        if (beacon->initialize(restPort, beaconPort)) {
            beacon->run();
            Log::Info("Discovery beacon broadcasting on port "s +
                      std::to_string(beaconPort));
        } else {
            Log::Error("Discovery beacon failed to initialize (continuing without discovery).");
            delete beacon;
            beacon = nullptr;
        }
    } else {
        Log::Info("Discovery beacon disabled by configuration.");
    }


    // Start broadcast query handler (non-fatal if it fails)
    //
    string broadcastPortStr;
    string broadcastEnabledStr;
    ushort broadcastPort = 8090;
    bool   broadcastEnabled = true;

    status = Configuration::getValue("Broadcast Port", broadcastPortStr);
    if (status && !broadcastPortStr.empty())
        broadcastPort = static_cast<ushort>(atoi(broadcastPortStr.c_str()));

    status = Configuration::getValue("Broadcast Handler Enabled", broadcastEnabledStr);
    if (status && (broadcastEnabledStr == "false" || broadcastEnabledStr == "0"))
        broadcastEnabled = false;

    BroadcastQueryHandler * broadcastHandler = nullptr;

    if (broadcastEnabled) {
        broadcastHandler = new BroadcastQueryHandler();

        if (broadcastHandler->initialize(broadcastPort)) {
            broadcastHandler->run();
            Log::Info("Broadcast query handler listening on port "s +
                      std::to_string(broadcastPort));
        } else {
            Log::Error("Broadcast query handler failed to initialize (continuing without broadcast).");
            delete broadcastHandler;
            broadcastHandler = nullptr;
        }
    } else {
        Log::Info("Broadcast query handler disabled by configuration.");
    }


    // Start WiFi discovery (non-fatal if it fails)
    //
    string wifiDiscoveryEnabledStr;
    string wifiMulticastGroupStr;
    string wifiMulticastPortStr;
    string wifiAnnounceIntervalStr;
    string wifiPeerTimeoutStr;
    string wifiInterfaceStr;
    string wifiBeaconIntervalStr;
    bool   wifiDiscoveryEnabled = true;

    status = Configuration::getValue("WiFi Discovery Enabled", wifiDiscoveryEnabledStr);
    if (status && (wifiDiscoveryEnabledStr == "false" || wifiDiscoveryEnabledStr == "0"))
        wifiDiscoveryEnabled = false;

    if (wifiDiscoveryEnabled) {
        status = Configuration::getValue("WiFi Multicast Group", wifiMulticastGroupStr);
        if (status && !wifiMulticastGroupStr.empty())
            WifiDiscovery::setMulticastGroup(wifiMulticastGroupStr);

        status = Configuration::getValue("WiFi Multicast Port", wifiMulticastPortStr);
        if (status && !wifiMulticastPortStr.empty())
            WifiDiscovery::setMulticastPort(static_cast<ushort>(atoi(wifiMulticastPortStr.c_str())));

        status = Configuration::getValue("WiFi Announce Interval", wifiAnnounceIntervalStr);
        if (status && !wifiAnnounceIntervalStr.empty())
            WifiDiscovery::setAnnounceInterval(atoi(wifiAnnounceIntervalStr.c_str()));

        status = Configuration::getValue("WiFi Peer Timeout", wifiPeerTimeoutStr);
        if (status && !wifiPeerTimeoutStr.empty())
            WifiDiscovery::setPeerTimeout(atoi(wifiPeerTimeoutStr.c_str()));

        status = Configuration::getValue("WiFi Interface", wifiInterfaceStr);
        if (status && !wifiInterfaceStr.empty())
            WifiDiscovery::setWifiInterface(wifiInterfaceStr);

        status = Configuration::getValue("WiFi Beacon Interval", wifiBeaconIntervalStr);
        if (status && !wifiBeaconIntervalStr.empty())
            WifiDiscovery::setBeaconInterval(atoi(wifiBeaconIntervalStr.c_str()));

        // Generate peer ID matching BroadcastQueryHandler pattern
        char hostname[256];
        gethostname(hostname, sizeof(hostname));
        string peerId = "bridge-"s +
            std::to_string(std::hash<string>{}(string(hostname)) & 0xFFFFFF);

        uint wifiCaps = WifiDiscovery::CAP_QUERY | WifiDiscovery::CAP_TRANSFER;

        if (WifiDiscovery::initialize(peerId, ipAddressStr, ntohs(port),
                                      restPort, wifiCaps)) {
            Log::Info("WiFi discovery started (peerId: "s + peerId + ").");
        } else {
            Log::Error("WiFi discovery failed to initialize (continuing without WiFi discovery).");
        }
    } else {
        Log::Info("WiFi discovery disabled by configuration.");
    }


    // Start Tor hidden service and tunnel (non-fatal if it fails)
    //
    string torEnabledStr;
    string torControlPortStr;
    string torSocksPortStr;
    string torControlAuthStr;
    string torListenPortStr;
    string torPeersStr;
    bool   torEnabled = false;
    ushort torControlPort = 9051;
    ushort torSocksPort   = 9050;
    ushort torListenPort  = 8091;

    status = Configuration::getValue("Tor Enabled", torEnabledStr);
    if (status && (torEnabledStr == "true" || torEnabledStr == "1"))
        torEnabled = true;

    status = Configuration::getValue("Tor Control Port", torControlPortStr);
    if (status && !torControlPortStr.empty())
        torControlPort = static_cast<ushort>(atoi(torControlPortStr.c_str()));

    status = Configuration::getValue("Tor SOCKS Port", torSocksPortStr);
    if (status && !torSocksPortStr.empty())
        torSocksPort = static_cast<ushort>(atoi(torSocksPortStr.c_str()));

    Configuration::getValue("Tor Control Auth", torControlAuthStr);

    status = Configuration::getValue("Tor Listen Port", torListenPortStr);
    if (status && !torListenPortStr.empty())
        torListenPort = static_cast<ushort>(atoi(torListenPortStr.c_str()));

    Configuration::getValue("Tor Peers", torPeersStr);

    TorHiddenService * torService = nullptr;
    TorTunnel *        torTunnel  = nullptr;

    if (torEnabled) {
        torService = new TorHiddenService();

        if (torService->initialize(torListenPort, torControlPort,
                                   broadcastPort, torControlAuthStr)) {
            torTunnel = new TorTunnel();

            if (torTunnel->initialize(torListenPort, torSocksPort,
                                      torPeersStr,
                                      torService->onionAddress(), restPort)) {
                torTunnel->run();
                Log::Info("Tor tunnel started, hidden service at "s +
                          torService->onionAddress());
            } else {
                Log::Error("Tor tunnel failed to initialize (continuing without Tor tunnel).");
                delete torTunnel;
                torTunnel = nullptr;
            }
        } else {
            Log::Error("Tor hidden service failed to initialize (continuing without Tor).");
            delete torService;
            torService = nullptr;
        }
    } else {
        Log::Info("Tor hidden service disabled by configuration.");
    }


    // Start DLNA media server (non-fatal if it fails)
    //
    string dlnaEnabledStr;
    string dlnaPortStr;
    string mediaDirStr;
    string dlnaServerNameStr;
    string transcodeEnabledStr;
    string dlnaHostStr;
    ushort dlnaPort = 9090;
    bool   dlnaEnabled = true;
    bool   transcodeEnabled = true;

    status = Configuration::getValue("DLNA Enabled", dlnaEnabledStr);
    if (status && (dlnaEnabledStr == "false" || dlnaEnabledStr == "0"))
        dlnaEnabled = false;

    status = Configuration::getValue("DLNA Port", dlnaPortStr);
    if (status && !dlnaPortStr.empty())
        dlnaPort = (ushort)atoi(dlnaPortStr.c_str());

    Configuration::getValue("Media Directory", mediaDirStr);
    Configuration::getValue("DLNA Server Name", dlnaServerNameStr);

    status = Configuration::getValue("Transcode Enabled", transcodeEnabledStr);
    if (status && (transcodeEnabledStr == "false" || transcodeEnabledStr == "0"))
        transcodeEnabled = false;

    status = Configuration::getValue("DLNA Host Address", dlnaHostStr);
    if (!status || dlnaHostStr.empty())
        dlnaHostStr = ipAddressStr;

    if (dlnaServerNameStr.empty())
        dlnaServerNameStr = "Alpine Media Server";

    ContentStore *  contentStore = nullptr;
    DlnaServer *    dlnaServer   = nullptr;
    SsdpService *   ssdpService  = nullptr;
    MdnsService *   mdnsService  = nullptr;

    if (dlnaEnabled && !mediaDirStr.empty()) {
        contentStore = new ContentStore();

        if (contentStore->initialize(mediaDirStr)) {
            dlnaServer = new DlnaServer(*contentStore);

            if (dlnaServer->initialize(dlnaPort, dlnaHostStr, transcodeEnabled)) {
                dlnaServer->run();

                string locationUrl = dlnaServer->getBaseUrl() + "/device.xml";
                string serverString = "Linux/1.0 UPnP/1.0 Alpine/1.0";

                ssdpService = new SsdpService();
                if (ssdpService->initialize(dlnaServer->getDeviceUuid(),
                                            locationUrl, serverString)) {
                    ssdpService->run();
                    Log::Info(string("SSDP service started."));
                } else {
                    Log::Error(string("SSDP service failed to initialize."));
                    delete ssdpService;
                    ssdpService = nullptr;
                }

                mdnsService = new MdnsService();
                if (mdnsService->initialize(string("alpine-host"),
                                            dlnaHostStr, dlnaPort)) {
                    mdnsService->run();
                    Log::Info(string("mDNS service started."));
                } else {
                    Log::Error(string("mDNS service failed to initialize."));
                    delete mdnsService;
                    mdnsService = nullptr;
                }

                Log::Info("DLNA server started on port "s +
                          std::to_string(dlnaPort) + " serving " + mediaDirStr);
            } else {
                Log::Error(string("DLNA server failed to initialize (continuing without DLNA)."));
                delete dlnaServer;
                dlnaServer = nullptr;
            }
        } else {
            Log::Error(string("Content store failed to initialize (continuing without DLNA)."));
            delete contentStore;
            contentStore = nullptr;
        }
    } else if (!dlnaEnabled) {
        Log::Info(string("DLNA server disabled by configuration."));
    } else {
        Log::Info(string("DLNA server disabled (no media directory configured)."));
    }


    // Start HTTP server (blocking accept loop)
    //
    HttpServer server(router);

    if (!server.start(restBindAddress, restPort)) {
        Log::Error(string("Failed to start HTTP server.  Exiting."));
        WifiDiscovery::shutdown();
        if (mdnsService)  { mdnsService->stop(); delete mdnsService; }
        if (ssdpService)  { ssdpService->stop(); delete ssdpService; }
        if (dlnaServer)   { dlnaServer->stop(); delete dlnaServer; }
        if (contentStore) { delete contentStore; }
        if (torTunnel)  { torTunnel->stop(); delete torTunnel; }
        if (torService) { torService->shutdown(); delete torService; }
        if (beacon) { beacon->stop(); delete beacon; }
        if (broadcastHandler) { broadcastHandler->stop(); delete broadcastHandler; }
        return 1;
    }

    // Clean up in reverse order
    WifiDiscovery::shutdown();
    if (torTunnel)  { torTunnel->stop(); delete torTunnel; }
    if (torService) { torService->shutdown(); delete torService; }
    if (mdnsService)  { mdnsService->stop(); delete mdnsService; }
    if (ssdpService)  { ssdpService->stop(); delete ssdpService; }
    if (dlnaServer)   { dlnaServer->stop(); delete dlnaServer; }
    if (contentStore) { delete contentStore; }
    if (beacon) { beacon->stop(); delete beacon; }
    if (broadcastHandler) { broadcastHandler->stop(); delete broadcastHandler; }


    Log::Info(string("REST Bridge finished.  Exiting."));

    return 0;
}
