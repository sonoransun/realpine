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


