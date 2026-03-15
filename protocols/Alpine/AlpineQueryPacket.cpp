/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AlpineQueryPacket.h>
#include <AlpinePacket.h>
#include <AlpineQueryOptionData.h>
#include <AlpineExtensionIndex.h>
#include <DataBuffer.h>
#include <Log.h>
#include <StringUtils.h>
#include <NetUtils.h>



AlpineQueryPacket::AlpineQueryPacket ()
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryPacket constructor invoked.");
#endif

    parent_         = nullptr;
    packetType_     = AlpinePacket::t_PacketType::none;
    queryId_        = 0;
    optionId_       = 0;
    optionData_     = nullptr;
    queryString_    = "";
    numHits_        = 0;
    uploadSlots_    = 0;
    offset_         = 0;
    replySetSize_   = 0;
    priority_       = 128;
    resourceList_   = nullptr;
}



AlpineQueryPacket::AlpineQueryPacket (StackLinkInterface * parent)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryPacket link constructor invoked.");
#endif

    parent_         = parent;
    packetType_     = AlpinePacket::t_PacketType::none;
    queryId_        = 0;
    optionId_       = 0;
    optionData_     = nullptr;
    queryString_    = "";
    numHits_        = 0;
    uploadSlots_    = 0;
    offset_         = 0;
    replySetSize_   = 0;
    priority_       = 128;
    resourceList_   = nullptr;
}



AlpineQueryPacket::AlpineQueryPacket (const AlpineQueryPacket & copy)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryPacket copy constructor invoked.");
#endif

    parent_         = copy.parent_;
    packetType_     = copy.packetType_;
    queryId_        = copy.queryId_;
    optionId_       = copy.optionId_;

    if ( (optionId_ != 0) && (copy.optionData_) ) {
        optionData_ = copy.optionData_->duplicate ();
    }

    queryString_    = copy.queryString_;
    numHits_        = copy.numHits_;
    uploadSlots_    = copy.uploadSlots_;
    offset_         = copy.offset_;
    replySetSize_   = copy.replySetSize_;
    priority_       = copy.priority_;

    if (copy.resourceList_) {
        resourceList_ = new t_ResourceDescList;
        *resourceList_ = *copy.resourceList_;
    }
}



AlpineQueryPacket::~AlpineQueryPacket ()
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryPacket destructor invoked.");
#endif

    delete resourceList_;

    delete optionData_;
}



AlpineQueryPacket & 
AlpineQueryPacket::operator = (const AlpineQueryPacket & copy)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryPacket assignment invoked.");
#endif

    if (&copy == this) {
        return *this;
    }

    if (resourceList_) {
        delete resourceList_;
        resourceList_ = nullptr;
    }

    parent_         = copy.parent_;
    packetType_     = copy.packetType_;
    queryId_        = copy.queryId_;
    optionId_       = copy.optionId_;

    if ( (optionId_ != 0) && (copy.optionData_) ) {
        if (optionData_) {
            delete optionData_;
        }

        optionData_ = copy.optionData_->duplicate ();
    }

    queryString_    = copy.queryString_;
    numHits_        = copy.numHits_;
    uploadSlots_    = copy.uploadSlots_;
    offset_         = copy.offset_;
    replySetSize_   = copy.replySetSize_;
    priority_       = copy.priority_;

    if (copy.resourceList_) {
        resourceList_ = new t_ResourceDescList;
        *resourceList_ = *copy.resourceList_;
    }


    return *this;
}



AlpinePacket::t_PacketType
AlpineQueryPacket::getPacketType ()
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryPacket::getPacketType invoked.");
#endif

    return packetType_;
}



bool 
AlpineQueryPacket::setPacketType (AlpinePacket::t_PacketType  type)
{
#ifdef _VERBOSE
    string packetTypeString;
    AlpinePacket::packetTypeAsString (type, packetTypeString);
    Log::Debug ("AlpineQueryPacket::setPacketType invoked.  New type: "s + packetTypeString);
#endif

    if ( (type != AlpinePacket::t_PacketType::queryDiscover ) &&
         (type != AlpinePacket::t_PacketType::queryOffer ) &&
         (type != AlpinePacket::t_PacketType::queryRequest ) &&
         (type != AlpinePacket::t_PacketType::queryReply ) )  {

        Log::Error ("Invalid packet type passed in call to AlpineQueryPacket::setPacketType!");
        return false;
    }
    packetType_ = type;

    return true;
}



