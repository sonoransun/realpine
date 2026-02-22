/// Copyright (C) 2026 sonoransun — see LICENCE.txt



#include <DtcpBaseConnTransport.h>
#include <DtcpBaseUdpTransport.h>
#include <DtcpBaseConnMux.h>
#include <DtcpConnPacket.h>
#include <DtcpDataPacket.h>
#include <DataBuffer.h>
#include <Log.h>
#include <StringUtils.h>
#include <Platform.h>



ulong            DtcpBaseConnTransport::currSequenceNum_s = 0;



DtcpBaseConnTransport::DtcpBaseConnTransport ()
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnTransport constructor invoked.");
#endif

    parentTransport_  = nullptr;
    mux_              = nullptr;
    lastRecv_.tv_sec  = 0;
    lastRecv_.tv_usec = 0;
    lastSend_.tv_sec  = 0;
    lastSend_.tv_usec = 0;
    peerId_           = 0;
    myId_             = 0;
    peerIpAddress_    = 0;
    peerPort_         = 0;
    pendingAck_       = false;
    requestId_        = 0;
    sendSequenceNum_  = 0;
    recvSequenceNum_  = 0;
}



DtcpBaseConnTransport::~DtcpBaseConnTransport ()
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnTransport destructor invoked.");
#endif

    // MRP_TEMP something should be cleanup up here.. not sure what. 
}



bool 
DtcpBaseConnTransport::setParent (DtcpBaseUdpTransport * parent)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnTransport::setParent invoked.");
#endif

    parentTransport_  = parent;

    return true;
}



bool
DtcpBaseConnTransport::getParent (DtcpBaseUdpTransport *& parent)
{
    parent = parentTransport_;
    return true;
}



bool 
DtcpBaseConnTransport::getDataBuffer (DataBuffer *& dataBuffer)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnTransport::getDataBuffer invoked.");
#endif

    bool status;
    status = parentTransport_->getDataBuffer (dataBuffer);

    return status;
}



bool 
DtcpBaseConnTransport::processData (const byte * data,
                                    uint         dataLength)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnTransport::processData invoked.");
#endif


    return true;
}



bool 
DtcpBaseConnTransport::processPacket (StackLinkInterface * packet)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnTransport::processPacket invoked.");
#endif

  
    DtcpConnPacket * connPacket;
    connPacket = dynamic_cast<DtcpConnPacket *>(packet);

    if (!connPacket) {
        Log::Error ("Invalid packet type passed to DtcpBaseConnTransport::processPacket.");

        return false;
    }
    DataBuffer * dataBuffer;
    parentTransport_->getDataBuffer (dataBuffer);

    DtcpPacket::t_PacketType  packetType;
    packetType = connPacket->getPacketType ();    

    if ( (packetType == DtcpPacket::t_PacketType::connData) ||
         (packetType == DtcpPacket::t_PacketType::connReliableData) ) {
        // send incoming data to derived handler
        //
        byte * data;
        uint   dataLength;

        dataBuffer->getReadBuffer (data, dataLength);

        handleData (data, dataLength);
    }
    else {
        // MRP_TEMP complete handlers
        //
        Log::Debug ("Unhandled packet type passed to DtcpBaseConnTransport::processPacket.");
        return false;
    }


    return true;
}



bool 
DtcpBaseConnTransport::sendData (const byte * data,
                                 uint         dataLength)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnTransport::sendData invoked.");
#endif


    // Create a DtcpDataPacket to pass this down the link chain.
    //
    DtcpDataPacket * dataPacket;
    dataPacket =  new DtcpDataPacket ();

    dataPacket->setPacketData (data, dataLength);

    bool status;
    status = sendPacket (dataPacket);


    return status;
}



bool 
DtcpBaseConnTransport::sendPacket (StackLinkInterface * packet)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnTransport::sendPacket invoked.");
#endif

    // Set packet links for packet transfer
    //
    bool status;

    DtcpConnPacket * dtcpConnPacket = new DtcpConnPacket ();
    DtcpPacket * dtcpPacket = new DtcpPacket();

    dtcpPacket->setParent (dtcpConnPacket);
    dtcpConnPacket->setParent (packet);

    dtcpConnPacket->setPeerLocation (peerIpAddress_, peerPort_);
    dtcpConnPacket->setMyId (myId_);
    dtcpConnPacket->setPacketType (DtcpPacket::t_PacketType::connData);

    status = parentTransport_->sendPacket (dtcpPacket);


    return status;
}



bool 
DtcpBaseConnTransport::sendReliableData (const byte * data,
                                         uint         dataLength)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnTransport::sendReliableData invoked.");
#endif

    // Create a DtcpDataPacket to pass this down the link chain.
    //
    DtcpDataPacket * dataPacket;
    dataPacket =  new DtcpDataPacket ();

    dataPacket->setPacketData (data, dataLength);

    bool status;
    status = sendReliablePacket (dataPacket);


    return status;
}



