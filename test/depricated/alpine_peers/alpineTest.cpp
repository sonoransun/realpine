/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <iostream>
using std::cout; using std::endl; using std::cerr;
#include <stdlib.h>
#include <unistd.h>
#include <sys/poll.h>
#include <vector>
#include <Log.h>
#include <UdpConnection.h>
#include <AlpineMessage.h>
#include <AlpineReactor.h>
#include <AlpineConnectionMap.h>
#include <StringUtils.h>


void sendConnectRequest (UdpConnection & udpConnection,
                         ulong           peerIpAddress,
                         int             peerPort);


int 
main (int argc, char *argv[])
{
    string  ipAddressStr;
    string  portStr;
    string  peerIpAddressStr;
    string  peerPortStr;

    ulong  ipAddress;
    int    port;
    ulong  peerIpAddress;
    int    peerPort;

    string logFilename;
    int debugLevel;

    if (argc != 7) {
        cerr << "Usage: " << argv[0] << " <debug filename> <debugLevel 1-4> <myIpAddr> "
                                        "<myPort> <destIpAddr> <destPort>" << endl;
        return 1;
    }
    logFilename = argv[1];
    debugLevel = atoi (argv[2]);

    // Local location
    //
    ipAddressStr = argv[3];
    portStr = argv[4];

    if (!NetUtils::stringIpToLong (ipAddressStr, ipAddress)) {
        cerr << "Invalid IP Address.  Exiting." << endl;
        return 1;
    }
    port = atoi (portStr.c_str ());
    port = htons(port);


    // Peer location
    //
    peerIpAddressStr = argv[5];
    peerPortStr = argv[6];

    if (!NetUtils::stringIpToLong (peerIpAddressStr, peerIpAddress)) {
        cerr << "Invalid IP Address.  Exiting." << endl;
        return 1;
    }
    peerPort = atoi (peerPortStr.c_str());
    peerPort = htons(peerPort);



    if (debugLevel == 1) {
        Log::initialize (logFilename, Log::t_LogLevel::Silent);
    }
    else if (debugLevel == 2) {
        Log::initialize (logFilename, Log::t_LogLevel::Error);
    }
    else if (debugLevel == 3) {
        Log::initialize (logFilename, Log::t_LogLevel::Info);
    }
    else if (debugLevel == 4) {
        Log::initialize (logFilename, Log::t_LogLevel::Debug);
    }
    else {
        cout << "Invalid log level." << endl;
        return 1;
    }
    Log::Info ("Starting server."s +
               "\nMy IP: "s + ipAddressStr +
               "\nMy Port: "s + portStr +
               "\nPeer IP: "s + peerIpAddressStr +
               "\nPeer Port: "s + peerPortStr );

 
    // Create socket...
    UdpConnection  udpConnection;
    if (!udpConnection.create (ipAddress, port)) {
        Log::Error ("Creating connection failed.  Exiting.");
        return 1;
    }
    int fd;

    fd = udpConnection.getFd ();

    Log::Debug ("Connection created.  Active FD: "s + std::to_string(fd));

   
    struct pollfd  pollData;
    pollData.fd = fd;
    pollData.events = POLLIN;

    bool done = false;
    int retVal;
    const int timeout = 6000;

    
    // Send connection request
    //
    sendConnectRequest (udpConnection, peerIpAddress, peerPort);

    // Process incoming packets.
    //
    bool receiveOk;
    const int maxDataSize = 65536; // 64k
    byte * dataBuffer = new byte[maxDataSize];
    ulong sourceIpAddress;
    ushort sourcePort;
    uint receivedLength;

    ulong totalReceived = 0;
    ulong totalData = 0;

    AlpineMessage * msg;
    msg = new AlpineMessage;

    // Reactor
    AlpineReactor  reactor;


    while (!done) {

        // Check reason for poll return...
        //
        retVal = poll (&pollData, 1, timeout);

        if (retVal == 0) {
            Log::Debug ("Poll timeout...");
            continue;

        if (retVal < 0) {
            Log::Error ("Poll failed!");
            done = true;
            continue;

        Log::Debug ("Received active FD, checking socket.");

        // Receive data...
        receiveOk = udpConnection.receiveData (dataBuffer,
                                               maxDataSize,
					       sourceIpAddress,
                                               sourcePort,
                                               receivedLength);

        if (!receiveOk) {
            Log::Error ("Receive Data failed!");
            continue;

        // Catch backdoor exit... ;)
        if (receivedLength == 0) {
            done = true;
            continue;

#ifdef _VERBOSE
        string logMsg;
        NetUtils::longIpToString (sourceIpAddress, ipAddressStr);
        logMsg = "Received data:"s +
                 "\n\tSource Ip Address: "s + ipAddressStr +
                 "\n\tSource Port: "s + std::to_string(ntohs(sourcePort)) +
                 "\n\tData Received: "s + std::to_string(receivedLength) + " bytes.\n";

        Log::Debug (logMsg);
#endif

        totalReceived++;
        totalData += receivedLength;

       
        bool isValid;
        isValid = msg->isValidType (dataBuffer, receivedLength);

        if (!isValid) {
            Log::Error ("Received packet is of invalid type.");
            continue;


        bool retVal;
        retVal = msg->setRawData (dataBuffer, receivedLength);

        if (!retVal) {
            Log::Error ("Convert data failed.");
            continue;

        string messageType;
        retVal = msg->msgTypeString (messageType);

        if (!retVal) {
            Log::Error ("Get message type string failed.");
            continue;

        Log::Info ("Received message: "s + messageType + ".  Invoking reactor.");

       
        // Process message data
        //
        retVal = reactor.processMsg (sourceIpAddress, sourcePort, *msg);

        if (!retVal) {
            Log::Error ("Reactor processing failed.");
            continue;
        return false;
    }


    // cleanup
    delete [] dataBuffer;
 
    Log::Info ("Server finished.  Exiting.");
    cout << "Finished." << endl;
    cout << "TOTALS:  PacketsRecv = " << totalReceived << "       DataRecv = " << totalData << endl;


    return 0;
}


void sendConnectRequest (UdpConnection & udpConnection,
                         ulong           peerIpAddress,
                         int             peerPort)
{
    AlpineMessage *  msg;
    msg = new AlpineMessage;

    AlpineMessage::t_ConnRequest  connRequest;
    connRequest.protocolVer = 1;

    msg->setMsgData (connRequest);


    byte * buffer;
    int    buffLen;

    bool  retVal;

    retVal = msg->getRawData (buffer, buffLen);

    if (!retVal) {
        Log::Error ("Get raw buffer failed!");
        return;
    }

    Log::Info ("Sending "s + std::to_string(buffLen) + " bytes to peer.");

    retVal = udpConnection.sendData (peerIpAddress,
                                     peerPort,
                                     buffer,
                                     buffLen);

    if (!retVal) {
        Log::Error ("Send data failed!");
    }
}