bool  
AlpineQueryPacket::setQueryId (ulong  queryId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryPacket::setQueryId invoked.  Query ID: "s +
                std::to_string (queryId));
#endif

    queryId_ = queryId;

    return true;
}



bool  
AlpineQueryPacket::getQueryId (ulong &  queryId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryPacket::getQueryId invoked.");
#endif

    queryId_ =  queryId;

    return true;
}



bool  
AlpineQueryPacket::setOptionId (ulong  optionId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryPacket::setOptionId invoked.  Option ID: "s +
                std::to_string (optionId));
#endif

    if (optionId != 0) {
        Log::Error ("Attempt to set extended option ID in call to "
                             "AlpineQueryPacket::setOptionId!  Use setOptionData for extended options.");
        return false;
    }
    optionId_ = optionId;

    return true;
}



bool  
AlpineQueryPacket::getOptionId (ulong &  optionId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryPacket::getOptionId invoked.");
#endif

    optionId = optionId_;

    return true;
}



bool  
AlpineQueryPacket::setOptionData (AlpineQueryOptionData *  optionData)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryPacket::setOptionData invoked.");
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
AlpineQueryPacket::getOptionData (AlpineQueryOptionData *&  optionData)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryPacket::getOptionData invoked.");
#endif

    if ( (optionId_ == 0) || (!optionData_) ) {
        Log::Error ("Attempt to get option data when no extension set in call to "
                             "AlpineQueryPacket::getOptionData!");
        return false;
    }
    optionData = optionData_->duplicate ();


    return true;
}



bool  
AlpineQueryPacket::setQueryString (const string &  queryString)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryPacket::setQueryString invoked.  Query: "s + queryString);
#endif

    queryString_ = queryString;

    return true;
}



bool  
AlpineQueryPacket::getQueryString (string &  queryString)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryPacket::getQueryString invoked.");
#endif

    queryString = queryString_;

    return true;
}



bool  
AlpineQueryPacket::setNumHits (ulong  numHits)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryPacket::setNumHits invoked.  NumHits: "s +
                std::to_string (numHits));
#endif

    numHits_ = numHits;

    return true;
}



bool  
AlpineQueryPacket::getNumHits (ulong &  numHits)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryPacket::getNumHits invoked.");
#endif

    numHits = numHits_;

    return true;
}



bool  
AlpineQueryPacket::setUploadSlots (ushort  uploadSlots)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryPacket::setUploadSlots invoked.  Slots: "s +
                std::to_string(uploadSlots));
#endif

    uploadSlots_ = uploadSlots;

    return true;
}



bool  
AlpineQueryPacket::getUploadSlots (ushort &  uploadSlots)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryPacket::getUploadSlots invoked.");
#endif

    uploadSlots = uploadSlots_;

    return true;
}



bool  
AlpineQueryPacket::setOffset (ulong  offset)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryPacket::setOffset invoked.  Offset: "s +
                std::to_string (offset));
#endif

    offset_ = offset;

    return true;
}



bool  
AlpineQueryPacket::getOffset (ulong &  offset)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryPacket::getOffset invoked.");
#endif

    offset = offset_;

    return true;
}



bool  
AlpineQueryPacket::setReplySetSize (ushort  setSize)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryPacket::setReplySetSize invoked.  Set Size: "s +
                std::to_string (setSize));
#endif

    replySetSize_ = setSize;

    return true;
}



bool
AlpineQueryPacket::getReplySetSize (ushort &  setSize)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryPacket::getReplySetSize invoked.");
#endif

    setSize = replySetSize_;

    return true;
}



bool
AlpineQueryPacket::setPriority (uint8_t  priority)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryPacket::setPriority invoked.  Priority: "s +
                std::to_string (priority));
#endif

    priority_ = priority;

    return true;
}



bool
AlpineQueryPacket::getPriority (uint8_t &  priority)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryPacket::getPriority invoked.");
#endif

    priority = priority_;

    return true;
}



bool
AlpineQueryPacket::setTraceContext (const string &  traceContext)
{
#ifdef _VERBOSE
    Log::Debug("AlpineQueryPacket::setTraceContext invoked.");
#endif

    traceContext_ = traceContext;
    return true;
}



