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
using std::cout; using std::endl;
#include <stdlib.h>
#include <unistd.h>
#include <vector>
#include <Log.h>
#include <NetManager.h>


int 
main (int argc, char *argv[])
{
    NetManager::initialize ();

    int debugLevel;
    int createCount;
    int connectionCount;

    if (argc != 6) {
        cout << "Usage: " << argv[0] << " <debug lvl> <creates> <messages> <size> <ttl>" << endl;
        cout << "         debug lvl = 1-Silent  2-Error  3-Info  4-Debug" << endl;
        exit (1);
    }
    else {
        debugLevel = atoi(argv[1]);
        createCount = atoi(argv[2]);
        messageCount = atoi(argv[3]);
        messageSize = atoi(argv[4]);
        ttlCount = atoi(argv[5]);
    }

    const string logFile("model.log");

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
        exit (1);
    }

    Log::Info ("\n======== Begin Test ========"s +
               "\nCreate Count ..: "s + std::to_string(createCount) +
               "\nMsg Count .....: "s + std::to_string(messageCount) +
               "\nMsg Size ......: "s + std::to_string(messageSize) +
               "\nTTL Values ....: "s + std::to_string(ttlCount) +
               "\n============================\n\n");


    Log::Info ("<main> Creating "s + std::to_string(createCount) + " clients...");
    NetManager::createClients (createCount, 56000);

    
    Log::Info ("<main> Starting event cycle.");

    int cycles = 5;
    int i;
    string statusMsg;

    for (i = 0; i < cycles; i++) {

        Log::Info ("<main> Begin cycle: "s + std::to_string(i));

        NetManager::sendMessages (messageCount, messageSize, ttlCount);
        NetManager::processCycle ();
        NetManager::status (statusMsg);    
        Log::Debug (statusMsg);

        Log::Info ("<main> End cycle: "s + std::to_string(i) + "\n\n");
    }

    Log::Info ("Test complete.  Exiting.");

    return 0;
}
