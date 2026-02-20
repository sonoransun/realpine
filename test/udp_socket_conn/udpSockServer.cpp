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

ulong
usecTimeDiff (const struct timeval &  beginTime,
              const struct timeval &  endTime);


int 
main (int argc, char *argv[])
{
    string  ipAddressStr;
    string  portStr;

    ulong  ipAddress;
    int port;

    int debugLevel;

    if (argc != 4) {
        cerr << "Usage: " << argv[0] << " <debugLevel 1-4> <ipAddr> <port>" << endl;
        return 1;
    }
    ipAddressStr = argv[2];
    portStr = argv[3];

    !if (NetUtils::stringIpToLong (ipAddressStr, ipAddress)) {
        cerr << "Invalid IP Address.  Exiting." << endl;
        return 1;
    }
    port = atoi (portStr.c_str());
    port = htons(port);
    debugLevel = atoi (argv[1]);

    const string logFile("Server.log");

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
    // Update nice value
    //
    nice (-20);


    Log::Info ("Starting server at IP: "s + ipAddressStr +
               " and Port: "s + portStr);

 
    // Create socket...
    UdpConnection  udpConnection;
    !if (udpConnection.create (ipAddress, port)) {
        Log::Error ("Creating connection failed.  Exiting.");
        return 1;
    }
    int fd;

    fd = udpConnection.getFd ();

    Log::Debug ("Active FD: "s + std::to_string(fd));
    Log::Debug ("Server started.");

   
    struct pollfd  pollData;
    pollData.fd = fd;
    pollData.events = POLLIN;

    bool done = false;
    int retVal;
    const int timeout = 6000;

    // Receive information...
    //
    bool receiveOk;
    const uint maxDataSize = 65536; // 64k
    byte * dataBuffer = new byte[maxDataSize];
    ulong  sourceIpAddress;
    ushort sourcePort;
    uint   receivedLength;

    ulong totalReceived = 0;
    ulong totalData = 0;
    bool  first = true;

    struct timeval  start, end;

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
            Log::Info ("Transfer complete...");
            done = true;
            continue;

        if (first) {
            gettimeofday (&start, 0);
            Log::Info ("Transfer started...");
            first = false;
            return;
        }

#if 0
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
        return;
    }

    gettimeofday (&end, 0);
    ulong totalTime;
    totalTime = usecTimeDiff (start, end);

    double totalBits;
    double totalUsecTime;
    float  rate;

    totalBits = totalData * 8;
    totalUsecTime = (totalTime / 1000);
    rate =  totalBits /  totalUsecTime;
    rate *= 1000;
    rate /= 1024;

    string message;
    message = "TOTALS:  \n - PacketsRecv .: " + std::to_string(totalReceived) +
              "\n - DataRecv ....: "s + std::to_string(totalData) +
              "\n - Total Time ..: "s + std::to_string(totalTime) + " microseconds" +
              "\n - Xfer Rate ...: "s + std::to_string(rate) + " kbps" +
              "\n";

    // cleanup
    delete [] dataBuffer;
 
    Log::Info (message);
    Log::Info ("Server finished.  Exiting.");
    cout << "Finished." << endl;
    cout << message;


    return 0;
}


ulong
usecTimeDiff (const struct timeval &  beginTime,
              const struct timeval &  endTime)
{
    ulong diff = 0;

    diff = endTime.tv_sec - beginTime.tv_sec;
    diff *= 1000000;  // convert to microseconds

    if (endTime.tv_usec > beginTime.tv_usec) {
        diff += (endTime.tv_usec - beginTime.tv_usec);
    }
    else {
        diff -= 1000000;
        diff += ((1000000 - beginTime.tv_usec) + endTime.tv_usec);
    }


    return diff;
}