bool
AlpineQueryPacket::getTraceContext (string &  traceContext)
{
#ifdef _VERBOSE
    Log::Debug("AlpineQueryPacket::getTraceContext invoked.");
#endif

    traceContext = traceContext_;
    return true;
}



bool
AlpineQueryPacket::setResourceDescList (t_ResourceDescList &  resourceList)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryPacket::setResourceDescList invoked.");
#endif

    if (resourceList_) {
        delete resourceList_;
        resourceList_ = nullptr;
    }

    resourceList_   = new t_ResourceDescList(resourceList);


    return true;
}



bool  
AlpineQueryPacket::getResourceDescList (t_ResourceDescList &  resourceList)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryPacket::getResourceDescList invoked.");
#endif

    if (!resourceList_) {
        return false;
    }
    resourceList = *resourceList_;

    return true;
}



bool  
AlpineQueryPacket::setParent (StackLinkInterface *  parent)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryPacket::setParent invoked.");
#endif

    parent_ = parent;

    return true;
}



void  
AlpineQueryPacket::unsetParent ()
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryPacket::unsetParent invoked.");
#endif

    parent_ = nullptr;
}



bool
AlpineQueryPacket::writeData (DataBuffer * linkBuffer)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryPacket::writeData invoked.");
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
        Log::Debug ("getWriteBuffer failed in AlpineQueryPacket::writeData.");

        return false;
    }
    if (packetType_ == AlpinePacket::t_PacketType::queryDiscover) {
        // Query Discover Message
        //
        // 4b - Query ID
        // Nb - Query String
        // 4b - Option ID
        // 0-Nb - Option Data (if applicable)
        //
        writeLength = sizeof(long) +
                      sizeof(long) +
                      queryString_.size () + 1;

        if ( (optionId_) && (optionData_) ) {
            writeLength += optionData_->getOptionDataLength ();
        }
    }
    else if (packetType_ == AlpinePacket::t_PacketType::queryOffer) {
        // Query Offer Message
        //
        // 4b - Query ID
        // 4b - Number of Hits
        // 2b - Upload slots available
        // 4b - Option ID
        // 0-Nb - Option Data (if applicable)
        //
        writeLength = sizeof(long) +
                      sizeof(long) + 
                      sizeof(short) + 
                      sizeof(long);

        if ( (optionId_) && (optionData_) ) {
            writeLength += optionData_->getOptionDataLength ();
        }
    }
    else if (packetType_ == AlpinePacket::t_PacketType::queryRequest) {
        // Query Request Message
        //
        // 4b - Query ID
        // 4b - Offset
        // 2b - Desired Reply Set Size
        // Nb - Query String
        // 4b - Option ID
        // 0-Nb - Option Data (if applicable)
        //
        writeLength = sizeof(long) + 
                      sizeof(long) + 
                      sizeof(short) + 
                      sizeof(long) + 
                      queryString_.size () + 1;

        if ( (optionId_) && (optionData_) ) {
            writeLength += optionData_->getOptionDataLength ();
        }
    }
    else if (packetType_ == AlpinePacket::t_PacketType::queryReply) {
        // Query Reply Message
        //
        // 4b - Query ID
        // 2b - Reply Set Size
        // Nb - List of t_ResourceDesc objects
        //

        // See if we have a resource list set before performing any processing
        //
        if (!resourceList_) {
            Log::Error ("Cannot send queryReply with no resource list set in "
                                 "AlpineQueryPacket::writeData!");

            return false;
        }
        writeLength = calculateResourceListSize ();
        writeLength += sizeof(long) + sizeof(short);
    }


    // Make sure we have enough space to write the data.
    //
    if (bufferSize < writeLength) {
        Log::Debug ("Insufficient space in write buffer for AlpineQueryPacket::writeData!");
        return false;
    }
    // Write data to packet buffer
    //
    curr = buffer;

    // All query types use the query ID
    //
    *(reinterpret_cast<ulong *>(curr)) = htonl(queryId_);
    curr += sizeof(long);

    // Reset the write length tally according to what _IS_ currently written and
    // not what is expected to be written overall.
    //
    writeLength = sizeof(long);

    if (packetType_ == AlpinePacket::t_PacketType::queryDiscover) {
        strcpy (reinterpret_cast<char *>(curr), queryString_.c_str());
        curr        += queryString_.size () + 1;
        writeLength += queryString_.size () + 1;

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
                                     "AlpineQueryPacket::writeData! (queryDiscover packet)");
                return false;
            }
        }
    }
    else if (packetType_ == AlpinePacket::t_PacketType::queryOffer) {
        *(reinterpret_cast<ulong *>(curr)) = htonl(numHits_);
        curr        += sizeof(long);
        writeLength += sizeof(long);

        *(reinterpret_cast<ushort *>(curr)) = htons(uploadSlots_);
        curr        += sizeof(short);      
        writeLength += sizeof(short);      
    
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
                                     "AlpineQueryPacket::writeData! (queryOffer packet)");
                return false;
            }
        }
    }
    else if (packetType_ == AlpinePacket::t_PacketType::queryRequest) {
        *(reinterpret_cast<ulong *>(curr)) = htonl(offset_);
        curr        += sizeof(long);
        writeLength += sizeof(long);
    
        *(reinterpret_cast<ushort *>(curr)) = htons(replySetSize_);
        curr        += sizeof(short);      
        writeLength += sizeof(short);      

        strcpy (reinterpret_cast<char *>(curr), queryString_.c_str());
        curr        += queryString_.size () + 1;
        writeLength += queryString_.size () + 1;

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
                                     "AlpineQueryPacket::writeData! (queryRequest packet)");
                return false;
            }
        }
    }
    else if (packetType_ == AlpinePacket::t_PacketType::queryReply) {
        *(reinterpret_cast<ushort *>(curr)) = htons(replySetSize_);
        curr        += sizeof(short);
        writeLength += sizeof(short);

        linkBuffer->addWriteBytes (writeLength);

        status = writeResourceListData (linkBuffer);

        if (!status) {
            return false;
        }
    }

    // Write priority field (protocol version >= 1)
    //
    {
        byte * priBuf;
        uint   priBufSize;
        status = linkBuffer->getWriteBuffer (priBuf, priBufSize);

        if (!status || priBufSize < sizeof(uint8_t)) {
            Log::Debug ("Insufficient space for priority in AlpineQueryPacket::writeData.");
            return false;
        }
        *priBuf = priority_;
        linkBuffer->addWriteBytes (sizeof(uint8_t));
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
AlpineQueryPacket::readData (DataBuffer * linkBuffer)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryPacket::readData invoked.");
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
    if (packetType_ == AlpinePacket::t_PacketType::queryDiscover) {
        // Query Discover Message
        //
        // 4b - Query ID
        // Nb - Query String
        // 4b - Option ID
        //
        readLength = sizeof(long) +
                     sizeof(long) +
                     + 2;  // at least one char and null terminator
    }
    else if (packetType_ == AlpinePacket::t_PacketType::queryOffer) {
        // Query Offer Message
        //
        // 4b - Query ID
        // 4b - Number of Hits
        // 2b - Upload slots available
        // 4b - Option ID
        //
        readLength = sizeof(long) +
                     sizeof(long) + 
                     sizeof(short) + 
                     sizeof(long);
    }
    else if (packetType_ == AlpinePacket::t_PacketType::queryRequest) {
        // Query Request Message
        //
        // 4b - Query ID
        // 4b - Offset
        // 2b - Desired Reply Set Size
        // Nb - Query String
        // 4b - Option ID
        //
        readLength = sizeof(long) + 
                     sizeof(long) + 
                     sizeof(short) + 
                     sizeof(long) + 
                     + 2;  // at least one char and null terminator
    }
    else if (packetType_ == AlpinePacket::t_PacketType::queryReply) {
        // Query Request Message
        //
        // 4b - Query ID
        // 2b - Reply Set Size
        // Nb - List of t_ResourceDesc objects
        //
        readLength = sizeof(long) +
                     sizeof(short) +
                     (sizeof(long) *3) + 2 + 2;  // at least one descriptor, two 1 char strings
    }

    if (bufferSize < readLength) {
#ifdef _VERBOSE
        Log::Debug ("Packet size too small in AlpineQueryPacket::readData.");
#endif
        return false;
    }
    // Read data from packet buffer
    //
    curr       = buffer;
    readLength = 0;

    // All query packet types contain a query ID
    //
    queryId_    = static_cast<ulong>(ntohl(*(reinterpret_cast<ulong *>(curr))));
    curr       += sizeof(long);
    readLength += sizeof(long);

    if (packetType_ == AlpinePacket::t_PacketType::queryDiscover) {
        status = verifyStringData (curr, (bufferSize - readLength));

        if (!status) {
#ifdef _VERBOSE
            Log::Debug ("Invalid string in query discover packet in "
                                 "AlpineQueryPacket::readData! (queryDiscover packet)");
#endif
            return false;
        }
        queryString_ = reinterpret_cast<const char *>(curr);
        curr        += queryString_.size () + 1;
        readLength  += queryString_.size () + 1;

        if (readLength + sizeof(long) > bufferSize) {
            Log::Error ("AlpineQueryPacket: buffer too small for option ID in queryDiscover"s);
            return false;
        }
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

            status = AlpineExtensionIndex::getQueryOptionExt (optionId_, optionData_);

            if (!status) {
#ifdef _VERBOSE
                Log::Error ("Attempt to get QueryOptionExt(Data) failed for OptionID in call to "
                                     "AlpineQueryPacket::readData! (queryDiscover packet)");
#endif
                return false;
            }
            status = optionData_->readData (linkBuffer);

            if (!status) {
#ifdef _VERBOSE
                Log::Error ("readData failed for Option Data in call to "
                                     "AlpineQueryPacket::readData! (queryDiscover packet)");
#endif
                return false;
            }
        }
    }
    else if (packetType_ == AlpinePacket::t_PacketType::queryOffer) {
        numHits_      = static_cast<ulong>(ntohl(*(reinterpret_cast<ulong *>(curr))));
        curr         += sizeof(long);
        readLength   += sizeof(long);

        uploadSlots_  = static_cast<short>(ntohl(*(reinterpret_cast<short *>(curr))));
        curr         += sizeof(short);
        readLength   += sizeof(short);

        optionId_     = static_cast<ulong>(ntohl(*(reinterpret_cast<ulong *>(curr))));
        curr         += sizeof(long);
        readLength   += sizeof(long);


        // If extended query options given, read them into the buffer.
        //
        linkBuffer->addReadBytes (readLength);

        if (optionId_) {
            if (optionData_) {
                delete optionData_;
                optionData_ = nullptr;
            }

            status = AlpineExtensionIndex::getQueryOptionExt (optionId_, optionData_);

            if (!status) {
#ifdef _VERBOSE
                Log::Error ("Attempt to get QueryOptionExt(Data) failed for OptionID in call to "
                                     "AlpineQueryPacket::readData! (queryOffer packet)");
#endif
                return false;
            }
            status = optionData_->readData (linkBuffer);

            if (!status) {
#ifdef _VERBOSE
                Log::Error ("readData failed for Option Data in call to "
                                     "AlpineQueryPacket::readData! (queryOffer packet)");
#endif
                return false;
            }
        }
    }
    else if (packetType_ == AlpinePacket::t_PacketType::queryRequest) {
        offset_     = static_cast<ulong>(ntohl(*(reinterpret_cast<ulong *>(curr))));
        curr       += sizeof(long);
        readLength += sizeof(long);

        replySetSize_   = static_cast<short>(ntohl(*(reinterpret_cast<short *>(curr))));
        curr           += sizeof(short);
        readLength     += sizeof(short);

        static constexpr ushort MAX_REPLY_SET_SIZE = 10000;
        if (replySetSize_ > MAX_REPLY_SET_SIZE) {
            Log::Error ("AlpineQueryPacket: reply set size exceeds maximum"s);
            return false;
        }

        status = verifyStringData (curr, (bufferSize - readLength));

        if (!status) {
#ifdef _VERBOSE
            Log::Debug ("Invalid string in query request packet in "
                                 "AlpineQueryPacket::readData! (queryRequest packet)");
#endif
            return false;
        }
        queryString_ = reinterpret_cast<const char *>(curr);
        curr        += queryString_.size () + 1;
        readLength  += queryString_.size () + 1;

        if (readLength + sizeof(long) > bufferSize) {
            Log::Error ("AlpineQueryPacket: buffer too small for option ID in queryRequest"s);
            return false;
        }
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

            status = AlpineExtensionIndex::getQueryOptionExt (optionId_, optionData_);

            if (!status) {
#ifdef _VERBOSE
                Log::Error ("Attempt to get QueryOptionExt(Data) failed for OptionID in call to "
                                     "AlpineQueryPacket::readData! (queryRequest packet)");
#endif
                return false;
            }
            status = optionData_->readData (linkBuffer);

            if (!status) {
#ifdef _VERBOSE
                Log::Error ("readData failed for Option Data in call to "
                                     "AlpineQueryPacket::readData! (queryRequest packet)");
#endif
                return false;
            }
        }
    }
    else if (packetType_ == AlpinePacket::t_PacketType::queryReply) {
        replySetSize_   = static_cast<short>(ntohl(*(reinterpret_cast<short *>(curr))));
        curr           += sizeof(short);
        readLength     += sizeof(short);

        static constexpr ushort MAX_REPLY_SET_SIZE = 10000;
        if (replySetSize_ > MAX_REPLY_SET_SIZE) {
            Log::Error ("AlpineQueryPacket: reply set size exceeds maximum"s);
            return false;
        }

        linkBuffer->addReadBytes (readLength);

        if (!resourceList_) {
            resourceList_ = new t_ResourceDescList;
        }

        status = readResourceListData (linkBuffer);

        if (!status) {
#ifdef _VERBOSE
            Log::Debug ("Invalid resource list data in query reply packet in "
                                 "AlpineQueryPacket::readData.");
#endif
            return false;
        }
    }

    // Read priority field if present (protocol version >= 1).
    // If the remaining buffer has at least one byte, treat it as priority;
    // otherwise default to 128 (normal) for legacy packets.
    //
    {
        byte * priBuf;
        uint   priBufSize;
        status = linkBuffer->getReadBuffer (priBuf, priBufSize);

        if (status && priBufSize >= sizeof(uint8_t)) {
            priority_ = *priBuf;
            linkBuffer->addReadBytes (sizeof(uint8_t));
        }
        else {
            priority_ = 128;
        }
    }


    return true;
}



