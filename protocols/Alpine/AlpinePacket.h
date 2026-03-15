/// Copyright (C) 2026 sonoransun — see LICENCE.txt


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
    static constexpr uint16_t PROTOCOL_VERSION = 1;

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

        versionHandshake  = 12,
        versionAccept     = 13,
        queryCancellation = 14,

    };


    t_PacketType  getPacketType ();

    bool setPacketType (t_PacketType  type);

    uint16_t  getProtocolVersion ();

    static bool packetTypeAsString (t_PacketType  type,
                                    string &      typeString);


  protected:

    StackLinkInterface * parent_;
    t_PacketType         packetType_;
    uint16_t             protocolVersion_;

};


