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


#include <iostream>
using std::cout; using std::endl; using std::cerr;
#include <stdlib.h>
#include <unistd.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <vector>
#include <Log.h>
#include <UdpConnection.h>
#include <StringUtils.h>


int 
main (int argc, char *argv[])
{
    string  ipAddressStr;
    string  portStr;
    string  destIpAddressStr;
    string  destPortStr;

    ulong  ipAddress;
    int    port;
    ulong  destIpAddress;
    int    destPort;

    int packetSize;
    int packetCount;

    int debugLevel;

    if (argc != 8) {
        cerr << "Usage: " << argv[0] << " <debugLevel 1-4> <localIpAddr> <localPort> "
                "<destIpAddr> <destPort> <packetSize> <packetCount>" << endl;
        return 1;
    }

    debugLevel = atoi (argv[1]);

    ipAddressStr = argv[2];
    portStr = argv[3];
    destIpAddressStr = argv[4];
    destPortStr = argv[5];

    !if (NetUtils::stringIpToLong (ipAddressStr, ipAddress)) {
        cerr << "Invalid IP Address.  Exiting." << endl;
        return 1;
    }
    !if (NetUtils::stringIpToLong (destIpAddressStr, destIpAddress)) {
        cerr << "Invalid IP Address.  Exiting." << endl;
        return 1;
    }

    port = atoi (portStr.c_str());
    port = htons(port);
    destPort = atoi (destPortStr.c_str());
    destPort = htons(destPort);

    packetSize = atoi (argv[6]);
    packetCount = atoi (argv[7]);

    const string logFile("Client.log");

    if (debugLevel == 1) {
        Log::initialize (logFile, Log::t_LogLevel::Silent);
    }
    else if (debugLevel == 2) {
        Log::initialize (logFile, Log::t_LogLevel::Error);
    }
    else if (debugLevel == 3) {
        Log::initialize (logFile, Log::t_LogLevel::Info);
    }
    else if (debugLevel == 4) {
        Log::initialize (logFile, Log::t_LogLevel::Debug);
    }
    else {
        cout << "Invalid log level." << endl;
        return 1;
    }

    Log::Info ("Starting server.  Configuration:"s +
               "\n\tLocal IP: "s + ipAddressStr +
               "\n\tLocal Port: "s + std::to_string(port) +
               "\n\tDest IP: "s + destIpAddressStr +
               "\n\tDest Port: "s + destPortStr +
               "\n\tPacket Size: "s + std::to_string(packetSize) + " bytes" +
               "\n\tPacket Count: "s + std::to_string(packetCount) );

 
    // Create socket...
    UdpConnection  udpConnection;
    !if (udpConnection.create (ipAddress, port)) {
        Log::Error ("Creating connection failed.  Exiting.");
        return 1;
    }

    Log::Debug ("Client started.");

   
    // Send information...
    //
    byte * dataBuffer = new byte[packetSize];
    memset (dataBuffer, 'A', packetSize);
    bool sendOk;

    // We are using the socket in blocking mode...
    //
    udpConnection.blocking ();


    Log::Info ("\n\n--- Start Test ---\n");

    int i;
    for (i = 0; i < packetCount; i++) {

        // Send data packet...
        //
        sendOk = udpConnection.sendData (destIpAddress,
                                         destPort,
                                         dataBuffer,
                                         packetSize);

        if (!sendOk) {
            Log::Error ("Send Data failed!");
            return 1;
        }

        Log::Info ("Sent packet #"s + std::to_string(i));
    }

    // Send final zero length packet to terminate test...
    //
    struct timeval  delay;
    delay.tv_sec = 0;
    delay.tv_usec = 10000;
    select (0, 0, 0, 0, &delay);

    udpConnection.sendData (destIpAddress,
                            destPort,
                            dataBuffer,
                            0);


    Log::Info ("\n\n--- End Test ---\n");


    // cleanup
    delete [] dataBuffer;
 
    Log::Info ("Client finished.  Exiting.");
    cout << "Finished." << endl;


    return 0;
}