ulong
AlpineQueryPacket::calculateResourceListSize ()
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryPacket::calculateResourceListSize invoked.");
#endif

    if (!resourceList_) {
        return false;
    }
    ulong  listDataSize = 0;
    AlpineResourceDesc *          currDesc;

    AlpineResourceDesc::t_LocatorList            locatorList;

    string  description;
    AlpineQueryOptionData *  optionData;

    for (auto& resourceItem : *resourceList_) {

        currDesc = &resourceItem;

        listDataSize += sizeof(long); // matchId
        listDataSize += sizeof(long); // size

        // Calculate data for list of locator strings
        //
        listDataSize += sizeof(short); // locator list length

        currDesc->getLocatorList (locatorList);
        for (const auto& locatorItem : locatorList) {
            listDataSize += locatorItem.size () + 1;
        }

        currDesc->getDescription (description);
        listDataSize += description.size () + 1;
        listDataSize += sizeof(long); // option ID

        if (currDesc->getOptionId () != 0) {
            currDesc->getOptionData (optionData);
            listDataSize += optionData->getOptionDataLength ();
            delete optionData;
        }
    }


    return listDataSize;
}



bool   
AlpineQueryPacket::writeResourceListData (DataBuffer *  linkBuffer)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryPacket::writeResourceListData invoked.");
#endif

    if ( (!resourceList_) || (resourceList_->size () == 0) ) {
        Log::Error ("Invalid resource list present in call to "
                             "AlpineQueryPacket::writeResourceListData!");
        return false;
    }
    // Data Buffer info
    //
    bool   status;
    byte * buffer;
    byte * curr;
    uint   bufferSize;
    uint   writeLength;


    AlpineResourceDesc *          currDesc;

    AlpineResourceDesc::t_LocatorList            locatorList;

    string  description;
    AlpineQueryOptionData *  optionData;

    for (auto& resourceItem : *resourceList_) {

        // We must refresh our write buffer data with each iteration as
        // extended option data will modify the write buffer.
        //
        status = linkBuffer->getWriteBuffer (buffer, bufferSize);

        if (!status) {
            Log::Error ("getWriteBuffer failed in call to "
                                 "AlpineQueryPacket::writeResourceListData!");
            return false;
        }
        writeLength = 0;
        curr        = buffer;


        // Begin write
        //
        currDesc = &resourceItem;


        // Match ID
        //
        writeLength += sizeof(long);
        if (writeLength > bufferSize) {
            Log::Error ("Ran out of buffer space in call to "
                                 "AlpineQueryPacket::writeResourceListData! (Match ID)");
            return false;
        }
        *(reinterpret_cast<ulong *>(curr)) = htonl(currDesc->getMatchId ());
        curr        += sizeof(long);


        // Resource Size
        //
        writeLength += sizeof(long);
        if (writeLength > bufferSize) {
            Log::Error ("Ran out of buffer space in call to "
                                 "AlpineQueryPacket::writeResourceListData! (Resource Size)");
            return false;
        }
        *(reinterpret_cast<ulong *>(curr)) = htonl(currDesc->getSize ());
        curr += sizeof(long);


        // Locator list length
        //
        currDesc->getLocatorList (locatorList);

        writeLength += sizeof(short);
        if (writeLength > bufferSize) {
            Log::Error ("Ran out of buffer space in call to "
                                 "AlpineQueryPacket::writeResourceListData! (Locator List Length)");
            return false;
        }
        *(reinterpret_cast<ushort *>(curr)) = htons((ushort)(locatorList.size ()));
        curr += sizeof(short);


        // Locator list
        //
        for (const auto& locatorItem : locatorList) {
            writeLength += locatorItem.size () + 1;
        }
        if (writeLength > bufferSize) {
            Log::Error ("Ran out of buffer space in call to "
                                 "AlpineQueryPacket::writeResourceListData! (Locator List)");
            return false;
        }
        for (const auto& locatorItem : locatorList) {
            strcpy (reinterpret_cast<char *>(curr), locatorItem.c_str());
            curr += locatorItem.size () + 1;
        }


        // Description
        //
        currDesc->getDescription (description);
        writeLength += description.size () + 1;
        if (writeLength > bufferSize) {
            Log::Error ("Ran out of buffer space in call to "
                                 "AlpineQueryPacket::writeResourceListData! (Description)");
            return false;
        }
        strcpy (reinterpret_cast<char *>(curr), description.c_str());
        curr += description.size () + 1;


        // Option ID
        //
        writeLength += sizeof(long);
        if (writeLength > bufferSize) {
            Log::Error ("Ran out of buffer space in call to "
                                 "AlpineQueryPacket::writeResourceListData! (Option ID)");
            return false;
        }
        *(reinterpret_cast<ulong *>(curr)) = htonl(currDesc->getOptionId ());
        curr += sizeof(long);


        // If we have optional extensions given, update DataBuffer and write extensions
        //
        linkBuffer->addWriteBytes (writeLength);

        if (currDesc->getOptionId ()) {
            currDesc->getOptionData (optionData);

            status = optionData->writeData (linkBuffer);
            delete optionData;

            if (!status) {
                Log::Error ("Attempted write of extended option data failed in call to "
                                     "AlpineQueryPacket::writeResourceListData!");
                return false;
            }
        }
    }


    return true;
}



