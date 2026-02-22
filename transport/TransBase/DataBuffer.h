/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <vector>
#include <cstdint>


class DataBuffer
{
  public:

    DataBuffer (uint bufferSize);

    DataBuffer (const DataBuffer & copy);

    DataBuffer (const byte * data,
                uint         dataLength);

    virtual ~DataBuffer ();

    virtual DataBuffer & operator = (const DataBuffer & copy);


    ////
    //
    // DataBuffer operations
    //
    virtual bool  resize (uint size);

    virtual void  clear ();

    virtual void  readReset ();

    virtual void  writeReset ();

    virtual bool  setData (const byte * data,
                           uint         dataLength);

    virtual bool  getData (byte *& data,
                           uint &  dataLength);

    virtual bool  getReadBuffer (byte *& buffer,
                                 uint &  bufferSize);

    virtual bool  addReadBytes (uint  bytes);

    virtual bool  getWriteBuffer (byte *& buffer,
                                  uint &  bufferSize);

    virtual bool  addWriteBytes (uint bytes);



  private:

    std::vector<uint8_t>  buffer_;
    uint    dataSize_;

    uint    readOffset_;
    uint    readRemaining_;

    uint    writeOffset_;
    uint    writeRemaining_;

};


