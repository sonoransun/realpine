/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <DtcpConnPacket.h>
//#include <DtcpTxnPacket.h>
#include <DataBuffer.h>
#include <Log.h>
#include <NetUtils.h>
#include <StringUtils.h>


DtcpConnPacket::DtcpConnPacket()
{
#ifdef _VERBOSE
    Log::Debug("DtcpConnPacket constructor invoked.");
#endif

    peerId_ = 0;
    myId_ = 0;
    peerIpAddress_ = 0;
    peerPort_ = 0;
    parent_ = nullptr;
    sequenceNum_ = 0;
}


DtcpConnPacket::DtcpConnPacket(StackLinkInterface * parent)
{
#ifdef _VERBOSE
    Log::Debug("DtcpConnPacket stack link interface constructor invoked.");
#endif

    peerId_ = 0;
    myId_ = 0;
    peerIpAddress_ = 0;
    peerPort_ = 0;
    parent_ = parent;
}


DtcpConnPacket::DtcpConnPacket(const DtcpConnPacket & copy)
{
#ifdef _VERBOSE
    Log::Debug("DtcpConnPacket copy constructor invoked.");
#endif

    peerId_ = copy.peerId_;
    myId_ = copy.myId_;
    peerIpAddress_ = copy.peerIpAddress_;
    peerPort_ = copy.peerPort_;
    parent_ = copy.parent_;
}


DtcpConnPacket::~DtcpConnPacket()
{
#ifdef _VERBOSE
    Log::Debug("DtcpConnPacket destructor invoked.");
#endif
}


DtcpConnPacket &
DtcpConnPacket::operator=(const DtcpConnPacket & copy)
{
#ifdef _VERBOSE
    Log::Debug("DtcpConnPacket::operator = invoked.");
#endif

    if (&copy == this) {
        return *this;
    }

    peerId_ = copy.peerId_;
    myId_ = copy.myId_;
    peerIpAddress_ = copy.peerIpAddress_;
    peerPort_ = copy.peerPort_;
    parent_ = copy.parent_;

    return *this;
}


DtcpPacket::t_PacketType
DtcpConnPacket::getPacketType()
{
#ifdef _VERBOSE
    Log::Debug("DtcpConnPacket::getPacketType invoked.");
#endif

    return packetType_;
}


bool
DtcpConnPacket::setPacketType(DtcpPacket::t_PacketType type)
{
#ifdef _VERBOSE
    Log::Debug("DtcpConnPacket::setPacketType invoked.");
#endif

    packetType_ = type;

    return true;
}


bool
DtcpConnPacket::setPeerId(ulong peerId)
{
#ifdef _VERBOSE
    Log::Debug("DtcpConnPacket::setPeerId invoked.");
#endif

    peerId_ = peerId;

    return true;
}


bool
DtcpConnPacket::getPeerId(ulong & peerId)
{
#ifdef _VERBOSE
    Log::Debug("DtcpConnPacket::getPeerId invoked.");
#endif

    peerId = peerId_;

    return true;
}


bool
DtcpConnPacket::setMyId(ulong myId)
{
#ifdef _VERBOSE
    Log::Debug("DtcpConnPacket::setMyId invoked.");
#endif

    myId_ = myId;

    return true;
}


bool
DtcpConnPacket::getMyId(ulong & myId)
{
#ifdef _VERBOSE
    Log::Debug("DtcpConnPacket::getMyId invoked.");
#endif

    myId = myId_;

    return true;
}


bool
DtcpConnPacket::getPeerLocation(ulong & ipAddress, ushort & port)
{
#ifdef _VERBOSE
    Log::Debug("DtcpConnPacket::getPeerLocation invoked.");
#endif

    if (!peerIpAddress_) {
        return false;
    }
    ipAddress = peerIpAddress_;
    port = peerPort_;

    return true;
}


bool
DtcpConnPacket::setPeerLocation(ulong ipAddress, ushort port)
{
#ifdef _VERBOSE
    Log::Debug("DtcpConnPacket::setPeerLocation invoked.");
#endif

    peerIpAddress_ = ipAddress;
    peerPort_ = port;

    return true;
}


bool
DtcpConnPacket::setParent(StackLinkInterface * parent)
{
#ifdef _VERBOSE
    Log::Debug("DtcpConnPacket::setParent invoked.");
#endif

    parent_ = parent;

    return true;
}


bool
DtcpConnPacket::setSequenceNum(ulong sequenceNum)
{
#ifdef _VERBOSE
    Log::Debug("DtcpConnPacket::setParent invoked.");
#endif

    sequenceNum_ = sequenceNum;

    return true;
}


bool
DtcpConnPacket::getSequenceNum(ulong & sequenceNum)
{
#ifdef _VERBOSE
    Log::Debug("DtcpConnPacket::setParent invoked.");
#endif

    sequenceNum = sequenceNum_;

    return true;
}


void
DtcpConnPacket::unsetParent()
{
#ifdef _VERBOSE
    Log::Debug("DtcpConnPacket::unsetParent invoked.");
#endif

    parent_ = nullptr;
}


