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


#include <Common.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <Log.h>
#include <StringUtils.h>
#include <SpawnProcess.h>



int 
main (int argc, char *argv[])
{
    if (argc != 6) {
        cerr << "Usage: " << argv[0] << " <debug filename> <debugLevel 1-4> "
                "<command> <arguments (quoted)> <create count>" << endl;
        return 1;
    }

    string logFilename;
    int debugLevel;
    string command;
    string arguments;
    int createCount;

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

    command = argv[3];
    arguments = argv[4];
    createCount = atoi (argv[5]);



    Log::Info ("Starting spawn process test."s +
               "\nCommand: "s + command +
               "\nArguments: "s + arguments +
               "\nCreate Count: "s + std::to_string (createCount));


    SpawnProcess  spawnMaker;

    spawnMaker.setCommand (command);
    spawnMaker.setArguments (arguments);

    int i;
    for (i = 0; i < createCount; i++) {
        spawnMaker.spawnProcess ();
    }

 
    return 0;
}


