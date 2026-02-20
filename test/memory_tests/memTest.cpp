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


#include <DtcpMemTest.h>
#include <iostream>
using std::cout; using std::endl; using std::cerr;
#include <stdlib.h>
#include <unistd.h>
#include <vector>
#include <Log.h>
#include <StringUtils.h>


int 
main (int argc, char *argv[])
{
    string logFilename;
    int debugLevel;

    if (argc != 4) {
        cerr << "Usage: " << argv[0] << " <debug filename> <debugLevel 1-4> <num connections>" << endl;
        return 1;
    }

    logFilename = argv[1];
    debugLevel = atoi (argv[2]);

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

    int numPeers = atoi (argv[3]);

#ifndef _LEAN_N_MEAN
    Log::Info ("Starting memory test.  Num peer connections: "s +
               std::to_string ((ulong)numPeers));
#else
    Log::Info ("Starting -LEAN- memory test (optimized client for memory footprint).  "
                        "Num peer connections: " +
               std::to_string ((ulong)numPeers));
#endif


    DtcpMemTest  tester;

    Log::Info ("Sleeping before allocation proceedes..");
    sleep (20);

    Log::Info ("Allocating transport data...");
    tester.allocate ((ulong)numPeers);
    Log::Info ("Allocation complete.  Sleeping...");

    sleep (60);

    Log::Info ("Cleaning up before exit.");
    tester.cleanUp ();

 
    return 0;
}



