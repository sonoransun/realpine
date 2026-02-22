/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <DtcpIORecord.h>
#include <DataBuffer.h>
#include <DtcpPacket.h>



DtcpIORecord::DtcpIORecord (uint  bufferSize)
{
    buffer_ = new DataBuffer (bufferSize);
    packet_ = new DtcpPacket ();

    sourceIpAddress_ = 0;
    sourcePort_      = 0;
}


DtcpIORecord::~DtcpIORecord ()
{
    delete buffer_;

    delete packet_;
}



