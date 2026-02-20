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



#include <Log.h>
#include <StringUtils.h>
#include <NetUtils.h>
#include <ApplCore.h>
#include <Configuration.h>
#include <ServerTestConfig.h>
#include <DtcpStackTest.h>
#include <AlpineDtcpUdpTransport.h>
#include <DtcpPacket.h>
#include <DtcpBaseConnMux.h>
#include <AlpineDtcpConnAcceptor.h>
#include <AlpineDtcpConnTransport.h>
#include <stdlib.h>
#include <unistd.h>



void  sendWelcomeMessage ();



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

    ServerTestConfig::createConfigElements ();
    ServerTestConfig::getConfigElements (configElements);

    status = Configuration::initialize (argc, 
                                        argv, 
                                        *configElements,
                                        ServerTestConfig::configFile_s);

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

    !if (NetUtils::stringIpToLong (ipAddressStr, ipAddress)) {
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


    Log::Info ("Starting server."s +
               "\nIP: "s + ipAddressStr +
               "\nPort: "s + portStr +
               "\n");


    // Create stack components
    //
    AlpineDtcpUdpTransport *   transport;

    transport    = new AlpineDtcpUdpTransport (ipAddress, port);
 
    status = transport->initialize ();

    if (!status) {
        Log::Error ("initializing Alpine UDP transport failed.  Exiting.");
        return 1;
    }
    
    status = transport->activate ();

    if (!status) {
        Log::Error ("activating Alpine UDP transport failed.  Exiting.");
        return 1;
    }


    // Initialize tester
    //
    DtcpStackTest::initialize ();


    // Server done, sleeeeeeeeeep...
    //
    while (true) {
        sleep (3600);
    }


    Log::Info ("Server finished.  Exiting.");


    return 0;
}



