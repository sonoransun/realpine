/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AlpinePeerPacket.h>
#include <AlpinePacket.h>
#include <AlpinePeerOptionData.h>
#include <AlpineExtensionIndex.h>
#include <DataBuffer.h>
#include <Log.h>
#include <StringUtils.h>
#include <NetUtils.h>



AlpinePeerPacket::AlpinePeerPacket ()
{
#ifdef _VERBOSE
    Log::Debug ("AlpinePeerPacket constructor invoked.");
#endif

    parent_              = nullptr;
    packetType_          = AlpinePacket::t_PacketType::none;
    optionId_            = 0;
    optionData_          = nullptr;
    numPeersAvailable_   = 0;
    offerId_             = 0;
    offset_              = 0;
    setSize_             = 0;
    peerLocationList_    = nullptr;
}



AlpinePeerPacket::AlpinePeerPacket (StackLinkInterface * parent)
{
#ifdef _VERBOSE
    Log::Debug ("AlpinePeerPacket parent constructor invoked.");
#endif

    parent_              = parent;
    packetType_          = AlpinePacket::t_PacketType::none;
    optionId_            = 0;
    optionData_          = nullptr;
    numPeersAvailable_   = 0;
    offerId_             = 0;
    offset_              = 0;
    setSize_             = 0;
    peerLocationList_    = nullptr;
}



AlpinePeerPacket::AlpinePeerPacket (const AlpinePeerPacket & copy)
{
#ifdef _VERBOSE
    Log::Debug ("AlpinePeerPacket copy constructor invoked.");
#endif

    parent_              = copy.parent_;
    packetType_          = copy.packetType_;
    optionId_            = copy.optionId_;
    numPeersAvailable_   = copy.numPeersAvailable_;
    offerId_             = copy.offerId_;
    offset_              = copy.offset_;
    setSize_             = copy.setSize_;

    if (copy.peerLocationList_) {
        peerLocationList_ = new t_PeerLocationList (*(copy.peerLocationList_));
    }
    else {
        peerLocationList_ = nullptr;
    }

    if (copy.optionData_) {
        optionData_ = copy.optionData_->duplicate ();
    }
    else {
        optionData_ = nullptr;
    }
}



AlpinePeerPacket::~AlpinePeerPacket ()
{
#ifdef _VERBOSE
    Log::Debug ("AlpinePeerPacket destructor invoked.");
#endif

    delete peerLocationList_;

    delete optionData_;
}



AlpinePeerPacket & 
AlpinePeerPacket::operator = (const AlpinePeerPacket & copy)
{
#ifdef _VERBOSE
    Log::Debug ("AlpinePeerPacket assignment invoked.");
#endif

    if (&copy == this) {
        return *this;
    }

    if (peerLocationList_) {
        delete peerLocationList_;
        peerLocationList_ = nullptr;
    }

    if (optionData_) {
        delete optionData_;
        optionData_ = nullptr;
    }

    parent_              = copy.parent_;
    packetType_          = copy.packetType_;
    optionId_            = copy.optionId_;
    numPeersAvailable_   = copy.numPeersAvailable_;
    offerId_             = copy.offerId_;
    offset_              = copy.offset_;
    setSize_             = copy.setSize_;

    if (copy.peerLocationList_) {
        peerLocationList_ = new t_PeerLocationList (*(copy.peerLocationList_));
    }

    if (copy.optionData_) {
        optionData_ = copy.optionData_->duplicate ();
    }


    return *this;
}



AlpinePacket::t_PacketType  
AlpinePeerPacket::getPacketType ()
{
#ifdef _VERBOSE
    Log::Debug ("AlpinePeerPacket::getPacketType invoked.");
#endif

    return packetType_;
}



