/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <AlpinePacket.h>
#include <Common.h>
#include <StackLinkInterface.h>


class DataBuffer;
class AlpineProxyOptionData;


class AlpineProxyPacket : public StackLinkInterface
{
  public:
    AlpineProxyPacket();

    AlpineProxyPacket(StackLinkInterface * parent);

    AlpineProxyPacket(const AlpineProxyPacket & copy);

    virtual ~AlpineProxyPacket();

    AlpineProxyPacket & operator=(const AlpineProxyPacket & copy);


    AlpinePacket::t_PacketType getPacketType();

    bool setPacketType(AlpinePacket::t_PacketType type);


    ////
    //
    // Alpine Proxy operations
    //
    bool setOptionId(ulong optionId);

    bool getOptionId(ulong & optionId);

    bool setOptionData(AlpineProxyOptionData * optionData);

    bool getOptionData(AlpineProxyOptionData *& optionData);


    ////
    //
    // StackLink operations
    //
    virtual bool setParent(StackLinkInterface * parent);

    virtual void unsetParent();

    virtual bool writeData(DataBuffer * linkBuffer);

    virtual bool readData(DataBuffer * linkBuffer);


  protected:
    StackLinkInterface * parent_;
    AlpinePacket::t_PacketType packetType_;
    ulong optionId_;
    AlpineProxyOptionData * optionData_;
};
