/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>


class DataBuffer;
class DtcpPacket;


class DtcpIORecord
{
  public:

    DtcpIORecord (uint  bufferSize);

    ~DtcpIORecord ();



    // This is treated like a structure for now...
    //
    DataBuffer *  buffer_;
    DtcpPacket *  packet_;
    ulong         sourceIpAddress_;
    ushort        sourcePort_;



  private:

    DtcpIORecord (const DtcpIORecord & copy);
    DtcpIORecord & operator = (const DtcpIORecord & copy);

};

