/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>


class StackLinkInterface;
class MuxInterface;
class DataBuffer;


class TransportInterface
{
  public:
    TransportInterface() = default;
    virtual ~TransportInterface();


    // data buffer for processing requests or sending packets.
    //
    virtual bool getDataBuffer(DataBuffer *& dataBuffer) = 0;


    // process incoming data
    //
    virtual bool processData(const byte * data, uint dataLength) = 0;

    virtual bool processPacket(StackLinkInterface * packet) = 0;


    // Send outgoing
    //
    virtual bool sendPacket(StackLinkInterface * packet) = 0;
};