bool 
AlpinePeerPacket::setPacketType (AlpinePacket::t_PacketType  type)
{
#ifdef _VERBOSE
    string packetTypeString;
    AlpinePacket::packetTypeAsString (type, packetTypeString);
    Log::Debug ("AlpinePeerPacket::setPacketType invoked.  New type: "s + packetTypeString);
#endif

    if ( (type != AlpinePacket::t_PacketType::peerListRequest ) &&
         (type != AlpinePacket::t_PacketType::peerListOffer ) &&
         (type != AlpinePacket::t_PacketType::peerListGet ) &&
         (type != AlpinePacket::t_PacketType::peerListData ) ) {

        Log::Error ("Invalid packet type passed in call to AlpinePeerPacket::setPacketType!");
        return false;
    }
    packetType_ =  type;


    return true;
}



bool  
AlpinePeerPacket::setOptionId (ulong  optionId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpinePeerPacket::setOptionId invoked.  Option ID: "s +
                std::to_string (optionId));
#endif

    if (optionId != 0) {
        Log::Error ("Attempt to set extended option ID in call to "
                             "AlpinePeerPacket::setOptionId!  Use setOptionData for extended options.");
        return false;
    }
    optionId_ = optionId;

    return true;
}



bool  
AlpinePeerPacket::getOptionId (ulong &  optionId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpinePeerPacket::getOptionId invoked.");
#endif

    optionId = optionId_;

    return true;
}



bool  
AlpinePeerPacket::setOptionData (AlpinePeerOptionData *  optionData)
{
#ifdef _VERBOSE
    Log::Debug ("AlpinePeerPacket::setOptionData invoked.");
#endif

    if (optionData_) {
        delete optionData_;
        optionData_ = nullptr;
    }

    optionId_   = optionData->getOptionId ();
    optionData_ = optionData->duplicate ();  


    return true;
}



bool  
AlpinePeerPacket::getOptionData (AlpinePeerOptionData *&  optionData)
{
#ifdef _VERBOSE
    Log::Debug ("AlpinePeerPacket::getOptionData invoked.");
#endif

    if ( (optionId_ == 0) || (!optionData_) ) {
        Log::Error ("Attempt to get option data when no extension set in call to "
                             "AlpinePeerPacket::getOptionData!");
        return false;
    }
    optionData = optionData_->duplicate ();


    return true;
}



bool  
AlpinePeerPacket::setPeersAvailable (ulong  numAvailable)
{
#ifdef _VERBOSE
    Log::Debug ("AlpinePeerPacket::setPeersAvailable invoked.  NumAvailable: "s +
                std::to_string (numAvailable));
#endif

    numPeersAvailable_ = numAvailable;

    return true;
}



bool  
AlpinePeerPacket::getPeersAvailable (ulong &  numAvailable)
{
#ifdef _VERBOSE
    Log::Debug ("AlpinePeerPacket::getPeersAvailable invoked.");
#endif

    numAvailable = numPeersAvailable_;

    return true;
}



bool  
AlpinePeerPacket::setOfferId (ulong  offerId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpinePeerPacket::setOfferId invoked.  Offer ID: "s +
                std::to_string (offerId));
#endif

    offerId_ = offerId;

    return true;
}



bool  
AlpinePeerPacket::getOfferId (ulong &  offerId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpinePeerPacket::getOfferId invoked.");
#endif

    offerId = offerId_;

    return true;
}



bool  
AlpinePeerPacket::setOffset (ulong  offset)
{
#ifdef _VERBOSE
    Log::Debug ("AlpinePeerPacket::setOffset invoked.  Offset: "s +
                std::to_string (offset));
#endif

    offset_ = offset;

    return true;
}



bool  
AlpinePeerPacket::getOffset (ulong &  offset)
{
#ifdef _VERBOSE
    Log::Debug ("AlpinePeerPacket::getOffset invoked.");
#endif

    offset = offset_;

    return true;
}



bool  
AlpinePeerPacket::setReplySetSize (ushort  setSize)
{
#ifdef _VERBOSE
    Log::Debug ("AlpinePeerPacket::setReplySetSize invoked.  Set Size: "s +
                std::to_string (setSize));
#endif

    setSize_ = setSize;

    return true;
}



