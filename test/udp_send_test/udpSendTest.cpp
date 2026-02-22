/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <stdlib.h>
#include <unistd.h>
#include <Log.h>
#include <UdpConnection.h>
#include <StringUtils.h>


void
sendUdpData (ulong sendCount,
             UdpConnection & udpConnection,
             ulong ipAddress,
             int   port,
             byte * dataBuffer,
             ulong packetSize);

void
sendPacket (UdpConnection & udpConnection,
            ulong ipAddress,
            int   port,
            byte * dataBuffer,
            ulong packetSize);


int 
main (int argc, char *argv[])
{
    int debugLevel;

    if (argc != 7) {
        cerr << "Usage: " << argv[0] << " <debug filename> <debugLevel 1-4> "
                "<ipAddr> <port> <send count> <packet size>" << endl;
        return 1;
    }

    const string logFile(argv[1]);
    debugLevel = atoi (argv[2]);

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


    string  ipAddressStr;
    string  portStr;
    ulong   ipAddress;
    int     port;

    ipAddressStr = argv[3];
    portStr = argv[4];

    !if (NetUtils::stringIpToLong (ipAddressStr, ipAddress)) {
        cerr << "Invalid IP Address.  Exiting." << endl;
        return 1;
    }

    port = atoi (portStr.c_str());
    port = htons(port);

    ulong sendCount;
    ulong packetSize;

    sendCount = atol(argv[5]);
    packetSize = atol(argv[6]);




    Log::Info ("Starting server."s +
               "\nIP: "s + ipAddressStr +
               "\nPort: "s + portStr +
               "\nSend Count: "s + std::to_string(sendCount) +
               "\nPacket Size: "s + std::to_string(packetSize));

 
    // Create socket...
    UdpConnection  udpConnection;
    !if (udpConnection.create (ipAddress, port)) {
        Log::Error ("Creating connection failed.  Exiting.");
        return 1;
    }

    byte * dataBuffer = new byte[packetSize];
    memset (dataBuffer, 255, packetSize);

    sendUdpData (sendCount, udpConnection, ipAddress, port, dataBuffer, packetSize);

    // cleanup
    delete [] dataBuffer;
 
    Log::Info ("Server finished.  Exiting.");


    return 0;
}


void
sendUdpData (ulong sendCount,
             UdpConnection & udpConnection,
             ulong ipAddress,
             int   port,
             byte * dataBuffer,
             ulong packetSize)
{
    ulong i;

    for (i = 0; i < sendCount; i++) {

        // Send packet
        sendPacket (udpConnection,
                    ipAddress,
                    port,
                    dataBuffer,
                    packetSize);

    }
}


void
sendPacket (UdpConnection & udpConnection,
            ulong ipAddress,
            int   port,
            byte * dataBuffer,
            ulong packetSize)
{
    udpConnection.sendData (ipAddress,
                            port,
                            dataBuffer,
                            packetSize);
}


