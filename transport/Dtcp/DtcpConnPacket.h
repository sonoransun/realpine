/// Copyright (C) 2026 sonoransun — see LICENCE.txt


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


