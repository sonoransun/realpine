/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <DataBuffer.h>
#include <DtcpConnPacket.h>
#include <DtcpPacket.h>
#include <Log.h>
#include <NetUtils.h>
#include <StringUtils.h>


const ulong DtcpPacket::magicCookie_s = (68 << 24) + (84 << 16) + (67 << 8) + 80;  // DTCP
const ushort DtcpPacket::version_s = 0x0001;


DtcpPacket::DtcpPacket()
{
#ifdef _VERBOSE
    Log::Debug("DtcpPacket constructor invoked.");
#endif

    packetType_ = t_PacketType::none;
    parent_ = nullptr;
    peerIpAddress_ = 0;
    peerPort_ = 0;
}


DtcpPacket::DtcpPacket(const DtcpPacket & copy)
{
#ifdef _VERBOSE
    Log::Debug("DtcpPacket copy constructor invoked.");
#endif

    packetType_ = copy.packetType_;
    parent_ = copy.parent_;

    peerIpAddress_ = copy.peerIpAddress_;
    peerPort_ = copy.peerPort_;
}


DtcpPacket::~DtcpPacket()
{
#ifdef _VERBOSE
    Log::Debug("DtcpPacket destructor invoked.");
#endif
}


DtcpPacket &
DtcpPacket::operator=(const DtcpPacket & copy)
{
#ifdef _VERBOSE
    Log::Debug("DtcpPacket::operator = invoked.");
#endif

    if (&copy == this) {
        return *this;
    }

    packetType_ = copy.packetType_;
    parent_ = copy.parent_;

    peerIpAddress_ = copy.peerIpAddress_;
    peerPort_ = copy.peerPort_;

    return *this;
}


bool
DtcpPacket::setParent(StackLinkInterface * parent)
{
#ifdef _VERBOSE
    Log::Debug("DtcpPacket::setParent invoked.");
#endif

    parent_ = parent;

    return true;
}


void
DtcpPacket::unsetParent()
{
#ifdef _VERBOSE
    Log::Debug("DtcpPacket::unsetParent invoked.");
#endif

    parent_ = nullptr;
}


bool
DtcpPacket::writeData(DataBuffer * linkBuffer)
{
#ifdef _VERBOSE
    Log::Debug("DtcpPacket::writeData invoked.");
#endif

    ////
    //
    // Write data...
    //
    // ulong (4 bytes)  -- Magic Cookie
    // short (2 bytes)  -- Protocol Version
    // short (2 bytes)  -- Packet Type
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
        // If parent is a DtcpConnPacket, update our
        // destination IP and Port, and packet type.
        //
        DtcpConnPacket * connPacket;
        connPacket = dynamic_cast<DtcpConnPacket *>(parent_);

        if (connPacket) {
            connPacket->getPeerLocation(peerIpAddress_, peerPort_);

            packetType_ = connPacket->getPacketType();
        }
    }


    writeLength = sizeof(long) + sizeof(short) + sizeof(short);
    if (bufferSize < writeLength) {
        return false;
    }
    curr = buffer;
    *(reinterpret_cast<ulong *>(curr)) = htonl(magicCookie_s);
    curr += sizeof(long);
    *(reinterpret_cast<ushort *>(curr)) = htons(version_s);
    curr += sizeof(short);
    *(reinterpret_cast<ushort *>(curr)) = htons(static_cast<ushort>(packetType_));

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
DtcpPacket::readData(DataBuffer * linkBuffer)
{
#ifdef _VERBOSE
    Log::Debug("DtcpPacket::readData invoked.");
#endif


    ////
    //
    // Read data...
    //
    // ulong (4 bytes)  -- Magic Cookie
    // short (2 bytes)  -- Protocol Version
    // short (2 bytes)  -- Packet Type
    //
    bool status;
    byte * buffer;
    byte * curr;
    uint bufferSize;
    uint readLength;

    ulong readMagicCookie;
    ushort readVersion;

    status = linkBuffer->getReadBuffer(buffer, bufferSize);

    if (!status) {
#ifdef _VERBOSE
        Log::Debug("STOP: getReadBuffer failed in DtcpPacket::readData.");
#endif
        // nothing left to read?
        return false;
    }
    readLength = sizeof(long) + sizeof(short) + sizeof(short);
    if (bufferSize < readLength) {
#ifdef _VERBOSE
        Log::Debug("STOP: Packet size too small in DtcpPacket::readData.");
#endif
        return false;
    }
    curr = buffer;
    readMagicCookie = ntohl(*(reinterpret_cast<ulong *>(curr)));

    if (readMagicCookie != magicCookie_s) {
#ifdef _VERBOSE
        Log::Debug("STOP: Magic DTCP cookie does not match in DtcpPacket::readData.");
#endif
        // Invalid data...
        return false;
    }
    curr += sizeof(long);
    readVersion = ntohs(*(reinterpret_cast<ushort *>(curr)));

    if (readVersion != version_s) {
#ifdef _VERBOSE
        Log::Debug("STOP: DTCP protocol version does not match in DtcpPacket::readData.");
#endif
        // Invalid data...
        return false;
    }
    curr += sizeof(short);
    packetType_ = static_cast<t_PacketType>(ntohs(*(reinterpret_cast<ushort *>(curr))));

    linkBuffer->addReadBytes(readLength);


#ifdef _VERBOSE
    string typeString;
    packetTypeAsString(packetType_, typeString);

    Log::Debug("Received packet type: '"s + typeString + "'.");
#endif

    if (parent_) {
        // We have a parent link set, have parent read data...
        //
#ifdef _VERBOSE
        Log::Debug("Invoking parent link interface readData.");
#endif

        // If parent is a DtcpConnPacket, update conn packet
        // destination IP and Port, and packet type.
        //
        DtcpConnPacket * connPacket;
        connPacket = dynamic_cast<DtcpConnPacket *>(parent_);

        if (connPacket) {
            connPacket->setPeerLocation(peerIpAddress_, peerPort_);
            connPacket->setPacketType(packetType_);
        }

        status = parent_->readData(linkBuffer);

        if (!status) {
            return false;
        }
        return false;
    }


    return true;
}


