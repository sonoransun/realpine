/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <Common.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <functional>
#include <Log.h>
#include <StringUtils.h>

using std::cout; using std::cerr; using std::endl;
#include <OptHash.h>


using t_IndexMap = std::unordered_map < ulong,
                   byte *,
                   OptHash<ulong>,
                   equal_to<ulong> >;

using t_IndexMapPair = std::pair <ulong const, byte *>;


void 
populateMap (t_IndexMap * map,
             ulong        mapSize,
             ulong        elementSize);

void
iterateMap (t_IndexMap * map);


int 
main (int argc, char *argv[])
{
    string logFilename;
    int debugLevel;

    if (argc != 5) {
        cerr << "Usage: " << argv[0] << " <debug filename> <debugLevel 1-4> <map size> <element size>" << endl;
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

    ulong mapSize;
    mapSize = atol(argv[3]);

    ulong elementSize;
    elementSize = atol(argv[4]);

    Log::Info ("Starting map test."s +
               "\nMap Size: "s + std::to_string (mapSize) +
               "\nElement Size: "s + std::to_string (elementSize));



    t_IndexMap *  map = new t_IndexMap;
    map->reserve (mapSize + (ulong)(mapSize * 0.25));

    populateMap (map, mapSize, elementSize);

    Log::Info ("Inserts complete.  Current map size: "s + std::to_string (map->size ()) );



    Log::Info ("Starting map iteration.");

    iterateMap (map);

    Log::Info ("Iterations complete.");


#ifndef _PROFILE
    sleep (3600);
#endif

 
    return 0;
}


void 
populateMap (t_IndexMap * map,
             ulong        mapSize,
             ulong        elementSize)
{
    ulong i;
    byte * currAlloc;

    for (i = 0; i < mapSize; i++) {
        currAlloc = new byte[elementSize];
        memset (currAlloc, 255, elementSize);

        map->insert ( t_IndexMapPair (i, currAlloc) );
    }
}


void
iterateMap (t_IndexMap * map)
{
    ulong i;
    ulong size;
    byte * currData;

    size = map->size ();

    for (i = 0; i < size; i++) {
        auto iter = map->find (i);

        if (iter == map->end ()) {
            Log::Error ("Map is missing key index: "s + std::to_string (i));
            continue;
        }

        currData = (*iter).second;
        *currData = 0;
    }
}
