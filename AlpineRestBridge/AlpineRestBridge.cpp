/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <Platform.h>
#include <stdlib.h>

#include <Log.h>
#include <NetUtils.h>
#include <SafeParse.h>
#include <StringUtils.h>

#include <ApplCore.h>
#include <Configuration.h>

#include <AlpineStack.h>
#include <AlpineStackConfig.h>
#include <RestBridgeConfig.h>

#include <AdminHandler.h>
#include <ApiDocsHandler.h>
#include <ApiKeyAuth.h>
#include <AuthHandler.h>
#include <BroadcastQueryHandler.h>
#include <ClusterCoordinator.h>
#include <ContentStore.h>
#include <DiscoveryBeacon.h>
#include <DlnaServer.h>
#include <HttpResponse.h>
#include <HttpRouter.h>
#include <HttpServer.h>
#include <InterfaceEnumerator.h>
#include <MdnsService.h>
#include <MetricsHandler.h>
#include <PeerHandler.h>
#include <QueryHandler.h>
#include <SsdpService.h>
#include <StatusHandler.h>
#include <TorService.h>
#include <TorSocksProxy.h>
#include <TorTunnel.h>
#include <UpnpPortMapper.h>
#include <WebhookDispatcher.h>
#include <WifiDiscovery.h>
#include <chrono>
#include <thread>

#ifdef ALPINE_FUSE_ENABLED
#include <AlpineFuse.h>
#include <VfsStatsHandler.h>
#endif

#ifdef ALPINE_TRACING_ENABLED
#include <Tracing.h>
#endif

#ifdef __linux__
#include <sys/prctl.h>
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

    RestBridgeConfig::createConfigElements();
    RestBridgeConfig::getConfigElements(configElements);

    status = Configuration::initialize(argc, argv, *configElements, RestBridgeConfig::configFile_s);

    if (!status) {
        Log::Error("Error initializing configuration.  Exiting.");
        return 1;
    }


    // Apply PR_SET_NO_NEW_PRIVS very early — before any plugins, dynamic
    // libraries, or sub-processes are spawned. Once set, the bit is
    // inherited and cannot be cleared, so set-uid binaries lose their
    // privileges across exec(). Linux-only and opt-in via configuration.
    //
#ifdef __linux__
    {
        string noNewPrivsStr;
        bool noNewPrivs = false;

        if (Configuration::getValue("No New Privs", noNewPrivsStr) &&
            (noNewPrivsStr == "true" || noNewPrivsStr == "1")) {
            noNewPrivs = true;
        }

        if (noNewPrivs) {
            if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) == 0) {
                Log::Info("PR_SET_NO_NEW_PRIVS enabled — privileged exec disabled for this process tree.");
            } else {
                Log::Error("PR_SET_NO_NEW_PRIVS requested but prctl() failed (continuing).");
            }
        } else {
            Log::Info("PR_SET_NO_NEW_PRIVS disabled by configuration.");
        }
    }
#endif


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


    // Load REST server configuration
    //
    string restPortStr;
    string restBindStr;
    ushort restPort = 8080;
    ulong restBindAddress = 0;  // 0 == all interfaces

    status = Configuration::getValue("REST Port", restPortStr);

    if (status)
        restPort = parseUshort(restPortStr).value_or(restPort);

    status = Configuration::getValue("REST Bind Address", restBindStr);

    if (status && !restBindStr.empty())
        NetUtils::stringIpToLong(restBindStr, restBindAddress);


    Log::Info("Starting ALPINE REST Bridge-\nIP: "s + ipAddressStr + "\nPort: " + portStr +
              "\nREST Port: " + restPortStr + "\n");


    // Initialize Alpine stack
    //
    AlpineStackConfig config;
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

    StatusHandler::recordStartTime();

    QueryHandler::registerRoutes(router);
    PeerHandler::registerRoutes(router);
    StatusHandler::registerRoutes(router);
    AuthHandler::registerRoutes(router);
    MetricsHandler::registerRoutes(router);
    AdminHandler::registerRoutes(router);
    ClusterCoordinator::registerRoutes(router);
    ApiDocsHandler::registerRoutes(router);

    // Initialize API key authentication
    string apiKeyConfig;
    if (Configuration::getValue("API Key", apiKeyConfig))
        setenv("ALPINE_API_KEY", apiKeyConfig.c_str(), 0);
    ApiKeyAuth::initialize();
    router.setAuthMiddleware(ApiKeyAuth::validate);
    Log::Info("API key authentication enabled (key: "s + ApiKeyAuth::getKey().substr(0, 8) + "...)");

    // Configure CORS origin
    string corsOrigin;
    if (Configuration::getValue("CORS Origin", corsOrigin))
        HttpResponse::setCorsOrigin(corsOrigin);

    // Initialize webhook dispatcher (non-fatal)
    WebhookDispatcher::initialize();


    // Initialize OpenTelemetry tracing (non-fatal)
    //