bool 
DtcpBaseConnTransport::sendReliablePacket (StackLinkInterface * packet)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnTransport::sendReliablePacket invoked.");
#endif

    // Cannot send multiple reliable packets over the same transport
    // at one time.  (Requires use of a higher level transport)
    //
    if (pendingAck_) {
        Log::Debug ("Unable to send reliable packet; awaiting previously reply "
                             "in DtcpBaseConnTransport::sendReliableData.");

        return false;
    }
    // Not concerned about synchronization here, just need a fairly
    // unique sequence ID per send.
    //
    currSequenceNum_s++;
    sendSequenceNum_ = currSequenceNum_s;

    // the sequence number should never be zero
    //
    if (!currSequenceNum_s) currSequenceNum_s++;


    // Set packet links for packet transfer
    //
    bool status;

    DtcpConnPacket * dtcpConnPacket = new DtcpConnPacket ();
    DtcpPacket * dtcpPacket = new DtcpPacket();

    dtcpPacket->setParent (dtcpConnPacket);
    dtcpConnPacket->setParent (packet);

    dtcpConnPacket->setPeerLocation (peerIpAddress_, peerPort_);
    dtcpConnPacket->setMyId (myId_);
    dtcpConnPacket->setPacketType (DtcpPacket::t_PacketType::connReliableData);
    dtcpConnPacket->setSequenceNum (sendSequenceNum_);

    status = parentTransport_->sendReliablePacket (dtcpPacket,
                                                   this,
                                                   requestId_);

    if (status) {
        pendingAck_ = true;
    }


    return status;
}



bool 
DtcpBaseConnTransport::pendingAck ()
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnTransport::pendingAck invoked.");
#endif

    return pendingAck_;
}



bool 
DtcpBaseConnTransport::acknowledgeTransfer ()
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnTransport::acknowledgeTransfer invoked.");
#endif

    if (!pendingAck_) {
        return false;
    }
    pendingAck_ = false;
    mux_->acknowledgeTransfer (this);


    return true;
}



bool 
DtcpBaseConnTransport::handleSendFailure (ulong  id)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnTransport::handleSendFailure invoked.");
#endif

    if (id != requestId_) {
        Log::Error ("Invalid request ID passed back to DtcpBaseConnTransport::handleSendFailure!");
        return false;
    }
    pendingAck_ = false;

    bool status;
    status = handleSendFailure ();


    return status;
}



bool 
DtcpBaseConnTransport::handleSendReceived (ulong  id)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnTransport::handleSendReceived invoked.");
#endif

    if (id != requestId_) { 
        Log::Error ("Invalid request ID passed back to DtcpBaseConnTransport::handleSendFailure!");
        return false;
    }
    pendingAck_ = false;

    bool status;
    status = handleSendReceived ();


    return status;
}



bool
DtcpBaseConnTransport::setTransportId (ulong transportId)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnTransport::setTransportId invoked.");
#endif

    transportId_ = transportId;

    return true;
}



bool
DtcpBaseConnTransport::getTransportId (ulong & transportId)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnTransport::getTransportId invoked.");
#endif

    transportId = transportId_;

    return true;
}



bool
DtcpBaseConnTransport::setPeerId (ulong peerId)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnTransport::setPeerId invoked.");
#endif

    peerId_ = peerId;

    return true;
}



bool
DtcpBaseConnTransport::getPeerId (ulong & peerId)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnTransport::getPeerId invoked.");
#endif

    peerId = peerId_;

    return true;
}



bool
DtcpBaseConnTransport::setMyId (ulong myId)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnTransport::setMyId invoked.");
#endif

    myId_ = myId;

    return true;
}



bool
DtcpBaseConnTransport::getMyId (ulong & myId)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnTransport::getMyId invoked.");
#endif

    myId = myId_;

    return true;
}



bool
DtcpBaseConnTransport::getPeerLocation (ulong &   ipAddress,
                                        ushort &  port)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnTransport::getPeerLocation invoked.");
#endif

    if (!peerIpAddress_) {
        return false;
    }
    ipAddress = peerIpAddress_;
    port = peerPort_;

    return true;
}



bool 
DtcpBaseConnTransport::setMux (DtcpBaseConnMux *  mux)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnTransport::setMux invoked.");
#endif

    mux_ = mux;

    return true;
}



bool
DtcpBaseConnTransport::setPeerLocation (ulong   ipAddress,
                                        ushort  port)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnTransport::setPeerLocation invoked.");
#endif

    peerIpAddress_ = ipAddress;
    peerPort_ = port;

    return true;
}



bool 
DtcpBaseConnTransport::getLastRecvTime (struct timeval & lastRecv)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnTransport::getLastRecvTime invoked.");
#endif

    lastRecv.tv_sec  = lastRecv_.tv_sec;
    lastRecv.tv_usec = lastRecv_.tv_usec;
    
    return true;
}



bool
DtcpBaseConnTransport::getLastSendTime (struct timeval & lastSend)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnTransport::getLastSendTime invoked.");
#endif

    lastSend.tv_sec  = lastSend_.tv_sec;
    lastSend.tv_usec = lastSend_.tv_usec;

    return true;
}



bool 
DtcpBaseConnTransport::getRequestId (ulong & requestId)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnTransport::getRequestId invoked.");
#endif

    requestId = requestId_;

    return true;
}



bool 
DtcpBaseConnTransport::getSendSequenceNum (ulong & sendSequenceNum)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnTransport::getSendSequenceNum invoked.");
#endif

    sendSequenceNum = sendSequenceNum_;

    return true;
}



bool 
DtcpBaseConnTransport::getRecvSequenceNum (ulong & recvSequenceNum)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnTransport::getRecvSequenceNum invoked.");
#endif

    recvSequenceNum = recvSequenceNum_;    

    return true;
}


    
bool 
DtcpBaseConnTransport::setRecvSequenceNum (ulong recvSequenceNum)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnTransport::setRecvSequenceNum invoked.");
#endif

    recvSequenceNum_ = recvSequenceNum;

    return true;
}



bool 
DtcpBaseConnTransport::close ()
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnTransport::close invoked.");
#endif

    // MRP_TEMP complete


    return true;
}



