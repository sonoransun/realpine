/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AlpinePacket.h>
#include <AlpineQueryPacket.h>
#include <AlpinePeerPacket.h>
#include <AlpineProxyPacket.h>
#include <DataBuffer.h>
#include <Log.h>
#include <StringUtils.h>
#include <NetUtils.h>



AlpinePacket::AlpinePacket ()
{
#ifdef _VERBOSE
    Log::Debug ("AlpinePacket constructor invoked.");
#endif

    parent_     = nullptr;
    packetType_ = t_PacketType::none;
}



AlpinePacket::AlpinePacket (const AlpinePacket & copy)
{
#ifdef _VERBOSE
    Log::Debug ("AlpinePacket copy constructor invoked.");
#endif

    parent_     = copy.parent_;
    packetType_ = copy.packetType_;
}



AlpinePacket::~AlpinePacket ()
{
#ifdef _VERBOSE
    Log::Debug ("AlpinePacket destructor invoked.");
#endif

    // Nothing to do here
}



AlpinePacket & 
AlpinePacket::operator = (const AlpinePacket & copy)
{
#ifdef _VERBOSE
    Log::Debug ("AlpinePacket assignment invoked.");
#endif

    if (&copy == this) {
        return *this;
    }

    parent_     = copy.parent_;
    packetType_ = copy.packetType_;


    return *this;
}



bool  
AlpinePacket::setParent (StackLinkInterface *  parent)
{
#ifdef _VERBOSE
    Log::Debug ("AlpinePacket::setParent invoked.");
#endif

    parent_ = parent;


    return true;
}



void  
AlpinePacket::unsetParent ()
{
#ifdef _VERBOSE
    Log::Debug ("AlpinePacket::unsetParent invoked.");
#endif

    parent_ = nullptr;
}



bool
AlpinePacket::writeData (DataBuffer * linkBuffer)
{
#ifdef _VERBOSE
    Log::Debug ("AlpinePacket::writeData invoked.");
#endif

    ////
    //
    // Write data; all we really write at this layer is the packet type
    //
    // ushort (2 bytes)  -- Packet Type
    //
    bool   status;
    byte * buffer;
    byte * curr;
    uint   bufferSize;
    uint   writeLength;

    status = linkBuffer->getWriteBuffer (buffer, bufferSize);

    if (!status) {
        // no room left to write to?
        return false;
    }
    if (parent_) {
        // If parent is any of the higher layer Alpine Packet types, update our packet type.
        //
        bool  finished = false;

        if (!finished) {
            AlpineQueryPacket * queryPacket;
            queryPacket = dynamic_cast<AlpineQueryPacket *>(parent_);

            if (queryPacket) {
                packetType_ = queryPacket->getPacketType ();
                finished = true;
            }
        }

        if (!finished) {
            AlpinePeerPacket *  peerPacket;
            peerPacket = dynamic_cast<AlpinePeerPacket *>(parent_);

            if (peerPacket) {
                packetType_ = peerPacket->getPacketType ();
                finished = true;
            }
        }

        if (!finished) {
            AlpineProxyPacket *  proxyPacket;
            proxyPacket = dynamic_cast<AlpineProxyPacket *>(parent_);

            if (proxyPacket) {
                packetType_ = proxyPacket->getPacketType ();
                finished = true;
            }
        }
    }


    writeLength = sizeof(short);
    if (bufferSize < writeLength) {
        return false;
    }
    curr = buffer;
    *(reinterpret_cast<ushort *>(curr)) = htons(static_cast<ushort>(packetType_));
    curr += sizeof(short);

    linkBuffer->addWriteBytes (writeLength);


    if (parent_) {
        // We have a parent link set, have parent write data,
        //
        status = parent_->writeData (linkBuffer);

        if (!status) {
            return false;

        }
        return false;
    }


    return true;
}



