/// Copyright (C) 2026 sonoransun — see LICENCE.txt



#include <DataBuffer.h>
#include <Log.h>
#include <StringUtils.h>



DataBuffer::DataBuffer (uint bufferSize)
    : buffer_(bufferSize),
      dataSize_(0),
      readOffset_(0),
      readRemaining_(0),
      writeOffset_(0),
      writeRemaining_(bufferSize)
{
#ifdef _VERBOSE
    Log::Debug ("DataBuffer constructor invoked.");
#endif
}



DataBuffer::DataBuffer (const DataBuffer & copy)
    : buffer_(copy.buffer_),
      dataSize_(copy.dataSize_),
      readOffset_(copy.readOffset_),
      readRemaining_(copy.readRemaining_),
      writeOffset_(copy.writeOffset_),
      writeRemaining_(copy.writeRemaining_)
{
#ifdef _VERBOSE
    Log::Debug ("DataBuffer copy constructor invoked.");
#endif
}



DataBuffer::DataBuffer (const byte * data,
                        uint         dataLength)
    : buffer_(data, data + dataLength),
      dataSize_(dataLength),
      readOffset_(0),
      readRemaining_(dataLength),
      writeOffset_(dataLength),
      writeRemaining_(0)
{
#ifdef _VERBOSE
    Log::Debug ("DatBuffer raw buffer constructor invoked.");
#endif
}



DataBuffer::~DataBuffer ()
{
#ifdef _VERBOSE
    Log::Debug ("DataBuffer destructor invoked.");
#endif
}



DataBuffer &
DataBuffer::operator = (const DataBuffer & copy)
{
#ifdef _VERBOSE
    Log::Debug ("DataBuffer::operator = invoked.");
#endif

    if (&copy == this) {
        return *this;
    }

    buffer_         = copy.buffer_;
    dataSize_       = copy.dataSize_;

    readOffset_     = copy.readOffset_;
    readRemaining_  = copy.readRemaining_;

    writeOffset_    = copy.writeOffset_;
    writeRemaining_ = copy.writeRemaining_;

    return *this;
}



bool
DataBuffer::resize (uint size)
{
#ifdef _VERBOSE
    Log::Debug ("DataBuffer::resize invoked.");
#endif

    if (size == buffer_.size()) {
        return true;
    }

    buffer_.resize(size);
    dataSize_   = 0;

    readOffset_     = 0;
    readRemaining_  = dataSize_;

    writeOffset_    = 0;
    writeRemaining_ = buffer_.size();

    return true;
}



void
DataBuffer::clear ()
{
#ifdef _VERBOSE
    Log::Debug ("DataBuffer::clear invoked.");
#endif

    dataSize_ = 0;

    readOffset_     = 0;
    readRemaining_  = dataSize_;

    writeOffset_    = 0;
    writeRemaining_ = buffer_.size();
}



void
DataBuffer::readReset ()
{
#ifdef _VERBOSE
    Log::Debug ("DataBuffer::readReset invoked.");
#endif

    readOffset_    = 0;
    readRemaining_ = dataSize_;
}



void
DataBuffer::writeReset ()
{
#ifdef _VERBOSE
    Log::Debug ("DataBuffer::writeReset invoked.");
#endif

    writeOffset_    = 0;
    writeRemaining_ = buffer_.size();
    dataSize_       = 0;

    // We also need to perform a read reset to make sure
    // our data is consistant.
    //
    readOffset_    = 0;
    readRemaining_ = dataSize_;
}



bool
DataBuffer::setData (const byte * data,
                     uint         dataLength)
{
#ifdef _VERBOSE
    Log::Debug ("DataBuffer::setData invoked.");
#endif

    if (buffer_.size() < dataLength) {
        buffer_.resize(dataLength);
    }

    dataSize_ = dataLength;

    memcpy (buffer_.data(), data, dataSize_);

    readOffset_     = 0;
    readRemaining_  = dataSize_;

    writeOffset_    = buffer_.size();
    writeRemaining_ = 0;

    return true;
}



bool
DataBuffer::getData (byte *& data,
                     uint &  dataLength)
{
#ifdef _VERBOSE
    Log::Debug ("DataBuffer::getData invoked.");
#endif

    data = buffer_.data();
    dataLength = dataSize_;

    return true;
}



bool
DataBuffer::getReadBuffer (byte *& buffer,
                           uint &  bufferSize)
{
#ifdef _VERBOSE
    Log::Debug ("DataBuffer::getReadBuffer invoked.");
#endif

    if (readRemaining_ == 0) {
        return false;
    }

    buffer     = buffer_.data() + readOffset_;
    bufferSize = readRemaining_;

    return true;
}



bool
DataBuffer::addReadBytes (uint  bytes)
{
#ifdef _VERBOSE
    Log::Debug ("DataBuffer::addReadBytes invoked.");
#endif

    if (bytes > readRemaining_) {
        // Something is very wrong, this should not happen.
        return false;
    }

    readRemaining_ -= bytes;
    readOffset_    += bytes;

    return true;
}


    
bool
DataBuffer::getWriteBuffer (byte *& buffer,
                            uint &  bufferSize)
{
#ifdef _VERBOSE
    Log::Debug ("DataBuffer::getWriteBuffer invoked.");
#endif

    if (writeRemaining_ == 0) {
        return false;
    }

    buffer     = buffer_.data() + writeOffset_;
    bufferSize = writeRemaining_;

    return true;
}



bool
DataBuffer::addWriteBytes (uint bytes)
{
#ifdef _VERBOSE
    Log::Debug ("DataBuffer::addWriteBytes invoked.");
#endif

    if (bytes > writeRemaining_) {
        // Something is very wrong, this should not happen.
        return false;
    }

    writeRemaining_ -= bytes;
    writeOffset_    += bytes;
    dataSize_       += bytes;

    return true;
}

    

