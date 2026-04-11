/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <AlpinePacket.h>
#include <AlpineResourceDesc.h>
#include <Common.h>
#include <StackLinkInterface.h>
#include <vector>


class DataBuffer;
class AlpineQueryOptionData;


class AlpineQueryPacket : public StackLinkInterface
{
  public:
    AlpineQueryPacket();

    AlpineQueryPacket(StackLinkInterface * parent);

    AlpineQueryPacket(const AlpineQueryPacket & copy);

    virtual ~AlpineQueryPacket();

    AlpineQueryPacket & operator=(const AlpineQueryPacket & copy);


    AlpinePacket::t_PacketType getPacketType();

    bool setPacketType(AlpinePacket::t_PacketType type);


    ////
    //
    // Data Types
    //
    using t_ResourceDescList = vector<AlpineResourceDesc>;

    static constexpr ulong MAX_HITS = 10000;


    ////
    //
    // Alpine Query operations
    //
    bool setQueryId(ulong queryId);

    bool getQueryId(ulong & queryId);

    bool setOptionId(ulong optionId);

    bool getOptionId(ulong & optionId);

    bool setOptionData(AlpineQueryOptionData * optionData);

    bool getOptionData(AlpineQueryOptionData *& optionData);

    bool setQueryString(const string & queryString);

    bool getQueryString(string & queryString);

    bool setNumHits(ulong numHits);

    bool getNumHits(ulong & numHits);

    bool setUploadSlots(ushort uploadSlots);

    bool getUploadSlots(ushort & uploadSlots);

    bool setOffset(ulong offset);

    bool getOffset(ulong & offset);

    bool setReplySetSize(ushort setSize);

    bool getReplySetSize(ushort & setSize);

    bool setPriority(uint8_t priority);

    bool getPriority(uint8_t & priority);

    bool setTraceContext(const string & traceContext);

    bool getTraceContext(string & traceContext);

    bool setResourceDescList(t_ResourceDescList & resourceList);

    bool getResourceDescList(t_ResourceDescList & resourceList);


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
    ulong queryId_;
    ulong optionId_;
    AlpineQueryOptionData * optionData_;
    string queryString_;
    ulong numHits_;
    ushort uploadSlots_;
    ulong offset_;
    ushort replySetSize_;
    uint8_t priority_{128};
    string traceContext_;
    t_ResourceDescList * resourceList_;


    ulong calculateResourceListSize();

    bool writeResourceListData(DataBuffer * linkBuffer);

    bool readResourceListData(DataBuffer * linkBuffer);

    bool verifyStringData(const byte * data, ulong dataLength);
};
