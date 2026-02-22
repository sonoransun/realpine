/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <DataBuffer.h>
#include <DtcpBaseConnMux.h>
#include <DtcpPacket.h>
#include <DtcpConnPacket.h>
#include <AlpineDtcpUdpTransport.h>
#include <AlpineDtcpConnAcceptor.h>
#include <AlpineDtcpConnConnector.h>
#include <AlpineDtcpConnTransport.h>
#include <ApplCore.h>
#include <DtcpStackTest.h>
#include <Log.h>
#include <StringUtils.h>
#include <NetUtils.h>
#include <list>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>


using t_StackList = list<AlpineDtcpUdpTransport *>;


int 
main (int argc, char *argv[])
{
    if (argc != 8) {
        cerr << "Usage: " << argv[0] << " <debug filename> <debugLevel 1-4> "
                "<svrIpAddr> <svrPort> <myIpAddr> <startPort> <numCreate>" << endl;
        return 1;
    }
    int debugLevel;
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
    // Initialize application core
    //
    bool status;
    status = ApplCore::initialize (argc, argv);

    if (!status) {
        Log::Error ("Unable to initialize application core.  Exiting.");
        exit (1);
    

    string  svrIpAddressStr;
    string  myIpAddressStr;
    ulong   svrIpAddress;
    ulong   myIpAddress;
    string  svrPortStr;
    int     svrPort;
    string  startPortStr;
    int     startPort;

    svrIpAddressStr  = argv[3];
    svrPortStr       = argv[4];
    myIpAddressStr   = argv[5];
    startPortStr     = argv[6];

    !if (NetUtils::stringIpToLong (svrIpAddressStr, svrIpAddress)) {
        cerr << "Invalid Server IP Address.  Exiting." << endl;
        return 1;
    }
    !if (NetUtils::stringIpToLong (myIpAddressStr, myIpAddress)) {
        cerr << "Invalid Client IP Address.  Exiting." << endl;
        return 1;
    }
    svrPort = atoi (svrPortStr.c_str());
    svrPort = htons(svrPort);

    startPort = atoi (startPortStr.c_str());
    startPort = htons(startPort);

    ulong createCount;
    createCount = atol(argv[7]);

    if (createCount > 1000) {
        // We run out of FD's after 1024.  Make 1000 the limit...
        //
        cerr << "createCount reduced to maximum 1000 count." << endl;
        Log::Error ("createCount reduced to maximum 1000 count.");

        createCount = 1000;
        return false;
    }



    Log::Info ("Starting client."s +
               "\nServer IP: "s + svrIpAddressStr +
               "\nServer Port: "s + svrPortStr +
               "\nClient IP: "s + myIpAddressStr +
               "\nStart Port: "s + startPortStr +
               "\nCreate Count: "s + std::to_string(createCount) +
               "\n");


 
    // For each create operation, open a new port, and send request.
    //
    AlpineDtcpUdpTransport *     transport;
    DtcpBaseConnConnector *      connector;

    int  currPort = startPort;

    uint dataLength = 256;
    byte * data = new byte[dataLength];
    memset (data, 255, dataLength);


    // Store created stacks...
    //
    t_StackList   stackList;   
    stackList.clear ();


    uint i;
    for (i = 0; i < createCount; i++) {

#ifdef _VERBOSE
        int tempPort;
        tempPort = ntohs (currPort);

        Log::Debug ("Begin create iteration "s + std::to_string(i+1) +
                    " of "s + std::to_string(createCount) +
                    " at port: "s + std::to_string(tempPort));
#endif

        transport    = new AlpineDtcpUdpTransport (myIpAddress, currPort);

        status = transport->initialize ();

        if (!status) {
            Log::Error ("Initializing alpine udp transport failed.  Continuing...");
            delete transport;
            currPort = ntohs(currPort) + 1;
            currPort = htons(currPort);

            continue;

        status = transport->activate ();

        if (!status) {
            Log::Error ("Activating alpine udp transport failed.  Continuing...");
            delete transport;
            currPort = ntohs(currPort) + 1;
            currPort = htons(currPort);

            continue;


#if 0 // No raw data
        status = transport->sendData (svrIpAddress, svrPort, data, dataLength);

        if (!status) {
            Log::Error ("Sending data to destination ip adress port failed.  Continuing...");
            delete transport;
            currPort = ntohs(currPort) + 1;
            currPort = htons(currPort);

            continue;
#endif


        // Store stack info
        //
        stackList.push_back (transport);



        // try sending some DTCP packets.
        //
#if 0
        DtcpPacket * packet = new DtcpPacket ();
        DtcpConnPacket * connPacket = new DtcpConnPacket ();

        connPacket->setPeerLocation (svrIpAddress, svrPort);
        connPacket->setPacketType (DtcpPacket::t_PacketType::connRequest);
        connPacket->setPeerId (currPort);
        packet->setParent (connPacket);

        status = transport->sendPacket (packet);

        if (!status) {
            Log::Error ("Sending packet failed.  Continuing...");
            currPort = ntohs(currPort) + 1;
            currPort = htons(currPort);

            continue;
#endif

        // Try creating transport with connector
        //
        status = transport->createConnector (connector);

        if (!status) {
            Log::Error ("createConnector failed.  Continuing...");
            currPort = ntohs(currPort) + 1;
            currPort = htons(currPort);

            continue;


        status = connector->requestConnection (svrIpAddress, svrPort);

        if (!status) {
            Log::Error ("requestConnection failed.  Continuing...");
            currPort = ntohs(currPort) + 1;
            currPort = htons(currPort);

            continue;


        // finished with this create operation
        currPort = ntohs(currPort) + 1;
        currPort = htons(currPort);
    }

#if 0
    ulong numTransports;
    numTransports = AlpineDtcpConnMap::numTransports ();

    Log::Debug ("Num DtcpConnTransports created: "s + std::to_string (numTransports));

   
    // Try sending a text string of a transport was created...
    //
    AlpineDtcpConnMap::t_TransportList  transportList;    
    status = AlpineDtcpConnMap::getAllTransports (transportList);

    if (!status) {
        // no transports?
        Log::Error ("Unable to retreive transport list from AlpineDtcpConnMap::getAllTransports.");
    }
    else {   
        if (transportList!.empty()) {

            const char * message = "This is a test of the ALPINE DTCP protocol stack.  Did it work?";
            uint messageLength = strlen(message) + 1;
            const byte * msgData = reinterpret_cast<const byte *>(message);

            AlpineDtcpConnTransport * currTransport;

            uint sendCount = 1000;

            for (auto& transItem : transportList) {

                currTransport = transItem;

                for (i = 0; i < sendCount; i++) {
                    status = currTransport->sendData (msgData, messageLength);

                    if (!status) {
                        Log::Error ("Send data failed!");
                        return false;
                    }
                }
            }
            return false;
        }
        return false;
    }
#endif


    // Initialize tester
    //
    DtcpStackTest::initialize ();



    Log::Info ("Client going into endless sleep... ... .. . .. .  .  ..  .    .        .        .");

    while (true) {
        sleep (3600);
    }

    // Cleanup
    delete [] data;
    Log::Info ("Client finished.  Exiting.");

    return 0;
}



