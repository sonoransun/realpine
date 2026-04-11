/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>


////
//
// The TransportInterface is used by the Client and Server Transport Modules
// for individual data transfers which are requested or served.
//
// Persistant or non terminating transfers, such as chat or streaming will have
// no totalDataSize (), but must track all other statistics.
//
////


class AlpineTransportInterface
{
  public:
    AlpineTransportInterface(){};
    virtual ~AlpineTransportInterface(){};


    // Each transport must have a unqiue 4 byte unsigned identifier
    //
    virtual ulong getId() = 0;


    // Duplication of existing transports (handles)
    //
    virtual AlpineTransportInterface * duplicate() = 0;


    // Status
    //
    virtual float averageBandwidth() = 0;

    virtual float peakBandwidth() = 0;

    virtual ulong totalDataSize() = 0;

    virtual ulong xferDataSize() = 0;


    // Control
    //
    virtual bool isActive() = 0;

    virtual bool cancel() = 0;

    virtual bool pause() = 0;

    virtual bool resume() = 0;
};
