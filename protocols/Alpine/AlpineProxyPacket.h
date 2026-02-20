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
#include <AlpinePacket.h>


class DataBuffer;
class AlpineProxyOptionData;


class AlpineProxyPacket : public StackLinkInterface
{
  public:

    AlpineProxyPacket ();

    AlpineProxyPacket (StackLinkInterface * parent);

    AlpineProxyPacket (const AlpineProxyPacket & copy);

    virtual ~AlpineProxyPacket ();

    AlpineProxyPacket & operator = (const AlpineProxyPacket & copy);



    AlpinePacket::t_PacketType  getPacketType ();

    bool setPacketType (AlpinePacket::t_PacketType  type);



    ////
    //
    // Alpine Proxy operations
    //
    bool  setOptionId (ulong  optionId);

    bool  getOptionId (ulong &  optionId);

    bool  setOptionData (AlpineProxyOptionData *  optionData);

    bool  getOptionData (AlpineProxyOptionData *&  optionData);



    ////
    //
    // StackLink operations
    //
    virtual bool  setParent (StackLinkInterface *  parent);

    virtual void  unsetParent ();

    virtual bool  writeData (DataBuffer * linkBuffer);

    virtual bool  readData (DataBuffer * linkBuffer);




  protected:

    StackLinkInterface *           parent_;
    AlpinePacket::t_PacketType     packetType_;
    ulong                          optionId_;
    AlpineProxyOptionData *        optionData_;

};


