/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AlpineExtensionIndex.h>
#include <AlpinePacket.h>
#include <AlpineProxyOptionData.h>
#include <AlpineProxyPacket.h>
#include <DataBuffer.h>
#include <Log.h>
#include <NetUtils.h>
#include <StringUtils.h>


AlpineProxyPacket::AlpineProxyPacket()
{
#ifdef _VERBOSE
    Log::Debug("AlpineProxyPacket constructor invoked.");
#endif

    parent_ = nullptr;
    packetType_ = AlpinePacket::t_PacketType::none;
    optionId_ = 0;
    optionData_ = nullptr;
}

AlpineProxyPacket::AlpineProxyPacket(StackLinkInterface * parent)
{
#ifdef _VERBOSE
    Log::Debug("AlpineProxyPacket parent invoked.");
#endif

    parent_ = parent;
    packetType_ = AlpinePacket::t_PacketType::none;
    optionId_ = 0;
    optionData_ = nullptr;
}


AlpineProxyPacket::AlpineProxyPacket(const AlpineProxyPacket & copy)
{
#ifdef _VERBOSE
    Log::Debug("AlpineProxyPacket copy constructor invoked.");
#endif

    parent_ = copy.parent_;
    packetType_ = copy.packetType_;
    optionId_ = copy.optionId_;

    if (copy.optionData_) {
        optionData_ = copy.optionData_->duplicate();
    } else {
        optionData_ = nullptr;
    }
}


AlpineProxyPacket::~AlpineProxyPacket()
{
#ifdef _VERBOSE
    Log::Debug("AlpineProxyPacket destructor invoked.");
#endif

    delete optionData_;
}


AlpineProxyPacket &
AlpineProxyPacket::operator=(const AlpineProxyPacket & copy)
{
#ifdef _VERBOSE
    Log::Debug("AlpineProxyPacket assignment invoked.");
#endif

    if (&copy == this) {
        return *this;
    }

    if (optionData_) {
        delete optionData_;
        optionData_ = nullptr;
    }

    parent_ = copy.parent_;
    packetType_ = copy.packetType_;
    optionId_ = copy.optionId_;

    if (copy.optionData_) {
        optionData_ = copy.optionData_->duplicate();
    }


    return *this;
}


AlpinePacket::t_PacketType
AlpineProxyPacket::getPacketType()
{
#ifdef _VERBOSE
    Log::Debug("AlpineProxyPacket::getPacketType invoked.");
#endif

    return packetType_;
}


bool
AlpineProxyPacket::setPacketType(AlpinePacket::t_PacketType type)
{
#ifdef _VERBOSE
    string packetTypeString;
    AlpinePacket::packetTypeAsString(type, packetTypeString);
    Log::Debug("AlpineProxyPacket::setPacketType invoked.  New type: "s + packetTypeString);
#endif

    if ((type != AlpinePacket::t_PacketType::proxyRequest) && (type != AlpinePacket::t_PacketType::proxyAccepted) &&
        (type != AlpinePacket::t_PacketType::proxyHalt)) {

        Log::Error("Invalid packet type passed in call to AlpineProxyPacket::setPacketType!");
        return false;
    }
    packetType_ = type;

    return true;
}


bool
AlpineProxyPacket::setOptionId(ulong optionId)
{
#ifdef _VERBOSE
    Log::Debug("AlpineProxyPacket::setOptionId invoked.  Option ID: "s + std::to_string(optionId));
#endif

    if (optionId != 0) {
        Log::Error("Attempt to set extended option ID in call to "
                   "AlpineProxyPacket::setOptionId!  Use setOptionData for extended options.");
        return false;
    }
    optionId_ = optionId;

    return true;
}


bool
AlpineProxyPacket::getOptionId(ulong & optionId)
{
#ifdef _VERBOSE
    Log::Debug("AlpineProxyPacket::getOptionId invoked.");
#endif

    optionId = optionId_;

    return true;
}


bool
AlpineProxyPacket::setOptionData(AlpineProxyOptionData * optionData)
{
#ifdef _VERBOSE
    Log::Debug("AlpineProxyPacket::setOptionData invoked.");
#endif

    if (optionData_) {
        delete optionData_;
        optionData_ = nullptr;
    }

    optionId_ = optionData->getOptionId();
    optionData_ = optionData->duplicate();


    return true;
}


