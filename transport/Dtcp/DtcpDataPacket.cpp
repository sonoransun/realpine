/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <DtcpDataPacket.h>
#include <DataBuffer.h>
#include <Log.h>
#include <StringUtils.h>


DtcpDataPacket::DtcpDataPacket ()
{
#ifdef _VERBOSE
    Log::Debug ("DtcpDataPacket constructor invoked.");
#endif

    parent_        = nullptr;
    setData_       = nullptr;
    setDataLength_ = 0;
    getData_       = nullptr;
    getDataLength_ = 0;
}



DtcpDataPacket::DtcpDataPacket (StackLinkInterface * parent)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpDataPacket stack link constructor invoked.");
#endif

    parent_        = parent;
    setData_       = nullptr;
    setDataLength_ = 0;
    getData_       = nullptr;
    getDataLength_ = 0;
}



DtcpDataPacket::~DtcpDataPacket ()
{
#ifdef _VERBOSE
    Log::Debug ("DtcpDataPacket destructor invoked.");
#endif

}



bool  
DtcpDataPacket::setParent (StackLinkInterface *  parent)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpDataPacket::setParent invoked.");
#endif

    parent_ = parent;

    return true;
}



void  
DtcpDataPacket::unsetParent ()
{
#ifdef _VERBOSE
    Log::Debug ("DtcpDataPacket::unsetParent invoked.");
#endif

    parent_ = nullptr;
}



bool  
DtcpDataPacket::writeData (DataBuffer * linkBuffer)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpDataPacket::writeData invoked.");
#endif

    bool   status;
    byte * buffer;
    uint   bufferSize;
    uint   writeLength = 0;

    status = linkBuffer->getWriteBuffer (buffer, bufferSize);

    if (!status) {
        // no room left to write to?
        return false;
    }
    if (setDataLength_ > bufferSize) {
        // Not enough room in packet buffer for data...
        return false;
    }
    writeLength = setDataLength_;
    memcpy (buffer, setData_, writeLength);

    linkBuffer->addWriteBytes (writeLength);


    if (parent_) {
        // We have a parent link set, have parent write data,
        //
        status = parent_->writeData (linkBuffer);

        if (!status) {
            return false;

        }
        return false;
    }


    return true;
}



bool  
DtcpDataPacket::readData (DataBuffer * linkBuffer)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpDataPacket::readData invoked.");
#endif


    bool   status;
    byte * buffer;
    uint   bufferSize;
    uint   readLength = 0;

    status = linkBuffer->getReadBuffer (buffer, bufferSize);

    if (!status) {
        // nothing left to read?
        return false;
    }
    getData_ = buffer;
    getDataLength_ = bufferSize;
    readLength = bufferSize;

    linkBuffer->addReadBytes (readLength);


    if (parent_) {
        // We have a parent link set, have parent read data...
        status = parent_->readData (linkBuffer);

        if (!status) {
            return false;

        }
        return false;
    }


    return true;
}



bool  
DtcpDataPacket::setPacketData (const byte * data,
                               const uint   dataLength)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpDataPacket::setPacketData invoked.");
#endif

    setData_ = data;
    setDataLength_ = dataLength;


    return true;
}



bool  
DtcpDataPacket::getPacketData (byte *& data,
                               uint &  dataLength)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpDataPacket::getPacketData invoked.");
#endif

    data = getData_;
    dataLength = getDataLength_;


    return true;
}



