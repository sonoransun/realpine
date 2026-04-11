/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <ApplCore.h>
#include <Configuration.h>
#include <Log.h>
#include <StringUtils.h>
#include <TcpAcceptor.h>
#include <TcpClientThread.h>
#include <TcpServerConfig.h>
#include <TcpTransport.h>
#include <memory>
#include <unistd.h>


void runServerEvents(ulong ipAddress, ushort port);


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
    Log::Debug("Initializing configuration...");

    ConfigData::t_ConfigElementList * configElements;
    TcpServerConfig::getConfigElements(configElements);

    status = Configuration::initialize(argc, argv, *configElements, TcpServerConfig::configFile_s);

    if (!status) {
        Log::Error("Initialization of application configuration failed!  Exiting...");
        return 2;
    }


    // Get configuration settings
    //
    Log::Debug("Loading configuration settings...");

    string ipAddressStr;
    string portStr;
    ulong ipAddress;
    ushort port;

    Configuration::getValue("IP Address", ipAddressStr);
    Configuration::getValue("Port", portStr);

    Log::Debug("Converting configuration settings...");

    status = NetUtils::stringIpToLong(ipAddressStr, ipAddress);
    if (!status) {
        Log::Error("Invalid IP address given!  Exiting...");
        return 3;
    }

    port = atoi(portStr.c_str());
    port = htons(port);


    Log::Info("Starting TCP server."s + "\n IP: "s + ipAddressStr + "\n Port: "s + portStr + "\n");


    runServerEvents(ipAddress, port);

    return 0;
}


void
runServerEvents(ulong ipAddress, ushort port)
{
    bool status;

    TcpAcceptor * acceptor;
    acceptor = new TcpAcceptor;

    status = acceptor->create(ipAddress, port);

    if (!status) {
        Log::Error("Create acceptor failed in call to runServerEvents!");
        exit(4);
    }


    status = acceptor->blocking();

    if (!status) {
        Log::Error("Going to non blocking mode failed in call to runServerEvents!");
        exit(5);
    }


    // Renice process so we dont kill the system with tcp load
    //
    nice(40);


    // Accept connections until the cows come home...
    //
    std::unique_ptr<TcpTransport> transport;
    TcpClientThread * clientThread;

    while (true) {

        status = acceptor->accept(transport);
        if (!status) {
            continue;
        }

        Log::Info("Accepted TCP transport.  Spawning client thread.");

        clientThread = new TcpClientThread(transport.release());
        clientThread->run();
    }
}
