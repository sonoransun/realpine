/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <ApplCore.h>
#include <Configuration.h>
#include <Log.h>
#include <StringUtils.h>
#include <TcpAcceptor.h>
#include <TcpClientConfig.h>
#include <TcpClientConnector.h>
#include <TcpClientThread.h>
#include <TcpConnector.h>
#include <TcpTransport.h>
#include <unistd.h>


void runAsycClientEvents(ulong ipAddress, ushort port, uint numTransports);

void runSyncClientEvents(ulong ipAddress, ushort port, uint numTransports);


int
main(int argc, char * argv[])
{
    // Initialize application core
    //
    bool status;
    status = ApplCore::initialize(argc, argv);

    if (!status) {
        Log::Error("Initialization of Application Core failed!  Exiting...");
        return 1;
    }


    // Initialize configuration
    //
    ConfigData::t_ConfigElementList * configElements;

    TcpClientConfig::getConfigElements(configElements);

    status = Configuration::initialize(argc, argv, *configElements, TcpClientConfig::configFile_s);

    if (!status) {
        Log::Error("Initialization of application configuration failed!  Exiting...");
        return 2;
    }


    // Get configuration settings
    //
    string ipAddressStr;
    string portStr;
    string transportsStr;
    string connMethod;
    ulong ipAddress;
    ushort port;
    uint transports;

    Configuration::getValue("IP Address", ipAddressStr);
    Configuration::getValue("Port", portStr);
    Configuration::getValue("Transports", transportsStr);
    Configuration::getValue("Method", connMethod);

    status = NetUtils::stringIpToLong(ipAddressStr, ipAddress);
    if (!status) {
        Log::Error("Invalid IP address given!  Exiting...");
        return 3;
    }

    port = atoi(portStr.c_str());
    port = htons(port);

    transports = atoi(transportsStr.c_str());


    // Verify that connection method is synchronous or async.
    //
    if ((connMethod != "Async") && (connMethod != "Async")) {

        Log::Error("Invalid Connection Method given in configuartion!"
                   "  Valid values are: Async, Sync");
        return 3;
    }


    Log::Info("Starting TCP client."s + "\n IP: "s + ipAddressStr + "\n Port: "s + portStr + "\n Num Transports: "s +
              transportsStr + "\n Conn Method: "s + connMethod + "\n");


    if (connMethod == "Async") {
        runAsycClientEvents(ipAddress, port, transports);
    } else {
        runSyncClientEvents(ipAddress, port, transports);
    }


    return 0;
}


void
runAsycClientEvents(ulong ipAddress, ushort port, uint numTransports)
{
    TcpClientConnector * connector;
    connector = new TcpClientConnector;

    bool status;
    status = connector->setDestination(ipAddress, port);

    if (!status) {
        Log::Error("Setting connector destination failed in call to runAsyncClientEvents!");
        delete connector;
        exit(4);
    }


    // Renice process before spawning transports to keep system load from exploding
    //
    nice(40);


    uint i;
    ulong currId;

    for (i = 0; i < numTransports; i++) {
        status = connector->requestConnection(currId);

        if (!status) {
            Log::Error("Setting connector destination failed in call to runAsyncClientEvents!");
            delete connector;
            exit(4);
        }

        Log::Info("Requested connection with ID: "s + std::to_string(currId));
    }

    Log::Debug("Requested all transports, entering event loop...");

    // Proces events into infinity...
    //
    while (true) {
        connector->processEvents(true);
        sleep(1);
    }
}


void
runSyncClientEvents(ulong ipAddress, ushort port, uint numTransports)
{
    TcpConnector * connector;
    connector = new TcpConnector;

    bool status;
    status = connector->setDestination(ipAddress, port);

    if (!status) {
        Log::Error("Setting connector destination failed in call to runSyncClientEvents!");
        delete connector;
        exit(4);
    }


    // Renice process before spawning transports to keep system load from exploding
    //
    nice(40);


    uint i;
    TcpTransport * transport;

    for (i = 0; i < numTransports; i++) {
        status = connector->connect(transport);

        if (!status) {
            Log::Error("Connect to destination failed in call to runSyncClientEvents!");
            delete connector;
            exit(4);
        }

        TcpClientThread * clientThread;
        clientThread = new TcpClientThread(transport);
        clientThread->run();

        Log::Info("Started client thread for connected transport.");
    }

    while (1) {
        sleep(3600);
    }  // this should be a thread exit
}
