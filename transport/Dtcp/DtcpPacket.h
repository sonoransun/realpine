/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <StackLinkInterface.h>


constexpr size_t DtcpMaxPacketSize = 5 * 1024;


class DataBuffer;


class DtcpPacket : public StackLinkInterface
{
  public:
    DtcpPacket();

    DtcpPacket(const DtcpPacket & copy);

    virtual ~DtcpPacket();

    DtcpPacket & operator=(const DtcpPacket & copy);


    ////
    //
    // StackLink operations
    //
    virtual bool setParent(StackLinkInterface * parent);

    virtual void unsetParent();

    virtual bool writeData(DataBuffer * linkBuffer);

    virtual bool readData(DataBuffer * linkBuffer);


    ////
    //
    // some dtcp protocol constants
    //
    enum class t_PacketType {
        none = 0,  // uninitialized

        connRequest = 1,
        connOffer = 2,
        connAccept = 3,
        connSuspend = 4,
        connResume = 5,
        connData = 6,
        connReliableData = 7,
        connDataAck = 8,
        connClose = 9,

        poll = 10,
        ack = 11,

        error = 12,

        natDiscover = 13,
        natSource = 14,

        txnData = 15  // to be handled by transaction mux

    };


    t_PacketType getPacketType();

    bool setPacketType(t_PacketType type);

    static bool packetTypeAsString(t_PacketType type, string & typeString);


    ////
    //
    // Misc
    //
    bool getPeerLocation(ulong & ipAddress, ushort & port);

    bool setPeerLocation(ulong ipAddress, ushort port);


  protected:
    StackLinkInterface * parent_;

    static const ulong magicCookie_s;
    static const ushort version_s;

    t_PacketType packetType_;

    ulong peerIpAddress_;
    ushort peerPort_;
};