DtcpPacket::t_PacketType
DtcpPacket::getPacketType()
{
#ifdef _VERBOSE
    Log::Debug("DtcpPacket::getPacketType invoked.");
#endif

    return packetType_;
}


bool
DtcpPacket::setPacketType(t_PacketType type)
{
#ifdef _VERBOSE
    string typeString;
    packetTypeAsString(type, typeString);

    Log::Debug("DtcpPacket::setPacketType invoked."s + "\nType: "s + typeString);
#endif

    packetType_ = type;

    return true;
}


bool
DtcpPacket::packetTypeAsString(t_PacketType type, string & typeString)
{
#ifdef _VERBOSE
    Log::Debug("DtcpPacket::packetTypeAsString invoked.");
#endif

    switch (type) {

    case t_PacketType::none:
        typeString = "no type set";
        break;

    case t_PacketType::connRequest:
        typeString = "connection request";
        break;

    case t_PacketType::connOffer:
        typeString = "connection offer";
        break;

    case t_PacketType::connAccept:
        typeString = "connection accept";
        break;

    case t_PacketType::connSuspend:
        typeString = "connection suspend";
        break;

    case t_PacketType::connResume:
        typeString = "connection resume";
        break;

    case t_PacketType::connData:
        typeString = "connection data";
        break;

    case t_PacketType::connReliableData:
        typeString = "reliable connection data";
        break;

    case t_PacketType::connDataAck:
        typeString = "reliable data ack";
        break;

    case t_PacketType::connClose:
        typeString = "connection close";
        break;

    case t_PacketType::poll:
        typeString = "poll request";
        break;

    case t_PacketType::ack:
        typeString = "acknowledgement";
        break;

    case t_PacketType::error:
        typeString = "error";
        break;

    case t_PacketType::txnData:
        typeString = "transaction data";
        break;

    case t_PacketType::natDiscover:
        typeString = "NAT Discover";
        break;

    case t_PacketType::natSource:
        typeString = "NAT Source";
        break;


    default:
        return false;
    }
    return true;
}


bool
DtcpPacket::getPeerLocation(ulong & ipAddress, ushort & port)
{
#ifdef _VERBOSE
    Log::Debug("DtcpPacket::getPeerLocation invoked.");
#endif

    if (!peerIpAddress_) {
        return false;
    }
    ipAddress = peerIpAddress_;
    port = peerPort_;

    return true;
}


bool
DtcpPacket::setPeerLocation(ulong ipAddress, ushort port)
{
#ifdef _VERBOSE
    Log::Debug("DtcpPacket::setPeerLocation invoked.");
#endif

    peerIpAddress_ = ipAddress;
    peerPort_ = port;

    return true;
}