#ifdef ALPINE_TRACING_ENABLED
    string tracingEnabledStr;
    string otlpEndpoint;
    bool tracingEnabled = false;

    status = Configuration::getValue("Tracing Enabled", tracingEnabledStr);
    if (status && (tracingEnabledStr == "true" || tracingEnabledStr == "1"))
        tracingEnabled = true;

    if (tracingEnabled) {
        status = Configuration::getValue("OTLP Endpoint", otlpEndpoint);
        if (!status || otlpEndpoint.empty())
            otlpEndpoint = "http://localhost:4318/v1/traces"s;

        if (Tracing::initialize("alpine-rest-bridge"s, otlpEndpoint)) {
            Log::Info("OpenTelemetry tracing initialized (endpoint: "s + otlpEndpoint + ")");
        } else {
            Log::Error("OpenTelemetry tracing failed to initialize (continuing without tracing).");
        }
    } else {
        Log::Info("OpenTelemetry tracing disabled by configuration.");
    }
#endif


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
            if (AlpineFuse::run()) {
                Log::Info("FUSE virtual filesystem mounted at "s + fuseMountPoint);
                VfsStatsHandler::registerRoutes(router);
            } else {
                Log::Error("FUSE virtual filesystem failed to start (continuing without VFS).");
            }
        } else {
            Log::Error("FUSE virtual filesystem failed to initialize (continuing without VFS).");
        }
    } else {
        Log::Info("FUSE virtual filesystem disabled by configuration.");
    }
