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
#include <StackLinkInterface.h>


class DataBuffer;


class DtcpDataPacket : public StackLinkInterface
{
  public:

    DtcpDataPacket ();

    DtcpDataPacket (StackLinkInterface * parent);

    virtual ~DtcpDataPacket ();



    ////
    //
    // StackLink operations
    //
    virtual bool  setParent (StackLinkInterface *  parent);

    virtual void  unsetParent ();

    virtual bool  writeData (DataBuffer * linkBuffer);

    virtual bool  readData (DataBuffer * linkBuffer);


    
    ////
    //
    // Data packet operations
    //
    virtual bool  setPacketData (const byte * data,
                                 const uint   dataLength);

    virtual bool  getPacketData (byte *& data,
                                 uint &  dataLength);


  protected:

    StackLinkInterface * parent_;
    const byte *         setData_;
    uint                 setDataLength_;
    byte *               getData_;
    uint                 getDataLength_;

};