bool  
AlpinePeerPacket::getReplySetSize (ushort &  setSize)
{
#ifdef _VERBOSE
    Log::Debug ("AlpinePeerPacket::getReplySetSize invoked.");
#endif

    setSize = setSize_;

    return true;
}



bool  
AlpinePeerPacket::setPeerLocationList (t_PeerLocationList &  locationList)
{
#ifdef _VERBOSE
    Log::Debug ("AlpinePeerPacket::setPeerLocationList invoked.");
#endif

    if (peerLocationList_) {
        delete peerLocationList_;
    }

    peerLocationList_ = new t_PeerLocationList (locationList);

    return true;
}



bool  
AlpinePeerPacket::getPeerLocationList (t_PeerLocationList &  locationList)
{
#ifdef _VERBOSE
    Log::Debug ("AlpinePeerPacket::getPeerLocationList invoked.");
#endif

    if (!peerLocationList_) {
        return false;
    }
    locationList = *peerLocationList_;

    return true;
}



bool  
AlpinePeerPacket::setParent (StackLinkInterface *  parent)
{
#ifdef _VERBOSE
    Log::Debug ("AlpinePeerPacket::setParent invoked.");
#endif

    parent_ = parent;

    return true;
}



void  
AlpinePeerPacket::unsetParent ()
{
#ifdef _VERBOSE
    Log::Debug ("AlpinePeerPacket::unsetParent invoked.");
#endif

    parent_ = nullptr;
}



