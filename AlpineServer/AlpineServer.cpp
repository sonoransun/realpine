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



#include <stdlib.h>
#include <Platform.h>

#include <Log.h>
#include <StringUtils.h>
#include <NetUtils.h>

#include <ApplCore.h>
#include <Configuration.h>

#ifdef ALPINE_ENABLE_CORBA
#include <CorbaAdmin.h>
#endif

#include <AlpineConfig.h>
#include <AlpineStackConfig.h>
#include <AlpineStack.h>
#include <CovertChannel.h>
#include <WifiDiscovery.h>

#include <ServerSigMethods.h>




int 
main (int argc, char *argv[])
{
    bool status;

    // Initialize application core
    //
    status = ApplCore::initialize (argc, argv);

    if (!status) {
        Log::Error ("Unable to initialize application core.  Exiting.");
        exit (1);
    }
  
 
    // Initialize configuration
    //
    ConfigData::t_ConfigElementList * configElements;

    AlpineConfig::createConfigElements ();
    AlpineConfig::getConfigElements (configElements);

    status = Configuration::initialize (argc, 
                                        argv, 
                                        *configElements,
                                        AlpineConfig::configFile_s);

    if (!status) {
        Log::Error ("Error initializing configuration.  Exiting.");
        return 1;
    }


    // Load configuration settings
    //
    string  ipAddressStr;
    string  portStr;
    ulong   ipAddress;
    int     port;

    status = Configuration::getValue ("IP Address", ipAddressStr);

    if (!status) {
        Log::Error ("No IP Address value.  Exiting.");
        return 1;
    }

    if (!NetUtils::stringIpToLong (ipAddressStr, ipAddress)) {
        Log::Error ("Invalid IP Address.  Exiting.");
        return 1;
    }

    status = Configuration::getValue ("Port", portStr);

    if (!status) {
        Log::Error ("No Port value.  Exiting.");
        return 1;
    }

    port = atoi (portStr.c_str());
    port = htons(port);


    string  interfaceContext;
    status = Configuration::getValue ("Interface Context", interfaceContext);

    if (!status) {
        Log::Error ("No Interface Context value.  Exiting.");
        return 1;
    }


    // Initialize covert channel if configured
    //
    string covertChannelFlag;
    string covertKey;

    if (Configuration::getValue ("Covert Channel", covertChannelFlag)) {
        if (Configuration::getValue ("Covert Key", covertKey)) {
            CovertChannel::initialize (covertKey);
            Log::Info ("Covert channel enabled.");
        } else {
            Log::Error ("Covert channel requested but no key provided.  Ignoring.");
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
    bool   wifiDiscoveryEnabled = false;

    status = Configuration::getValue ("WiFi Discovery Enabled", wifiDiscoveryEnabledStr);
    if (status && (wifiDiscoveryEnabledStr == "true" || wifiDiscoveryEnabledStr == "1"))
        wifiDiscoveryEnabled = true;

    if (wifiDiscoveryEnabled) {
        status = Configuration::getValue ("WiFi Multicast Group", wifiMulticastGroupStr);
        if (status && !wifiMulticastGroupStr.empty())
            WifiDiscovery::setMulticastGroup (wifiMulticastGroupStr);

        status = Configuration::getValue ("WiFi Multicast Port", wifiMulticastPortStr);
        if (status && !wifiMulticastPortStr.empty())
            WifiDiscovery::setMulticastPort (static_cast<ushort>(atoi(wifiMulticastPortStr.c_str())));

        status = Configuration::getValue ("WiFi Announce Interval", wifiAnnounceIntervalStr);
        if (status && !wifiAnnounceIntervalStr.empty())
            WifiDiscovery::setAnnounceInterval (atoi(wifiAnnounceIntervalStr.c_str()));

        status = Configuration::getValue ("WiFi Peer Timeout", wifiPeerTimeoutStr);
        if (status && !wifiPeerTimeoutStr.empty())
            WifiDiscovery::setPeerTimeout (atoi(wifiPeerTimeoutStr.c_str()));

        status = Configuration::getValue ("WiFi Interface", wifiInterfaceStr);
        if (status && !wifiInterfaceStr.empty())
            WifiDiscovery::setWifiInterface (wifiInterfaceStr);

        status = Configuration::getValue ("WiFi Beacon Interval", wifiBeaconIntervalStr);
        if (status && !wifiBeaconIntervalStr.empty())
            WifiDiscovery::setBeaconInterval (atoi(wifiBeaconIntervalStr.c_str()));

        char hostname[256];
        gethostname (hostname, sizeof(hostname));
        string peerId = "server-"s +
            std::to_string(std::hash<string>{}(string(hostname)) & 0xFFFFFF);

        uint wifiCaps = WifiDiscovery::CAP_QUERY | WifiDiscovery::CAP_TRANSFER;

        if (WifiDiscovery::initialize (peerId, ipAddressStr, ntohs(port),
                                       0, wifiCaps)) {
            Log::Info ("WiFi discovery started (peerId: "s + peerId + ").");
        } else {
            Log::Error ("WiFi discovery failed to initialize (continuing without WiFi discovery).");
        }
    }


    Log::Info ("Starting ALPINE server-"s +
               "\nIP: "s + ipAddressStr +
               "\nPort: "s + portStr +
               "\nInterface Context: "s + interfaceContext +
               "\n");


    // Initialize Alpine stack
    //
    AlpineStackConfig  config;
    config.setLocalEndpoint (ipAddress, port);
    config.setMaxConcurrentQueries (10); // MRP_TEMP load from config

    status = AlpineStack::initialize (config);

    if (!status) {
        Log::Error ("Initializing AlpineStack failed.  Exiting.");
        return 1;
    }



    // Initialize tester
    //
    ServerSigMethods::initialize ();


#ifdef ALPINE_ENABLE_CORBA
    // start CORBA interface
    //
    status = CorbaAdmin::initialize (argc, argv);

    if (!status) {
        Log::Error ("Initializing CorbaAdmin failed.  Exiting.");
        return 1;
    }

    status = CorbaAdmin::activateAlpineCorbaServer (interfaceContext);

    if (!status) {
        Log::Error ("Activation for AlpineCorbaServer interface failed.  Exiting.");
        return 1;
    }
#endif



    // Initialization complete, should call pthread_exit here...
    //
    while (true) {
        sleep (3600);
    }


    Log::Info ("Server finished.  Exiting.");


    return 0;
}