bool
AlpineQueryPacket::readResourceListData (DataBuffer *  linkBuffer)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryPacket::readResourceListData invoked.");
#endif

    if (!resourceList_) {
        resourceList_ = new t_ResourceDescList;
    }
    else {
        delete resourceList_;
        resourceList_ = new t_ResourceDescList;
    }

    // Resource Description Data
    //
    AlpineResourceDesc                 currDesc;
    AlpineResourceDesc::t_LocatorList  locatorList;
    AlpineQueryOptionData *            optionData;
    ulong  longValue;
    ulong  optionId;


    // Buffer data
    //
    bool   status;
    byte * buffer;
    byte * curr;
    uint   bufferSize;
    uint   readLength;
    ushort expectedListLength;
    ushort currListIndex;
    ulong  stringBytes = 0;
    string currString;

    bool   finished = false;
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


        // Match ID
        //
        readLength += sizeof(long);
        if (readLength > bufferSize) {
#ifdef _VERBOSE
            Log::Error ("Ran out of buffer space in call to "
                                 "AlpineQueryPacket::readResourceListData! (Match ID)");
#endif
            return false;
        }
        longValue = static_cast<const ulong>(ntohl(*(reinterpret_cast<const ulong *>(curr))));
        currDesc.setMatchId (longValue);
        curr += sizeof(long);


        // Resource Size
        //
        readLength       += sizeof(long);
        if (readLength > bufferSize) {
#ifdef _VERBOSE
            Log::Error ("Ran out of buffer space in call to "
                                 "AlpineQueryPacket::readResourceListData! (Resource Size)");
#endif
            return false;
        }
        longValue  = static_cast<const ulong>(ntohl(*(reinterpret_cast<const ulong *>(curr))));
        currDesc.setSize (longValue);
        curr += sizeof(long);


        // Locator List Length
        //
        readLength += sizeof(short);
        if (readLength > bufferSize) {
#ifdef _VERBOSE
            Log::Error ("Ran out of buffer space in call to "
                                 "AlpineQueryPacket::readResourceListData! (Locator List Length)");
#endif
            return false;
        }
        expectedListLength = static_cast<const ushort>(ntohs(*(reinterpret_cast<const ushort *>(curr))));
        curr += sizeof(short);


        // Locator List
        //
        locatorList.clear ();
        for (currListIndex = 0; currListIndex < expectedListLength; currListIndex++) {

            status = verifyStringData (curr, bufferSize - readLength);
            if (!status) {
#ifdef _VERBOSE
                Log::Debug ("Invalid locator string in query reply packet in "
                                     "AlpineQueryPacket::readResourceListData.");
#endif
                return false;
            }
            currString  = reinterpret_cast<const char *>(curr);
            stringBytes = currString.size () + 1;
            curr       += stringBytes;
            readLength += stringBytes;

            locatorList.push_back (currString);
        }

        if (locatorList.size () != expectedListLength) {
            // Should never occur, due to verify string data?
#ifdef _VERBOSE
            Log::Debug ("Invalid number of locator strings in query reply packet in "
                                 "AlpineQueryPacket::readResourceListData.");
#endif
            return false;
        }
        currDesc.setLocatorList (locatorList);


        // Description
        //
        status = verifyStringData (curr, bufferSize - readLength);

        if (!status) {
#ifdef _VERBOSE
            Log::Debug ("Invalid description string in query reply packet in "
                                 "AlpineQueryPacket::readResourceListData.");
#endif
            return false;
        }
        currString   = reinterpret_cast<const char *>(curr);
        currDesc.setDescription (currString);
        stringBytes  = currString.size () + 1;
        curr        += stringBytes;
        readLength  += stringBytes;


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
            currDesc.setOptionId (optionId);
        }
        curr += sizeof(long);


        // If we have optional extensions given, update DataBuffer and write extensions
        //
        linkBuffer->addReadBytes (readLength);

        if (optionId) {

            // Get this specific extension
            //
            status = AlpineExtensionIndex::getQueryOptionExt (optionId, optionData);
            if (!status) {
#ifdef _VERBOSE
                Log::Debug ("Error locating extension for Option ID in call to "
                                     "AlpineQueryPacket::readResourceListData!");
#endif
                return false;
            }
            // Read optional extension data
            //
            status = optionData->readData (linkBuffer);
            if (!status) {
#ifdef _VERBOSE
                Log::Debug ("Attempted read of extended option data failed in call to "
                                     "AlpineQueryPacket::readResourceListData!");
#endif
                delete optionData;
                return false;
            }
            currDesc.setOptionData (optionData);
            delete optionData;
        }


        // Add this current reply descriptor to list
        //
        resourceList_->push_back (currDesc);

        if (resourceList_->size () == replySetSize_) {
            finished = true;
        }
    }

    // As one last check, verify that the number of replies we received is what we expected...
    //
    if (resourceList_->size () != replySetSize_) {
        // Should never occur?
#ifdef _VERBOSE
        Log::Debug ("Expected reply set size does not match actual count in list while "
                             "processing packet data in AlpineQueryPacket::readResourceListData.");
#endif
        return false;
    }
    return true;
}



bool   
AlpineQueryPacket::verifyStringData (const byte *  data,
                                     ulong         dataLength)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineQueryPacket::verifyStringData invoked.");
#endif

    // All we do here is verify that we have a null terminator somewhere at the end of dataLength chars.
    // (prevent buffer overruns, etc.
    //
    if (dataLength == 0) {
        return false;
    }
    const byte * curr   = data;
    ulong i;

    for (i = 0; i < dataLength; i++) {
        if (*curr == 0) {
            return true;

        }
        curr++;
    }


    return false;
}