bool
AlpinePeerPacket::writeData (DataBuffer * linkBuffer)
{
#ifdef _VERBOSE
    Log::Debug ("AlpinePeerPacket::writeData invoked.");
#endif

    ////
    //
    // Write data
    //
    // Various members
    //
    bool   status;
    byte * buffer;
    byte * curr;
    uint   bufferSize;
    uint   writeLength = 0;

    status = linkBuffer->getWriteBuffer (buffer, bufferSize);

    if (!status) {
        // no room left to write to?
        Log::Debug ("getWriteBuffer failed in AlpinePeerPacket::writeData.");

        return false;
    }
    if (packetType_ == AlpinePacket::t_PacketType::peerListRequest) {
        // Peer List Request Message
        //
        // 4b - Option ID
        // 0-Nb - Option Data
        //
        writeLength = sizeof(long);

        if ( (optionId_) && (optionData_) ) {
            writeLength += optionData_->getOptionDataLength ();
        }
    }
    else if (packetType_ == AlpinePacket::t_PacketType::peerListOffer) {
        // Peer List Offer Message
        //
        // 4b - Num Peers Available
        // 4b - Offer ID
        // 4b - Option ID
        // 0-Nb - Option Data
        //
        writeLength = sizeof(long) + sizeof(long) + sizeof(long);

        if ( (optionId_) && (optionData_) ) {
            writeLength += optionData_->getOptionDataLength ();
        }
    }
    else if (packetType_ == AlpinePacket::t_PacketType::peerListGet) {
        // Peer List Get Message
        //
        // 4b - Offer ID
        // 4b - Offset
        // 2b - Desired reply set size
        //
        writeLength = sizeof(long) + sizeof(long) + sizeof(short);
    }
    else if (packetType_ == AlpinePacket::t_PacketType::peerListData) {
        // Peer List Data Message
        //
        // 4b - Offer ID
        // 2b - Reply set size
        // Nb - List of t_PeerLocation data.
        //

        // See if we have a location list before continuing...
        //
        if (!peerLocationList_) {
            Log::Error ("Cannot send peerListData with no peer location list set in "
                                 "AlpinePeerPacket::writeData!");

            return false;
        }
        writeLength = calculatePeerListSize ();
        writeLength += sizeof(long) + sizeof(short);
    }


    // Make sure we have enough space to write the data.
    //
    if (bufferSize < writeLength) {
        Log::Debug ("Insufficient space in write buffer for AlpinePeerPacket::writeData!");
        return false;
    }
    // Write data in packet buffer
    //
    curr = buffer;
    writeLength = 0;

    if (packetType_ == AlpinePacket::t_PacketType::peerListRequest) {
        *(reinterpret_cast<ulong *>(curr)) = htonl(optionId_);
        curr        += sizeof(long);
        writeLength += sizeof(long);

        // If extended query options given, write them into the buffer.
        //
        linkBuffer->addWriteBytes (writeLength);

        if ( (optionId_) && (optionData_) ) {
            status = optionData_->writeData (linkBuffer);

            if (!status) {
                Log::Error ("writeData failed for Option Data in call to "
                                     "AlpinePeerPacket::writeData! (peerListRequest packet)");
                return false;
            }
        }
    }
    else if (packetType_ == AlpinePacket::t_PacketType::peerListOffer) {
        *(reinterpret_cast<ulong *>(curr)) = htonl(numPeersAvailable_);
        curr        += sizeof(long);
        writeLength += sizeof(long);

        *(reinterpret_cast<ulong *>(curr)) = htonl(offerId_);
        curr        += sizeof(long);
        writeLength += sizeof(long);

        *(reinterpret_cast<ulong *>(curr)) = htonl(optionId_);
        curr        += sizeof(long);
        writeLength += sizeof(long);


        // If extended query options given, write them into the buffer.
        //
        linkBuffer->addWriteBytes (writeLength);

        if ( (optionId_) && (optionData_) ) {
            status = optionData_->writeData (linkBuffer);

            if (!status) {
                Log::Error ("writeData failed for Option Data in call to "
                                     "AlpinePeerPacket::writeData! (peerListOffer packet)");
                return false;
            }
        }
    }
    else if (packetType_ == AlpinePacket::t_PacketType::peerListGet) {
        *(reinterpret_cast<ulong *>(curr)) = htonl(offerId_);
        curr        += sizeof(long);
        writeLength += sizeof(long);

        *(reinterpret_cast<ulong *>(curr)) = htonl(offset_);
        curr        += sizeof(long);
        writeLength += sizeof(long);

        *(reinterpret_cast<ushort *>(curr)) = htons(setSize_);
        curr        += sizeof(short);
        writeLength += sizeof(short);

        linkBuffer->addWriteBytes (writeLength);
    }
    else if (packetType_ == AlpinePacket::t_PacketType::peerListData) {
        *(reinterpret_cast<ulong *>(curr)) = htonl(offerId_);
        curr        += sizeof(long);
        writeLength += sizeof(long);

        *(reinterpret_cast<ushort *>(curr)) = htons(setSize_);
        curr        += sizeof(short);
        writeLength += sizeof(short);

        linkBuffer->addWriteBytes (writeLength);

        status = writePeerListData (linkBuffer);

        if (!status) {
            return false;
        }
    }


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
AlpinePeerPacket::readData (DataBuffer * linkBuffer)
{
#ifdef _VERBOSE
    Log::Debug ("AlpinePeerPacket::readData invoked.");
#endif


    ////
    //
    // Read data
    //
    // Various Members
    //
    bool   status;
    byte * buffer;
    byte * curr;
    uint   bufferSize;
    uint   readLength = 0;

    status = linkBuffer->getReadBuffer (buffer, bufferSize);

    if (!status) {
        Log::Debug ("getReadBuffer failed in AlpineQueryPacket::readData.");

        return false;
    }
    // Verify minum read length before performing much processing
    //
    if (packetType_ == AlpinePacket::t_PacketType::peerListRequest) {
        // Peer List Request Message
        //
        // 4b - Option ID
        //
        readLength = sizeof(long);
    }
    else if (packetType_ == AlpinePacket::t_PacketType::peerListOffer) {
        // Peer List Offer Message
        //
        // 4b - Num Peers Available
        // 4b - Offer ID
        // 4b - Option ID
        //
        readLength = sizeof(long) + sizeof(long) + sizeof(long);
    }
    else if (packetType_ == AlpinePacket::t_PacketType::peerListGet) {
        // Peer List Get Message
        //
        // 4b - Offer ID
        // 4b - Offset
        // 2b - Desired reply set size
        //
        readLength = sizeof(long) + sizeof(long) + sizeof(short);
    }
    else if (packetType_ == AlpinePacket::t_PacketType::peerListData) {
        // Peer List Data Message
        //
        // 4b - Offer ID
        // 2b - Reply set size
        // Nb - List of t_PeerLocation data.
        //
        readLength += sizeof(long) + sizeof(short);
    }

    if (bufferSize < readLength) {
#ifdef _VERBOSE
        Log::Debug ("Packet size too small in AlpinePeerPacket::readData.");
#endif
        return false;
    }
    // Read data from packet buffer
    //
    curr       = buffer;
    readLength = 0;

    if (packetType_ == AlpinePacket::t_PacketType::peerListRequest) {
        optionId_   = static_cast<ulong>(ntohl(*(reinterpret_cast<ulong *>(curr))));
        curr       += sizeof(long);
        readLength += sizeof(long);

        // If extended query options given, read them into the buffer.
        //
        linkBuffer->addReadBytes (readLength);

        if (optionId_) {
            if (optionData_) {
                delete optionData_;
                optionData_ = nullptr;
            }

            status = AlpineExtensionIndex::getPeerOptionExt (optionId_, optionData_);

            if (!status) {
#ifdef _VERBOSE
                Log::Error ("Attempt to get PeerOptionExt(Data) failed for OptionID in call to "
                                     "AlpinePeerPacket::readData! (peerListRequest packet)");
#endif
                return false;
            }
            status = optionData_->readData (linkBuffer);

            if (!status) {
#ifdef _VERBOSE
                Log::Error ("readData failed for Option Data in call to "
                                     "AlpinePeerPacket::readData! (peerListRequest packet)");
#endif
                return false;
            }
        }
    }
    else if (packetType_ == AlpinePacket::t_PacketType::peerListOffer) {
        numPeersAvailable_ = static_cast<ulong>(ntohl(*(reinterpret_cast<ulong *>(curr))));
        curr              += sizeof(long);
        readLength        += sizeof(long);

        offerId_    = static_cast<ulong>(ntohl(*(reinterpret_cast<ulong *>(curr))));
        curr       += sizeof(long);
        readLength += sizeof(long);

        optionId_   = static_cast<ulong>(ntohl(*(reinterpret_cast<ulong *>(curr))));
        curr       += sizeof(long);
        readLength += sizeof(long);

        // If extended query options given, read them into the buffer.
        //
        linkBuffer->addReadBytes (readLength);

        if (optionId_) {
            if (optionData_) {
                delete optionData_;
                optionData_ = nullptr;
            }

            status = AlpineExtensionIndex::getPeerOptionExt (optionId_, optionData_);

            if (!status) {
#ifdef _VERBOSE
                Log::Error ("Attempt to get PeerOptionExt(Data) failed for OptionID in call to "
                                     "AlpinePeerPacket::readData! (peerListOffer packet)");
#endif
                return false;
            }
            status = optionData_->readData (linkBuffer);

            if (!status) {
                Log::Error ("readData failed for Option Data in call to "
                                     "AlpinePeerPacket::readData! (peerListOffer packet)");
                return false;
            }
        }
    }
    else if (packetType_ == AlpinePacket::t_PacketType::peerListGet) {
        offerId_    = static_cast<ulong>(ntohl(*(reinterpret_cast<ulong *>(curr))));
        curr       += sizeof(long);
        readLength += sizeof(long);

        offset_     = static_cast<ulong>(ntohl(*(reinterpret_cast<ulong *>(curr))));
        curr       += sizeof(long);
        readLength += sizeof(long);

        setSize_    = static_cast<short>(ntohs(*(reinterpret_cast<short *>(curr))));
        curr       += sizeof(short);
        readLength += sizeof(short);

        linkBuffer->addReadBytes (readLength);
    }
    else if (packetType_ == AlpinePacket::t_PacketType::peerListData) {
        offerId_    = static_cast<ulong>(ntohl(*(reinterpret_cast<ulong *>(curr))));
        curr       += sizeof(long);
        readLength += sizeof(long);

        setSize_    = static_cast<short>(ntohs(*(reinterpret_cast<short *>(curr))));
        curr       += sizeof(short);
        readLength += sizeof(short);

        linkBuffer->addReadBytes (readLength);

        if (!peerLocationList_) {
            peerLocationList_ = new t_PeerLocationList;
            return false;
        }

        status = readPeerListData (linkBuffer);

        if (!status) {
#ifdef _VERBOSE
            Log::Debug ("Invalid peer location data in peer list data packet in "
                                 "AlpinePeerPacket::readData.");
#endif
            return false;
        }
    }


    return true;
}



ulong
AlpinePeerPacket::calculatePeerListSize ()
{
#ifdef _VERBOSE
    Log::Debug ("AlpinePeerPacket::calculatePeerListSize invoked.");
#endif

    if (!peerLocationList_) {
        return false;
    }
    ulong  listDataSize = 0;

    AlpinePeerLocation *          currLocation;
    AlpinePeerOptionData *        optionData = nullptr;

    for (auto& item : *peerLocationList_) {

        currLocation = &item;

        listDataSize += sizeof(long);  // IP Address
        listDataSize += sizeof(short); // port
        listDataSize += sizeof(long);  // option ID

        if (currLocation->getOptionId () != 0) {
            currLocation->getOptionData (optionData);
            listDataSize += optionData->getOptionDataLength ();
            delete optionData;
        }
    }


    return listDataSize;
}



bool   
AlpinePeerPacket::writePeerListData (DataBuffer *  linkBuffer)
{
#ifdef _VERBOSE
    Log::Debug ("AlpinePeerPacket::writePeerListData invoked.");
#endif

    if ( (!peerLocationList_) || (peerLocationList_->size () == 0) ) {
        Log::Error ("Invalid peer location list passed in call to "
                             "AlpinePeerPacket::writePeerListData!");
        return false;
    }
    // Data Buffer info
    //
    bool   status;
    byte * buffer;
    byte * curr;
    uint   bufferSize;
    uint   writeLength;


    AlpinePeerLocation *  currLocation;
    AlpinePeerOptionData *  optionData;

    for (auto& item : *peerLocationList_) {

        // We must refresh our write buffer data with each iteration as
        // extended option data will modify the write buffer.
        //
        status = linkBuffer->getWriteBuffer (buffer, bufferSize);

        if (!status) {
            Log::Error ("getWriteBuffer failed in call to "
                                 "AlpinePeerPacket::writePeerListData!");
            return false;
        }
        writeLength = 0;
        curr        = buffer;


        // Begin write
        //
        currLocation = &item;

        // Peer IP Address
        //
        *(reinterpret_cast<ulong *>(curr)) = htonl(currLocation->getIpAddress ());
        curr        += sizeof(long);
        writeLength += sizeof(long);

        *(reinterpret_cast<ushort *>(curr)) = htons(currLocation->getPort ());
        curr        += sizeof(short);
        writeLength += sizeof(short);

        *(reinterpret_cast<ulong *>(curr)) = htonl(currLocation->getOptionId ());
        curr        += sizeof(long);
        writeLength += sizeof(long);


        // If we have optional extensions given, update DataBuffer and write extensions
        //
        linkBuffer->addWriteBytes (writeLength);

        if (currLocation->getOptionId ()) {
            currLocation->getOptionData (optionData);

            status = optionData->writeData (linkBuffer);
            delete optionData;

            if (!status) {
                Log::Error ("Attempted write of extended option data failed in call to "
                                     "AlpinePeerPacket::writePeerListData!");
                return false;
            }
        }
    }


    return true;
}



bool
AlpinePeerPacket::readPeerListData (DataBuffer *  linkBuffer)
{
#ifdef _VERBOSE
    Log::Debug ("AlpinePeerPacket::readPeerListData invoked.");
#endif

    if (!peerLocationList_) {
        peerLocationList_ = new t_PeerLocationList;
    }
    else {
        delete peerLocationList_;
        peerLocationList_ = new t_PeerLocationList;
    }


    // Peer Location List Data
    //
    AlpinePeerLocation      currLocation;
    AlpinePeerOptionData *  optionData;
    ulong  longValue;
    ushort shortValue;
    ulong  optionId;


    // Buffer data
    //
    bool   status;
    byte * buffer;
    byte * curr;
    uint   bufferSize;
    uint   readLength;

    bool  finished = false;
    while (!finished) {

        // We must refresh our write buffer data with each iteration as
        // extended option data will modify the write buffer.
        //
        status = linkBuffer->getReadBuffer (buffer, bufferSize);

        if (!status) {
            Log::Error ("getReadBuffer failed in call to "
                                 "AlpineQueryPacket::readData!");
            return false;
        }
        readLength = 0;
        curr       = buffer;


        // Peer IP Address
        //
        readLength += sizeof(long);
        if (readLength > bufferSize) {
#ifdef _VERBOSE
            Log::Error ("Ran out of buffer space in call to "
                                 "AlpinePeerPacket::readPeerListData! (Peer IP Address)");
#endif
            return false;
        }
        longValue = static_cast<const ulong>(ntohl(*(reinterpret_cast<const ulong *>(curr))));
        currLocation.setIpAddress (longValue);
        curr += sizeof(long);


        // Port
        //
        readLength += sizeof(short);
        if (readLength > bufferSize) {
#ifdef _VERBOSE
            Log::Error ("Ran out of buffer space in call to "
                                 "AlpinePeerPacket::readPeerListData! (Peer Port)");
#endif
            return false;
        }
        shortValue = static_cast<const ushort>(ntohs(*(reinterpret_cast<const ushort *>(curr))));
        currLocation.setPort (shortValue);
        curr += sizeof(short);


        // Option ID
        if (readLength > bufferSize) {
#ifdef _VERBOSE
            Log::Error ("Ran out of buffer space in call to "
                                 "AlpinePeerPacket::readPeerListData! (Option ID)");
#endif
            return false;

        }
        // Option ID
        //
        readLength       += sizeof(long);
        if (readLength > bufferSize) {
#ifdef _VERBOSE
            Log::Error ("Ran out of buffer space in call to "
                                 "AlpineQueryPacket::readResourceListData! (Option ID)");
#endif
            return false;
        }
        optionId  = static_cast<const ulong>(ntohl(*(reinterpret_cast<const ulong *>(curr))));
        if (optionId == 0) {
            currLocation.setOptionId (optionId);
        }
        curr += sizeof(long);


        // If we have optional extensions given, update DataBuffer and write extensions
        //
        linkBuffer->addReadBytes (readLength);

        if (optionId) {

            // Get this specific extension
            //
            status = AlpineExtensionIndex::getPeerOptionExt (optionId, optionData);
            if (!status) {
#ifdef _VERBOSE
                Log::Debug ("Error locating peer list extension for Option ID in call to "
                                     "AlpinePeerPacket::readPeerListData!");
#endif
                return false;
            }
            // Read optional extension data
            //
            status = optionData->readData (linkBuffer);
            if (!status) {
#ifdef _VERBOSE
                Log::Debug ("Attempted read of extended peer list option data failed in call to "
                                     "AlpinePeerPacket::readPeerListData!");
#endif
                delete optionData;
                return false;
            }
            currLocation.setOptionData (optionData);
            delete optionData;
        }


        // Add this current peer location to packet peer location list
        //
        peerLocationList_->push_back (currLocation);

        if (peerLocationList_->size () == setSize_) {
            finished = true;
        }
    }

    if (peerLocationList_->size () != setSize_) {
        // Bad packet (wrong number of expected replies)  Should never happen here?
#ifdef _VERBOSE
        Log::Debug ("Expected reply set size does not match actual count in list while "
                             "processing packet data in AlpinePeerPacket::readPeerListData.");
#endif
        return false;
    }
    return true;
}