bool
DtcpConnPacket::writeData(DataBuffer * linkBuffer)
{
#ifdef _VERBOSE
    Log::Debug("DtcpConnPacket::writeData invoked.");
#endif

    ////
    //
    // Write our data...
    //
    bool status;
    byte * buffer;
    byte * curr;
    uint bufferSize;
    uint writeLength = 0;

    status = linkBuffer->getWriteBuffer(buffer, bufferSize);

    if (!status) {
        // no room left to write to?
        return false;
    }
    curr = buffer;


    // If this is a connRequest packet, we send assigned peerID
    //
    if (packetType_ == DtcpPacket::t_PacketType::connRequest) {
        // write peerId
        //
        writeLength += sizeof(long);
        if (bufferSize < writeLength) {
            return false;
        }
        *(reinterpret_cast<ulong *>(curr)) = htonl(peerId_);
        curr += sizeof(long);
    } else {
        // All other packets send myId...
        //
        writeLength += sizeof(long);
        if (bufferSize < writeLength) {
            return false;
        }
        *(reinterpret_cast<ulong *>(curr)) = htonl(myId_);
        curr += sizeof(long);
    }


    // If this is a connOffer, add peerID as well...
    //
    if (packetType_ == DtcpPacket::t_PacketType::connOffer) {
        // append PeerID
        //
        writeLength += sizeof(long);
        if (bufferSize < writeLength) {
            return false;
        }
        *(reinterpret_cast<ulong *>(curr)) = htonl(peerId_);
        curr += sizeof(long);
    }


    // If this is a connResume, add previous IP and PORT
    //
    if (packetType_ == DtcpPacket::t_PacketType::connResume) {
        // append previous IP address and port
        //
        writeLength += sizeof(long) + sizeof(short);
        if (bufferSize < writeLength) {
            return false;
        }
        *(reinterpret_cast<ulong *>(curr)) = htonl(prevIpAddress_);
        curr += sizeof(long);

        *(reinterpret_cast<ushort *>(curr)) = htons(prevPort_);
        curr += sizeof(short);
    }


    // If this is a reliable data packet, add sequence number
    //
    if (packetType_ == DtcpPacket::t_PacketType::connReliableData) {
        // append sequence number
        //
        writeLength += sizeof(long);
        if (bufferSize < writeLength) {
            return false;
        }
        *(reinterpret_cast<ulong *>(curr)) = htonl(sequenceNum_);
        curr += sizeof(long);
    }


    linkBuffer->addWriteBytes(writeLength);


    if (parent_) {
        // We have a parent link set, have parent write data,
        //
        status = parent_->writeData(linkBuffer);

        if (!status) {
            return false;
        }
#if 0  // TEMP!!!!

        // If parent is a DtcpTxnPacket, update our packet type
        //
        DtcpTxnPacket * txnPacket;
        txnPacket = dynamic_cast<DtcpTxnPacket *>(parent_);

        if (txnPacket) {
            DtcpTxnPacket::t_TxnPacketType  packetType;
            packetType = txnPacket->getPacketType ();

            packetType_ = reinterpret_cast<t_ConnPacketType>(packetType);
            return;
        }
#endif
    }


    return true;
}


bool
DtcpConnPacket::readData(DataBuffer * linkBuffer)
{
#ifdef _VERBOSE
    Log::Debug("DtcpConnPacket::readData invoked.");
#endif

    ////
    //
    // Read our data...
    //
    //
    bool status;
    byte * buffer;
    byte * curr;
    uint bufferSize;
    uint readLength = 0;

    status = linkBuffer->getReadBuffer(buffer, bufferSize);

    if (!status) {
        // nothing left to read?
        return false;
    }
    curr = buffer;


    // If this is a connRequest packet, read assigned myID
    //
    if (packetType_ == DtcpPacket::t_PacketType::connRequest) {
        // read myId
        //
        readLength += sizeof(long);
        if (bufferSize < readLength) {
            return false;
        }
        myId_ = ntohl(*(reinterpret_cast<ulong *>(curr)));
        curr += sizeof(long);
    } else {
        // All other packets read peerId...
        //
        readLength += sizeof(long);
        if (bufferSize < readLength) {
            return false;
        }
        peerId_ = ntohl(*(reinterpret_cast<ulong *>(curr)));
        curr += sizeof(long);
    }


    // If this is a connOffer, read myID
    //
    if (packetType_ == DtcpPacket::t_PacketType::connOffer) {
        // read myID
        //
        readLength += sizeof(long);
        if (bufferSize < readLength) {
            return false;
        }
        myId_ = ntohl(*(reinterpret_cast<ulong *>(curr)));
        curr += sizeof(long);
    }


    // If this is a connResume, read previous IP and PORT
    //
    if (packetType_ == DtcpPacket::t_PacketType::connResume) {
        // read previous IP address and port
        //
        readLength += sizeof(long) + sizeof(short);
        if (bufferSize < readLength) {
            return false;
        }
        prevIpAddress_ = ntohl(*(reinterpret_cast<ulong *>(curr)));
        curr += sizeof(long);

        prevPort_ = ntohs(*(reinterpret_cast<ushort *>(curr)));
        curr += sizeof(short);
    }


    // If this is a reliable data packet, read sequence number
    //
    if (packetType_ == DtcpPacket::t_PacketType::connReliableData) {
        // append sequence number
        //
        readLength += sizeof(long);
        if (bufferSize < readLength) {
            return false;
        }
        sequenceNum_ = ntohl(*(reinterpret_cast<ulong *>(curr)));
        curr += sizeof(long);
    }


    linkBuffer->addReadBytes(readLength);


    if (parent_) {
        // We have a parent link set, have parent read data...
        //
#ifdef _VERBOSE
        Log::Debug("Invoking parent link interface readData.");
#endif

#if 0
        // TEMP !!!
        // If parent is a DtcpTxnPacket, update our packet type.
        DtcpTxnPacket * txnPacket;
        txnPacket = dynamic_cast<DtcpTxnPacket *>(parent_);

        if (txnPacket) {
            DtcpTxnPacket::t_TxnPacketType  packetType;
            packetType = reinterpret_cast<DtcpTxnPacket::t_TxnPacketType>(packetType_);
            txnPacket->setPacketType (packetType);
        }
#endif

        status = parent_->readData(linkBuffer);

        if (!status) {
            return false;
        }
        return false;
    }


    return true;
}