bool  
AlpinePacket::readData (DataBuffer * linkBuffer)
{
#ifdef _VERBOSE
    Log::Debug ("AlpinePacket::readData invoked.");
#endif

    ////
    //
    // Read data
    //
    // short (2 bytes)  -- Packet Type
    //
    bool   status;
    byte * buffer;
    byte * curr;
    uint   bufferSize;
    uint   readLength;

    status = linkBuffer->getReadBuffer (buffer, bufferSize);

    if (!status) {
#ifdef _VERBOSE
        Log::Debug ("getReadBuffer failed in AlpinePacket::readData.");
#endif
        return false;
    }
    readLength = sizeof(short);

    if (bufferSize < readLength) {
#ifdef _VERBOSE
        Log::Debug ("Packet size too small in AlpinePacket::readData.");
#endif
        return false;
    }
    curr = buffer;
    packetType_ = static_cast<t_PacketType>(ntohs(*(reinterpret_cast<ushort *>(curr))));

    linkBuffer->addReadBytes (readLength);


#ifdef _VERBOSE
    string typeString;
    packetTypeAsString (packetType_, typeString);

    Log::Debug ("Received Alpine packet type: '"s + typeString +
                "'.");
#endif

    if (parent_) {
        // We have a parent link set, have parent read remaining data.
        //
#ifdef _VERBOSE
        Log::Debug ("Invoking parent link interface readData.");
#endif

        // If parent is any of the higher layer Alpine Packet types,
        // update packet type before invoking readData ().
        //
        bool  parentCheckDone = false;

        if (!parentCheckDone) {
            AlpineQueryPacket * queryPacket;
            queryPacket = dynamic_cast<AlpineQueryPacket *>(parent_);

            if (queryPacket) {
                queryPacket->setPacketType (packetType_);
                parentCheckDone = true;
            }
        }

        if (!parentCheckDone) {
            AlpinePeerPacket *  peerPacket;
            peerPacket = dynamic_cast<AlpinePeerPacket *>(parent_);

            if (peerPacket) {
                peerPacket->setPacketType (packetType_);
                parentCheckDone = true;
            }
        }

        if (!parentCheckDone) {
            AlpineProxyPacket *  proxyPacket;
            proxyPacket = dynamic_cast<AlpineProxyPacket *>(parent_);

            if (proxyPacket) {
                proxyPacket->setPacketType (packetType_);
                parentCheckDone = true;
            }
        }


        // Have parent packet link read remainder
        //
        status = parent_->readData (linkBuffer);

        if (!status) {
            return false;

        }
        return false;
    }


    return true;
}



AlpinePacket::t_PacketType  
AlpinePacket::getPacketType ()
{
#ifdef _VERBOSE
    Log::Debug ("AlpinePacket::getPacketType invoked.");
#endif

    return packetType_;
}



bool 
AlpinePacket::setPacketType (t_PacketType  type)
{
#ifdef _VERBOSE
    string  packetTypeString;
    packetTypeAsString (type, packetTypeString);
    Log::Debug ("AlpinePacket::setPacketType invoked.  New type: "s + packetTypeString);
#endif

    packetType_ = type;

    return true;
}



bool 
AlpinePacket::packetTypeAsString (t_PacketType  type,
                                  string &      typeString)
{
#ifdef _VERBOSE
    Log::Debug ("AlpinePacket::packetTypeAsString invoked.");
#endif

    switch (type) {

        case t_PacketType::none :
          typeString = "no type set";
          break;

        case t_PacketType::queryDiscover :
          typeString = "query discover";
          break;

        case t_PacketType::queryOffer :
          typeString = "query offer";
          break;

        case t_PacketType::queryRequest :
          typeString = "query request";
          break;

        case t_PacketType::queryReply :
          typeString = "query reply";
          break;

        case t_PacketType::peerListRequest :
          typeString = "peer list request";
          break;

        case t_PacketType::peerListOffer :
          typeString = "peer list offer";
          break;

        case t_PacketType::peerListGet :
          typeString = "peer list get";
          break;

        case t_PacketType::peerListData :
          typeString = "peer list data";
          break;

        case t_PacketType::proxyRequest :
          typeString = "proxy request";
          break;

        case t_PacketType::proxyAccepted :
          typeString = "proxy accepted";
          break;

        case t_PacketType::proxyHalt :
          typeString = "proxy halt/denied";
          break;

      default:
        Log::Debug ("Invalid packet type passed to AlpinePacket::packetTypeAsString!");
        return false;
    }
    return true;
}





