/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AlpinePacket.h>
#include <AlpinePeerPacket.h>
#include <AlpineProxyPacket.h>
#include <AlpineQueryPacket.h>
#include <DataBuffer.h>
#include <Log.h>
#include <NetUtils.h>
#include <StringUtils.h>


AlpinePacket::AlpinePacket()
{
#ifdef _VERBOSE
    Log::Debug("AlpinePacket constructor invoked.");
#endif

    parent_ = nullptr;
    packetType_ = t_PacketType::none;
    protocolVersion_ = PROTOCOL_VERSION;
}


AlpinePacket::AlpinePacket(const AlpinePacket & copy)
{
#ifdef _VERBOSE
    Log::Debug("AlpinePacket copy constructor invoked.");
#endif

    parent_ = copy.parent_;
    packetType_ = copy.packetType_;
    protocolVersion_ = copy.protocolVersion_;
}


AlpinePacket::~AlpinePacket()
{
#ifdef _VERBOSE
    Log::Debug("AlpinePacket destructor invoked.");
#endif

    // Nothing to do here
}


AlpinePacket &
AlpinePacket::operator=(const AlpinePacket & copy)
{
#ifdef _VERBOSE
    Log::Debug("AlpinePacket assignment invoked.");
#endif

    if (&copy == this) {
        return *this;
    }

    parent_ = copy.parent_;
    packetType_ = copy.packetType_;
    protocolVersion_ = copy.protocolVersion_;


    return *this;
}


bool
AlpinePacket::setParent(StackLinkInterface * parent)
{
#ifdef _VERBOSE
    Log::Debug("AlpinePacket::setParent invoked.");
#endif

    parent_ = parent;


    return true;
}


void
AlpinePacket::unsetParent()
{
#ifdef _VERBOSE
    Log::Debug("AlpinePacket::unsetParent invoked.");
#endif

    parent_ = nullptr;
}


