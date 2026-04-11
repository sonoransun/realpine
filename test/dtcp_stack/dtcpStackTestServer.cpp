/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AlpineDtcpConnAcceptor.h>
#include <AlpineDtcpConnTransport.h>
#include <AlpineDtcpUdpTransport.h>
#include <ApplCore.h>
#include <Configuration.h>
#include <DtcpBaseConnMux.h>
#include <DtcpPacket.h>
#include <DtcpStackTest.h>
#include <Log.h>
#include <NetUtils.h>
#include <ServerTestConfig.h>
#include <StringUtils.h>
#include <stdlib.h>
#include <unistd.h>


void sendWelcomeMessage();


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

    ServerTestConfig::createConfigElements();
    ServerTestConfig::getConfigElements(configElements);

    status = Configuration::initialize(argc, argv, *configElements, ServerTestConfig::configFile_s);

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

    port = atoi(portStr.c_str());
    port = htons(port);


    Log::Info("Starting server."s + "\nIP: "s + ipAddressStr + "\nPort: "s + portStr + "\n");


    // Create stack components
    //
    AlpineDtcpUdpTransport * transport;

    transport = new AlpineDtcpUdpTransport(ipAddress, port);

    status = transport->initialize();

    if (!status) {
        Log::Error("initializing Alpine UDP transport failed.  Exiting.");
        return 1;
    }

    status = transport->activate();

    if (!status) {
        Log::Error("activating Alpine UDP transport failed.  Exiting.");
        return 1;
    }


    // Initialize tester
    //
    DtcpStackTest::initialize();


    // Server done, sleeeeeeeeeep...
    //
    while (true) {
        sleep(3600);
    }


    Log::Info("Server finished.  Exiting.");


    return 0;
}
