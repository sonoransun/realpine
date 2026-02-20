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
#include <DtcpPacket.h>


class DataBuffer;


class DtcpConnPacket : public StackLinkInterface
{
  public:

    DtcpConnPacket ();

    DtcpConnPacket (StackLinkInterface * parent);

    DtcpConnPacket (const DtcpConnPacket & copy);

    virtual ~DtcpConnPacket ();

    DtcpConnPacket & operator = (const DtcpConnPacket & copy);



    DtcpPacket::t_PacketType  getPacketType ();

    bool setPacketType (DtcpPacket::t_PacketType  type);



    ////
    //
    // Dtcp Connection operations
    //
    bool setPeerId (ulong peerId);

    bool getPeerId (ulong & peerId);

    bool setMyId (ulong myId);

    bool getMyId (ulong & myId);

    bool getPeerLocation (ulong &   ipAddress,
                          ushort &  port);

    bool setPeerLocation (ulong   ipAddress,
                          ushort  port);

    bool getPrevLocation (ulong &   ipAddress,
                          ushort &  port);

    bool setPrevLocation (ulong   ipAddress,
                          ushort  port);

    bool setSequenceNum (ulong  sequenceNum);

    bool getSequenceNum (ulong & sequenceNum);





    ////
    //
    // StackLink operations
    //
    virtual bool  setParent (StackLinkInterface *  parent);

    virtual void  unsetParent ();

    virtual bool  writeData (DataBuffer * linkBuffer);

    virtual bool  readData (DataBuffer * linkBuffer);




  protected:

    StackLinkInterface * parent_;
    ulong                peerId_;
    ulong                myId_;
    ulong                peerIpAddress_;
    ushort               peerPort_;
    ulong                prevIpAddress_;
    ushort               prevPort_;
    ulong                sequenceNum_;

    DtcpPacket::t_PacketType     packetType_;


};


