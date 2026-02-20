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
#include <AlpinePeerLocation.h>
#include <vector>


class DataBuffer;
class AlpinePeerOptionData;


class AlpinePeerPacket : public StackLinkInterface
{
  public:

    AlpinePeerPacket ();

    AlpinePeerPacket (StackLinkInterface * parent);

    AlpinePeerPacket (const AlpinePeerPacket & copy);

    virtual ~AlpinePeerPacket ();

    AlpinePeerPacket & operator = (const AlpinePeerPacket & copy);



    AlpinePacket::t_PacketType  getPacketType ();

    bool setPacketType (AlpinePacket::t_PacketType  type);



    ////
    //
    // Data Types
    //
    using t_PeerLocationList = vector<AlpinePeerLocation>;



    ////
    //
    // Alpine Peer operations
    //
    bool  setOptionId (ulong  optionId);

    bool  getOptionId (ulong &  optionId);

    bool  setOptionData (AlpinePeerOptionData *  optionData);

    bool  getOptionData (AlpinePeerOptionData *&  optionData);

    bool  setPeersAvailable (ulong  numAvailable);

    bool  getPeersAvailable (ulong &  numAvailable);

    bool  setOfferId (ulong  offerId);

    bool  getOfferId (ulong &  offerId);

    bool  setOffset (ulong  offset);

    bool  getOffset (ulong &  offset);

    bool  setReplySetSize (ushort  setSize);

    bool  getReplySetSize (ushort &  setSize);

    bool  setPeerLocationList (t_PeerLocationList &  locationList);

    bool  getPeerLocationList (t_PeerLocationList &  locationList);



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
    AlpinePeerOptionData *         optionData_;
    ulong                          numPeersAvailable_;
    ulong                          offerId_;
    ulong                          offset_;
    ushort                         setSize_;
    t_PeerLocationList *           peerLocationList_;


    ulong  calculatePeerListSize ();

    bool   writePeerListData (DataBuffer * linkBuffer);

    bool   readPeerListData (DataBuffer * linkBuffer);

};