bool
AlpineProxyPacket::getOptionData(AlpineProxyOptionData *& optionData)
{
#ifdef _VERBOSE
    Log::Debug("AlpineProxyPacket::getOptionData invoked.");
#endif

    if ((optionId_ == 0) || (!optionData_)) {
        Log::Error("Attempt to get option data when no extension set in call to "
                   "AlpineProxyPacket::getOptionData!");
        return false;
    }
    optionData = optionData_->duplicate();


    return true;
}


bool
AlpineProxyPacket::setParent(StackLinkInterface * parent)
{
#ifdef _VERBOSE
    Log::Debug("AlpineProxyPacket::setParent invoked.");
#endif

    parent_ = parent;

    return true;
}


void
AlpineProxyPacket::unsetParent()
{
#ifdef _VERBOSE
    Log::Debug("AlpineProxyPacket::unsetParent invoked.");
#endif

    parent_ = nullptr;
}


bool
AlpineProxyPacket::writeData(DataBuffer * linkBuffer)
{
#ifdef _VERBOSE
    Log::Debug("AlpineProxyPacket::writeData invoked.");
#endif

    // We only need to write data if this is a proxy request or accept (for now at least)
    //
    if (packetType_ == AlpinePacket::t_PacketType::proxyHalt) {
        return true;
    }
    ////
    //
    // Write data
    //
    // 4b - Proxy Option ID
    // 0-Nb - Proxy Option Data
    //
    bool status;
    byte * buffer;
    byte * curr;
    uint bufferSize;
    uint writeLength;

    status = linkBuffer->getWriteBuffer(buffer, bufferSize);

    if (!status) {
        // no room left to write to?
        Log::Debug("getWriteBuffer failed in AlpineProxyPacket::writeData.");

        return false;
    }
    // Calculate length of packet
    //
    writeLength = sizeof(long);

    if ((optionId_) && (optionData_)) {
        writeLength += optionData_->getOptionDataLength();
    }

    if (bufferSize < writeLength) {
        Log::Error("Not enough buffer space to write packet data in call to "
                   "AlpineProxyPacket::writeData!");
        return false;
    }
    // Write packet data
    //
    curr = buffer;
    *(reinterpret_cast<ulong *>(curr)) = htonl(optionId_);
    curr += sizeof(long);

    linkBuffer->addWriteBytes(writeLength);

    // If extended query options given, write them into the buffer.
    //
    if ((optionId_) && (optionData_)) {
        status = optionData_->writeData(linkBuffer);

        if (!status) {
            Log::Error("writeData failed for Option Data in call to "
                       "AlpineProxyPacket::writeData!");
            return false;
        }
    }


    if (parent_) {
        // We have a parent link set, have parent write data,
        //
        status = parent_->writeData(linkBuffer);

        if (!status) {
            return false;
        }
        return false;
    }


    return true;
}


bool
AlpineProxyPacket::readData(DataBuffer * linkBuffer)
{
#ifdef _VERBOSE
    Log::Debug("AlpineProxyPacket::readData invoked.");
#endif

    // We only need to read data if this is a proxy request or accept.
    //
    if (packetType_ == AlpinePacket::t_PacketType::proxyHalt) {
        return true;
    }
    ////
    //
    // Read data
    //
    // 4b - Proxy Option ID
    // 0-Nb - Proxy Option Data
    //
    bool status;
    byte * buffer;
    byte * curr;
    uint bufferSize;
    uint readLength;

    status = linkBuffer->getReadBuffer(buffer, bufferSize);

    if (!status) {
        Log::Debug("getReadBuffer failed in AlpineProxyPacket::readData.");

        return false;
    }
    readLength = sizeof(long);

    if (bufferSize < readLength) {
#ifdef _VERBOSE
        Log::Debug("Packet size too small in AlpineProxyPacket::readData!");
#endif
        return false;
    }
    curr = buffer;
    optionId_ = static_cast<ulong>(ntohl(*(reinterpret_cast<ulong *>(curr))));
    linkBuffer->addReadBytes(readLength);


    // If extended query options given, read them into the buffer.
    //
    if (optionId_) {
        if (optionData_) {
            delete optionData_;
            optionData_ = nullptr;
        }

        status = AlpineExtensionIndex::getProxyOptionExt(optionId_, optionData_);

        if (!status) {
#ifdef _VERBOSE
            Log::Error("Attempt to get ProxyOptionExt(Data) failed for OptionID in call to "
                       "AlpineProxyPacket::readData!");
#endif
            return false;
        }
        status = optionData_->readData(linkBuffer);

        if (!status) {
#ifdef _VERBOSE
            Log::Error("readData failed for Option Data in call to "
                       "AlpineProxyPacket::readData!");
#endif
            return false;
        }
    }


    return true;
}
