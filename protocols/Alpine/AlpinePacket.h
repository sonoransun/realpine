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


class AlpinePacket : public StackLinkInterface
{
  public:

    AlpinePacket ();

    AlpinePacket (const AlpinePacket & copy);

    virtual ~AlpinePacket ();

    AlpinePacket & operator = (const AlpinePacket & copy);



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
    // some alpine protocol constants
    //
    enum class t_PacketType {
        none              = 0, // uninitialized

        queryDiscover     = 1,
        queryOffer        = 2,
        queryRequest      = 3,
        queryReply        = 4,

        peerListRequest   = 5,
        peerListOffer     = 6,
        peerListGet       = 7,
        peerListData      = 8,

        proxyRequest      = 9,
        proxyAccepted     = 10,
        proxyHalt         = 11,

    };


    t_PacketType  getPacketType ();

    bool setPacketType (t_PacketType  type);

    static bool packetTypeAsString (t_PacketType  type,
                                    string &      typeString);


  protected:

    StackLinkInterface * parent_;
    t_PacketType         packetType_;

};