#endif


    // Start discovery beacon (non-fatal if it fails)
    //
    string beaconPortStr;
    string beaconEnabledStr;
    ushort beaconPort = 8089;
    bool beaconEnabled = true;

    status = Configuration::getValue("Beacon Port", beaconPortStr);
    if (status && !beaconPortStr.empty())
        beaconPort = parseUshort(beaconPortStr).value_or(beaconPort);

    status = Configuration::getValue("Beacon Enabled", beaconEnabledStr);
    if (status && (beaconEnabledStr == "false" || beaconEnabledStr == "0"))
        beaconEnabled = false;

    DiscoveryBeacon * beacon = nullptr;

    if (beaconEnabled) {
        beacon = new DiscoveryBeacon();

        if (beacon->initialize(restPort, beaconPort)) {
            beacon->run();
            Log::Info("Discovery beacon broadcasting on port "s + std::to_string(beaconPort));
        } else {
            Log::Error("Discovery beacon failed to initialize (continuing without discovery).");
            delete beacon;
            beacon = nullptr;
        }
    } else {
        Log::Info("Discovery beacon disabled by configuration.");
    }


    // Initialize cluster coordinator (non-fatal if it fails)
    //
    if (beaconEnabled && beacon) {
        if (ClusterCoordinator::initialize(restPort, beaconPort)) {
            Log::Info("Cluster coordinator started (nodeId: "s + ClusterCoordinator::getLocalNodeInfo().nodeId + ")");
        } else {
            Log::Error("Cluster coordinator failed to initialize (continuing without cluster).");
        }
    } else {
        Log::Info("Cluster coordinator disabled (beacon not active).");
    }


    // Start broadcast query handler (non-fatal if it fails)
    //
    string broadcastPortStr;
    string broadcastEnabledStr;
    ushort broadcastPort = 8090;
    bool broadcastEnabled = true;

    status = Configuration::getValue("Broadcast Port", broadcastPortStr);
    if (status && !broadcastPortStr.empty())
        broadcastPort = parseUshort(broadcastPortStr).value_or(broadcastPort);

    status = Configuration::getValue("Broadcast Handler Enabled", broadcastEnabledStr);
    if (status && (broadcastEnabledStr == "false" || broadcastEnabledStr == "0"))
        broadcastEnabled = false;

    BroadcastQueryHandler * broadcastHandler = nullptr;

    if (broadcastEnabled) {
        broadcastHandler = new BroadcastQueryHandler();

        if (broadcastHandler->initialize(broadcastPort)) {
            broadcastHandler->run();
            Log::Info("Broadcast query handler listening on port "s + std::to_string(broadcastPort));
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
    bool wifiDiscoveryEnabled = true;

    status = Configuration::getValue("WiFi Discovery Enabled", wifiDiscoveryEnabledStr);
    if (status && (wifiDiscoveryEnabledStr == "false" || wifiDiscoveryEnabledStr == "0"))
        wifiDiscoveryEnabled = false;

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

        // Generate peer ID matching BroadcastQueryHandler pattern
        char hostname[256];
        gethostname(hostname, sizeof(hostname));
        string peerId = "bridge-"s + std::to_string(std::hash<string>{}(string(hostname)) & 0xFFFFFF);

        uint wifiCaps = WifiDiscovery::CAP_QUERY | WifiDiscovery::CAP_TRANSFER;

        if (WifiDiscovery::initialize(peerId, ipAddressStr, ntohs(port), restPort, wifiCaps)) {
            Log::Info("WiFi discovery started (peerId: "s + peerId + ").");
        } else {
            Log::Error("WiFi discovery failed to initialize (continuing without WiFi discovery).");
        }
    } else {
        Log::Info("WiFi discovery disabled by configuration.");
    }


    // Initialize UPnP port mapping (non-fatal)
    //
    if (UpnpPortMapper::initialize()) {
        UpnpPortMapper::addMapping(restPort, restPort, "TCP", "Alpine REST API");
        UpnpPortMapper::addMapping(ntohs(port), ntohs(port), "UDP", "Alpine Protocol UDP");
        UpnpPortMapper::addMapping(ntohs(port), ntohs(port), "TCP", "Alpine Protocol TCP");
        if (beaconEnabled)
            UpnpPortMapper::addMapping(beaconPort, beaconPort, "UDP", "Alpine Beacon");
        if (broadcastEnabled)
            UpnpPortMapper::addMapping(broadcastPort, broadcastPort, "UDP", "Alpine Broadcast");

        string externalIp = UpnpPortMapper::getExternalIpAddress();
        if (!externalIp.empty())
            Log::Info("UPnP external IP: "s + externalIp);
    }


    // Start Tor hidden service and tunnel (non-fatal if it fails)
    //
    string torEnabledStr;
    string torControlPortStr;
    string torSocksPortStr;
    string torControlAuthStr;
    string torListenPortStr;
    string torPeersStr;
    bool torEnabled = false;
    ushort torControlPort = 9051;
    ushort torSocksPort = 9050;
    ushort torListenPort = 8091;

    status = Configuration::getValue("Tor Enabled", torEnabledStr);
    if (status && (torEnabledStr == "true" || torEnabledStr == "1"))
        torEnabled = true;

    status = Configuration::getValue("Tor Control Port", torControlPortStr);
    if (status && !torControlPortStr.empty())
        torControlPort = parseUshort(torControlPortStr).value_or(torControlPort);

    status = Configuration::getValue("Tor SOCKS Port", torSocksPortStr);
    if (status && !torSocksPortStr.empty())
        torSocksPort = parseUshort(torSocksPortStr).value_or(torSocksPort);

    Configuration::getValue("Tor Control Auth", torControlAuthStr);

    status = Configuration::getValue("Tor Listen Port", torListenPortStr);
    if (status && !torListenPortStr.empty())
        torListenPort = parseUshort(torListenPortStr).value_or(torListenPort);

    Configuration::getValue("Tor Peers", torPeersStr);

    TorService * torService = nullptr;
    TorTunnel * torTunnel = nullptr;

    if (torEnabled) {
        torService = new TorService();

        if (torService->initialize(torControlPort, torControlAuthStr)) {
            torService->addPortMapping(broadcastPort, torListenPort);
            torService->addPortMapping(restPort, restPort);

            if (torService->createService()) {
                torTunnel = new TorTunnel();

                if (torTunnel->initialize(
                        torListenPort, torSocksPort, torPeersStr, torService->onionAddress(), restPort)) {
                    torTunnel->run();
                    Log::Info("Tor tunnel started, hidden service at "s + torService->onionAddress());
                } else {
                    Log::Error("Tor tunnel failed to initialize (continuing without Tor tunnel).");
                    delete torTunnel;
                    torTunnel = nullptr;
                }
            } else {
                Log::Error("Tor hidden service creation failed (continuing without Tor).");
                delete torService;
                torService = nullptr;
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
    bool dlnaEnabled = true;
    bool transcodeEnabled = true;

    status = Configuration::getValue("DLNA Enabled", dlnaEnabledStr);
    if (status && (dlnaEnabledStr == "false" || dlnaEnabledStr == "0"))
        dlnaEnabled = false;

    status = Configuration::getValue("DLNA Port", dlnaPortStr);
    if (status && !dlnaPortStr.empty())
        dlnaPort = parseUshort(dlnaPortStr).value_or(dlnaPort);

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

    ContentStore * contentStore = nullptr;
    DlnaServer * dlnaServer = nullptr;
    SsdpService * ssdpService = nullptr;
    MdnsService * mdnsService = nullptr;

    if (dlnaEnabled && !mediaDirStr.empty()) {
        contentStore = new ContentStore();

        if (contentStore->initialize(mediaDirStr)) {
            dlnaServer = new DlnaServer(*contentStore);

            if (dlnaServer->initialize(dlnaPort, dlnaHostStr, transcodeEnabled)) {
                dlnaServer->run();

                string locationUrl = dlnaServer->getBaseUrl() + "/device.xml";
                string serverString = "Linux/1.0 UPnP/1.0 Alpine/1.0";

                ssdpService = new SsdpService();
                if (ssdpService->initialize(dlnaServer->getDeviceUuid(), locationUrl, serverString)) {
                    ssdpService->run();
                    Log::Info(string("SSDP service started."));
                } else {
                    Log::Error(string("SSDP service failed to initialize."));
                    delete ssdpService;
                    ssdpService = nullptr;
                }

                mdnsService = new MdnsService();
                if (mdnsService->initialize(string("alpine-host"), dlnaHostStr, dlnaPort)) {
                    mdnsService->run();
                    Log::Info(string("mDNS service started."));
                } else {
                    Log::Error(string("mDNS service failed to initialize."));
                    delete mdnsService;
                    mdnsService = nullptr;
                }

                Log::Info("DLNA server started on port "s + std::to_string(dlnaPort) + " serving " + mediaDirStr);
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
        // Failed to start — cleanup and exit
        Log::Error(string("Failed to start HTTP server.  Exiting."));
        WebhookDispatcher::shutdown();
#ifdef ALPINE_FUSE_ENABLED
        if (AlpineFuse::isRunning())
            AlpineFuse::shutdown();
#endif
        WifiDiscovery::shutdown();
        if (mdnsService) {
            mdnsService->stop();
            delete mdnsService;
        }
        if (ssdpService) {
            ssdpService->stop();
            delete ssdpService;
        }
        if (dlnaServer) {
            dlnaServer->stop();
            delete dlnaServer;
        }
        if (contentStore) {
            delete contentStore;
        }
        if (torTunnel) {
            torTunnel->stop();
            delete torTunnel;
        }
        if (torService) {
            torService->shutdown();
            delete torService;
        }
        UpnpPortMapper::shutdown();
        ClusterCoordinator::shutdown();
        if (beacon) {
            beacon->stop();
            delete beacon;
        }
        if (broadcastHandler) {
            broadcastHandler->stop();
            delete broadcastHandler;
        }
        return 1;
    }

    // Wait for shutdown signal
    //
    Log::Info("REST Bridge running.  Waiting for shutdown signal..."s);

    while (!ApplCore::isShutdownRequested()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }

    Log::Info("Shutdown requested (signal "s + std::to_string(ApplCore::getShutdownSignal()) +
              "), beginning graceful shutdown..."s);


    // Stop accepting new HTTP connections
    server.stop();

    // Configurable drain period for in-flight requests
    int drainSeconds = RestBridgeConfig::getShutdownDrainSeconds();
    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(drainSeconds);
    while (server.getActiveConnections() > 0 && std::chrono::steady_clock::now() < deadline) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    if (server.getActiveConnections() > 0) {
        Log::Info("Drain timeout reached with "s + std::to_string(server.getActiveConnections()) +
                  " connections still active"s);
    }

    // Shutdown webhook dispatcher (drain pending deliveries)
    WebhookDispatcher::shutdown();

    // Cluster coordinator departure heartbeat
    ClusterCoordinator::shutdown();

    // Shutdown Alpine stack (stops event loop, cancels queries, persists ratings, shuts down transports)
    AlpineStack::requestShutdown();
    AlpineStack::cleanUp();


    // Clean up optional services in reverse order
#ifdef ALPINE_FUSE_ENABLED
    if (AlpineFuse::isRunning())
        AlpineFuse::shutdown();
#endif

    WifiDiscovery::shutdown();
    if (torTunnel) {
        torTunnel->stop();
        delete torTunnel;
    }
    if (torService) {
        torService->shutdown();
        delete torService;
    }
    UpnpPortMapper::shutdown();
    if (mdnsService) {
        mdnsService->stop();
        delete mdnsService;
    }
    if (ssdpService) {
        ssdpService->stop();
        delete ssdpService;
    }
    if (dlnaServer) {
        dlnaServer->stop();
        delete dlnaServer;
    }
    if (contentStore) {
        delete contentStore;
    }
    if (beacon) {
        beacon->stop();
        delete beacon;
    }
    if (broadcastHandler) {
        broadcastHandler->stop();
        delete broadcastHandler;
    }

#ifdef ALPINE_TRACING_ENABLED
    Tracing::shutdown();
#endif


    Log::Info(string("REST Bridge finished.  Exiting."));

    return 0;
}