bool
AlpinePacket::writeData(DataBuffer * linkBuffer)
{
#ifdef _VERBOSE
    Log::Debug("AlpinePacket::writeData invoked.");
#endif

    ////
    //
    // Write data; all we really write at this layer is the packet type
    //
    // ushort (2 bytes)  -- Packet Type
    //
    bool status;
    byte * buffer;
    byte * curr;
    uint bufferSize;
    uint writeLength;

    status = linkBuffer->getWriteBuffer(buffer, bufferSize);

    if (!status) {
        // no room left to write to?
        return false;
    }
    if (parent_) {
        // If parent is any of the higher layer Alpine Packet types, update our packet type.
        //
        bool finished = false;

        if (!finished) {
            AlpineQueryPacket * queryPacket;
            queryPacket = dynamic_cast<AlpineQueryPacket *>(parent_);

            if (queryPacket) {
                packetType_ = queryPacket->getPacketType();
                finished = true;
            }
        }

        if (!finished) {
            AlpinePeerPacket * peerPacket;
            peerPacket = dynamic_cast<AlpinePeerPacket *>(parent_);

            if (peerPacket) {
                packetType_ = peerPacket->getPacketType();
                finished = true;
            }
        }

        if (!finished) {
            AlpineProxyPacket * proxyPacket;
            proxyPacket = dynamic_cast<AlpineProxyPacket *>(parent_);

            if (proxyPacket) {
                packetType_ = proxyPacket->getPacketType();
                finished = true;
            }
        }
    }


    // Versioned wire format:
    //   uint16 (0x0000)           -- version marker (t_PacketType::none, never sent by legacy nodes)
    //   uint16                    -- protocol version
    //   uint16                    -- packet type
    //
    writeLength = sizeof(uint16_t) + sizeof(uint16_t) + sizeof(short);
    if (bufferSize < writeLength) {
        return false;
    }
    curr = buffer;

    // Write version marker (0) to signal versioned packet
    *(reinterpret_cast<uint16_t *>(curr)) = htons(static_cast<uint16_t>(0));
    curr += sizeof(uint16_t);

    // Write protocol version
    *(reinterpret_cast<uint16_t *>(curr)) = htons(protocolVersion_);
    curr += sizeof(uint16_t);

    *(reinterpret_cast<ushort *>(curr)) = htons(static_cast<ushort>(packetType_));
    curr += sizeof(short);

    linkBuffer->addWriteBytes(writeLength);


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
AlpinePacket::readData(DataBuffer * linkBuffer)
{
#ifdef _VERBOSE
    Log::Debug("AlpinePacket::readData invoked.");
#endif

    ////
    //
    // Read data
    //
    // short (2 bytes)  -- Packet Type
    //
    bool status;
    byte * buffer;
    byte * curr;
    uint bufferSize;
    uint readLength;

    status = linkBuffer->getReadBuffer(buffer, bufferSize);

    if (!status) {
#ifdef _VERBOSE
        Log::Debug("getReadBuffer failed in AlpinePacket::readData.");
#endif
        return false;
    }
    readLength = sizeof(short);

    if (bufferSize < readLength) {
#ifdef _VERBOSE
        Log::Debug("Packet size too small in AlpinePacket::readData.");
#endif
        return false;
    }
    curr = buffer;

    // Backward-compatible version detection.
    //
    // Versioned packets:  [uint16 marker=0] [uint16 version] [uint16 packetType]
    // Legacy packets:     [uint16 packetType (1..14)]
    //
    // The marker value 0 corresponds to t_PacketType::none, which legacy nodes
    // never transmit. If the first uint16 is 0, this is a versioned packet.
    // Otherwise, it is a legacy packet and the first uint16 is the packet type.
    //
    auto firstWord = static_cast<uint16_t>(ntohs(*(reinterpret_cast<ushort *>(curr))));

    if (firstWord != 0) {
        // Legacy packet — no version field present
        protocolVersion_ = 0;
        packetType_ = static_cast<t_PacketType>(firstWord);
        readLength = sizeof(short);
    } else {
        // Versioned packet — first word is zero marker, then version, then type
        if (bufferSize < sizeof(uint16_t) + sizeof(uint16_t) + sizeof(short)) {
#ifdef _VERBOSE
            Log::Debug("Packet size too small for versioned header in AlpinePacket::readData.");
#endif
            return false;
        }
        curr += sizeof(uint16_t);  // skip marker
        protocolVersion_ = static_cast<uint16_t>(ntohs(*(reinterpret_cast<ushort *>(curr))));
        curr += sizeof(uint16_t);
        packetType_ = static_cast<t_PacketType>(ntohs(*(reinterpret_cast<ushort *>(curr))));
        readLength = sizeof(uint16_t) + sizeof(uint16_t) + sizeof(short);
    }

    linkBuffer->addReadBytes(readLength);


#ifdef _VERBOSE
    string typeString;
    packetTypeAsString(packetType_, typeString);

    Log::Debug("Received Alpine packet type: '"s + typeString + "'.");
#endif

    if (parent_) {
        // We have a parent link set, have parent read remaining data.
        //
#ifdef _VERBOSE
        Log::Debug("Invoking parent link interface readData.");
#endif

        // If parent is any of the higher layer Alpine Packet types,
        // update packet type before invoking readData ().
        //
        bool parentCheckDone = false;

        if (!parentCheckDone) {
            AlpineQueryPacket * queryPacket;
            queryPacket = dynamic_cast<AlpineQueryPacket *>(parent_);

            if (queryPacket) {
                queryPacket->setPacketType(packetType_);
                parentCheckDone = true;
            }
        }

        if (!parentCheckDone) {
            AlpinePeerPacket * peerPacket;
            peerPacket = dynamic_cast<AlpinePeerPacket *>(parent_);

            if (peerPacket) {
                peerPacket->setPacketType(packetType_);
                parentCheckDone = true;
            }
        }

        if (!parentCheckDone) {
            AlpineProxyPacket * proxyPacket;
            proxyPacket = dynamic_cast<AlpineProxyPacket *>(parent_);

            if (proxyPacket) {
                proxyPacket->setPacketType(packetType_);
                parentCheckDone = true;
            }
        }


        // Have parent packet link read remainder
        //
        status = parent_->readData(linkBuffer);

        if (!status) {
            return false;
        }
        return false;
    }


    return true;
}


AlpinePacket::t_PacketType
AlpinePacket::getPacketType()
{
#ifdef _VERBOSE
    Log::Debug("AlpinePacket::getPacketType invoked.");
#endif

    return packetType_;
}


bool
AlpinePacket::setPacketType(t_PacketType type)
{
#ifdef _VERBOSE
    string packetTypeString;
    packetTypeAsString(type, packetTypeString);
    Log::Debug("AlpinePacket::setPacketType invoked.  New type: "s + packetTypeString);
#endif

    packetType_ = type;

    return true;
}


bool
AlpinePacket::packetTypeAsString(t_PacketType type, string & typeString)
{
#ifdef _VERBOSE
    Log::Debug("AlpinePacket::packetTypeAsString invoked.");
#endif

    switch (type) {

    case t_PacketType::none:
        typeString = "no type set";
        break;

    case t_PacketType::queryDiscover:
        typeString = "query discover";
        break;

    case t_PacketType::queryOffer:
        typeString = "query offer";
        break;

    case t_PacketType::queryRequest:
        typeString = "query request";
        break;

    case t_PacketType::queryReply:
        typeString = "query reply";
        break;

    case t_PacketType::peerListRequest:
        typeString = "peer list request";
        break;

    case t_PacketType::peerListOffer:
        typeString = "peer list offer";
        break;

    case t_PacketType::peerListGet:
        typeString = "peer list get";
        break;

    case t_PacketType::peerListData:
        typeString = "peer list data";
        break;

    case t_PacketType::proxyRequest:
        typeString = "proxy request";
        break;

    case t_PacketType::proxyAccepted:
        typeString = "proxy accepted";
        break;

    case t_PacketType::proxyHalt:
        typeString = "proxy halt/denied";
        break;

    case t_PacketType::versionHandshake:
        typeString = "version handshake";
        break;

    case t_PacketType::versionAccept:
        typeString = "version accept";
        break;

    case t_PacketType::queryCancellation:
        typeString = "query cancellation";
        break;

    default:
        Log::Debug("Invalid packet type passed to AlpinePacket::packetTypeAsString!");
        return false;
    }
    return true;
}


uint16_t
AlpinePacket::getProtocolVersion()
{
#ifdef _VERBOSE
    Log::Debug("AlpinePacket::getProtocolVersion invoked.");
#endif

    return protocolVersion_;
}
