/// Copyright (C) 2026 sonoransun — see LICENCE.txt



#include <DtcpBaseConnMux.h>
#include <DtcpBaseUdpTransport.h>
#include <DtcpBaseConnTransport.h>
#include <DtcpBaseConnAcceptor.h>
#include <DtcpBaseConnConnector.h>
#include <DtcpConnectionMap.h>
#include <DtcpConnPacket.h>
#include <DtcpPacket.h>
#include <WriteLock.h>
#include <ReadLock.h>
#include <TransportInterface.h>
#include <DataBuffer.h>
#include <Log.h>
#include <StringUtils.h>



DtcpBaseConnMux::DtcpBaseConnMux ()
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnMux constructor invoked.");
#endif

    connectionMap_         = new DtcpConnectionMap;
    connRequestIndex_      = new t_ConnRequestIndex;
    pendingAcceptIndex_    = new t_PendingAcceptIndex;
    pendingTransportIndex_ = new t_PendingTransportIndex;
    pendingAckIndex_       = new t_PendingAckRecordIndex;

    parentTransport_       = nullptr;
    acceptor_              = nullptr;

    initialized_           = false;
}



DtcpBaseConnMux::~DtcpBaseConnMux ()
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnMux destructor invoked.");
#endif

    delete connectionMap_;

    delete connRequestIndex_;

    delete pendingAcceptIndex_;

    delete pendingTransportIndex_;

    delete acceptor_;

    delete pendingAckIndex_;

}


 
bool 
DtcpBaseConnMux::initialize (TransportInterface *  parentTransport)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnMux::initialize invoked.");
#endif

    parentTransport_  = dynamic_cast<DtcpBaseUdpTransport *>(parentTransport);

    if (!parentTransport_) {
        // Invalid transport type...
        //
        Log::Error ("Invalid transport type passed to DtcpBaseConnMux::initialize.");
        return false;
    }
    bool status;
    status = createAcceptor (acceptor_);

    if (!status) {
        Log::Error ("createAcceptor failed in DtcpBaseConnMux::initialize.");
        return false;
    }
    // Mux is now ready for action
    //
    initialized_  = true;


    return true;
}



bool
DtcpBaseConnMux::getParentTransport (TransportInterface *& parentTransport)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnMux::getParentTransport invoked.");
#endif 

    if (!initialized_) {
        // must be initialized...
        Log::Error ("getParentTransport invoked before initialization of DtcpBaseConnMux.");

        return false;      
    }
    parentTransport = static_cast<TransportInterface *>(parentTransport_);


    return true;
}



bool 
DtcpBaseConnMux::exists (ulong   ipAddress,
                         ushort  port)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnMux::exists invoked.");
#endif

    bool  status;
    ReadLock  lock(connMapLock_);

    status = connectionMap_->exists (ipAddress, port);


    return status;
}



bool 
DtcpBaseConnMux::locateConnection (ulong                     ipAddress,
                                   ushort                    port,
                                   DtcpBaseConnTransport *&  connection)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnMux::locateConnection invoked.");
#endif

    bool  status;
    ReadLock  lock(connMapLock_);

    status = connectionMap_->locateConnection (ipAddress, port, connection);


    return status;
}



bool 
DtcpBaseConnMux::processPacket (StackLinkInterface * packet)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnMux::processPacket invoked.");
#endif

    if (!initialized_) {
        // must be initialized before processing packets...
        Log::Error ("processPacket invoked before initialization of DtcpBaseConnMux.");

        return false;
    }
    // Get data buffer from parent interface
    //
    DataBuffer * dataBuffer;
    parentTransport_->getDataBuffer (dataBuffer);



    DtcpPacket * dtcpPacket;

    dtcpPacket = dynamic_cast<DtcpPacket *>(packet);

    if (!dtcpPacket) {
        // This mux can only process DtcpPackets...
        Log::Error ("processPacket passed a NON DtcpPacket object.");

        return false;
    }
    // See what kind of packet this is...
    //    
    bool  status;
    t_IncomingMessage  request;

    request.dataBuffer = dataBuffer;
    request.packet     = dtcpPacket;

    dtcpPacket->getPeerLocation (request.ipAddress,
                                 request.port);

    ipPortToNetId (request.ipAddress,
                   request.port,
                   request.id);


    DtcpPacket::t_PacketType  packetType;

    packetType = dtcpPacket->getPacketType ();

#ifdef _VERBOSE
    string packetTypeString;
    DtcpPacket::packetTypeAsString (packetType, packetTypeString);

    Log::Debug ("Received packet type: '"s + packetTypeString +
                "' .");
#endif


    // dispatch handler based on packet type.
    //
    switch (packetType) {

       case DtcpPacket::t_PacketType::connRequest :
         status = handleConnRequestPacket (request);
         break;

       case DtcpPacket::t_PacketType::connOffer :
         status = handleConnOfferPacket (request);
         break;

       case DtcpPacket::t_PacketType::connAccept :
         status = handleConnAcceptPacket (request);
         break;

       case DtcpPacket::t_PacketType::connSuspend :
         status = handleConnSuspendPacket (request);
         break;

       case DtcpPacket::t_PacketType::connResume :
         status = handleConnResumePacket (request);
         break;

       case DtcpPacket::t_PacketType::connData :
         status = handleConnDataPacket (request);
         break;

       case DtcpPacket::t_PacketType::connReliableData :
         status = handleConnReliableDataPacket (request);
         break;

       case DtcpPacket::t_PacketType::connDataAck :
         status = handleConnDataAckPacket (request);
         break;

       case DtcpPacket::t_PacketType::connClose :
         status = handleConnClosePacket (request);
         break;

       case DtcpPacket::t_PacketType::poll :
         status = handlePollPacket (request);
         break;

       case DtcpPacket::t_PacketType::ack :
         status = handleAckPacket (request);
         break;

       case DtcpPacket::t_PacketType::error :
         status = handleErrorPacket (request);
         break;

       case DtcpPacket::t_PacketType::natDiscover :
         status = handleNatDiscoverPacket (request);
         break;

       case DtcpPacket::t_PacketType::natSource :
         status = handleNatSourcePacket (request);
         break;

       case DtcpPacket::t_PacketType::txnData :
         status = handleTxnPacket (request);
         break;



      default:
        Log::Error ("Invalid packetType in switch dispatch at DtcpBaseConnMux::processPacket.");
        status = false;
        break;
    }

    return status;
}



bool 
DtcpBaseConnMux::handleConnRequestPacket (t_IncomingMessage & request)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnMux::handleConnRequestPacket invoked.");
#endif


    // make sure we dont already have a connection at this location.
    //
    bool  status;

    // scope lock
    {
        ReadLock  lock(connMapLock_);

        status = connectionMap_->exists (request.ipAddress,
                                         request.port);
    }

    if (status) {
        // duplicate request, ignore.
#ifdef _VERBOSE        
        Log::Debug ("Duplicate request (conn exists) ! Ignoring.");
#endif
        return true;
    }
    // Verify that this connection is not in setup state
    //
    t_PendingAckRecord *  ackRecord;

    status = getPendingAckRecord (request.id, ackRecord);

    if (status) {
#ifdef _VERBOSE
        Log::Debug ("Duplicate request (in progress) ! Ignoring.");
#endif
        return true;
    }
    // Parse connection packet information (verification)
    //
    DtcpConnPacket *  dtcpConnPacket;
    dtcpConnPacket =  new DtcpConnPacket ();

    dtcpConnPacket->setPacketType (request.packet->getPacketType ());        
    status = dtcpConnPacket->readData (request.dataBuffer);

    if (!status) {
        // Invalid packet type?
#ifdef _VERBOSE
        Log::Debug ("readData failed for DtcpConnPacket in DtcpBaseConnMux::handleConnRequestPacket.");
#endif

        return false;
}
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnMux - Sending packet to acceptor.");
#endif

    status = acceptor_->acceptConnection (request.packet);

    if (!status) {
        // acceptor rejected request...
        return true;
    }
    // Connection request accepted, setup for transport...
    //
    dtcpConnPacket->setPeerLocation (request.ipAddress, request.port);
    dtcpConnPacket->unsetParent ();

    status = createConnectionAcceptEntry (dtcpConnPacket,
                                          request.ipAddress,
                                          request.port);

    if (!status) {
        // This shouldnt happen
        Log::Error ("createConnectionEntry failed in DtcpBaseConnMux::handleConnRequestPacket.");

        return false;
    }
    status = sendConnectionOffer (dtcpConnPacket);

    if (!status) {
        // UDP Transport error?
#ifdef _VERBOSE
        Log::Debug ("sendConnectionOffer failed in DtcpBaseConnMux::handleConnRequestPacket.");
#endif

        return false;
    }
    delete dtcpConnPacket;


    return true;
}



bool 
DtcpBaseConnMux::handleConnOfferPacket (t_IncomingMessage & request)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnMux::handleConnOfferPacket invoked.");
#endif


    // Verify pending acknowledgement record state
    //
    bool status;
    t_PendingAckRecord *  ackRecord;

    status = getPendingAckRecord (request.id, ackRecord);

    if (!status) {
#ifdef _VERBOSE
        Log::Debug ("Unable to locate pending ack record for request! Ignoring.");
#endif
        return true;
    }
    if (ackRecord->msgType != DtcpPacket::t_PacketType::connRequest) {
#ifdef _VERBOSE
        Log::Debug ("Request for this ID is not in correct state. Ignoring.");
#endif
        return true;
    }
    // cleanup pending ack record; reliable transfer complete
    //
    removePendingAckRecord (ackRecord);


    // Parse connection packet information 
    //
    DtcpConnPacket *  dtcpConnPacket;
    dtcpConnPacket =  new DtcpConnPacket ();
        
    dtcpConnPacket->setPacketType (request.packet->getPacketType ());        
    status = dtcpConnPacket->readData (request.dataBuffer);

    if (!status) {
        // Invalid packet data...
#ifdef _VERBOSE
        Log::Debug ("readData failed for DtcpConnPacket in DtcpBaseConnMux::handleConnOfferPacket.");
#endif

        return false;
    }
    // Lookup connection request for this offer.
    //
    ulong peerId;
    ulong myId;
    dtcpConnPacket->getPeerId (peerId);
    dtcpConnPacket->getMyId (myId);


#ifdef _VERBOSE
    Log::Debug ("CONNECTION OFFER RECEIVED with values: "s +
                "\nPeer ID: "s + std::to_string (peerId) +
                "\nMy ID: "s + std::to_string (myId));
#endif


    t_ConnRequestRecord *  connRequestRecord = nullptr;

    // scope lock
    {
        ReadLock  lock(indexLock_);

        auto iter = connRequestIndex_->find (peerId);

        if (iter != connRequestIndex_->end ()) {
            connRequestRecord = (*iter).second;
        }
    }

    if (!connRequestRecord) {
#ifdef _VERBOSE
        Log::Debug ("Received connection offer from unknown peer.");
#endif

        return false;
    }
    // Update transport information..
    //
    ConnectorInterface * connector;
    DtcpBaseConnTransport * connTransport;

    connector = connRequestRecord->requestor;
    connTransport = dynamic_cast<DtcpBaseConnTransport *>(connRequestRecord->transport);

    if (!connTransport) {
        Log::Error ("Invalid transport type in DtcpBaseConnMux::handleConnRequestPacket.");

        return false;
    }
    connTransport->setMyId (myId);

    // scope lock
    {
        WriteLock  lock(indexLock_);

        connRequestIndex_->erase (peerId);
    }

    delete connRequestRecord;


    // Index this connection...
    //
    {
        WriteLock  lock(connMapLock_);

        status = connectionMap_->indexConnection (peerId, connTransport);
    }

    if (!status) {
        Log::Error ("Unable to index transport in DtcpBaseConnMux::handleConnOfferPacket.");

        return false;
    }
    // Send connection accept packet.
    //
    status = sendConnectionAccept (connTransport);

    if (!status) {
        // UDP Transport error?
#ifdef _VERBOSE
        Log::Debug ("sendConnectionAccept failed in DtcpBaseConnMux::handleConnRequestPacket.");
#endif

        return false;
    }
    // Add pending transport record once we receive accept packet.
    //
    t_PendingTransportRecord *  transportRecord;
    transportRecord = new t_PendingTransportRecord;

    transportRecord->netId     = request.id;
    transportRecord->requestor = connector;
    transportRecord->transport = connTransport;

    // scope lock
    {
        WriteLock  lock(indexLock_);

        pendingTransportIndex_->emplace (request.id, transportRecord);
    }


    return status;
}



bool 
DtcpBaseConnMux::handleConnAcceptPacket (t_IncomingMessage & request)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnMux::handleConnAcceptPacket invoked.");
#endif


    // Verify pending acknowledgement record state
    //
    bool status;
    t_PendingAckRecord *  ackRecord;

    status = getPendingAckRecord (request.id, ackRecord);

    if (!status) {
#ifdef _VERBOSE
        Log::Debug ("Unable to locate pending ack record for request. Checking retry logic...");
#endif
        status = checkAcceptAckRetry (request);

        return status;
    }

    if (ackRecord->msgType != DtcpPacket::t_PacketType::connOffer) {
#ifdef _VERBOSE
        Log::Debug ("Request for this ID is not in correct state. Ignoring.");
#endif
        return true;
    }
    // cleanup pending ack record; reliable transfer complete
    //
    removePendingAckRecord (ackRecord);


    // Parse connection packet information 
    //
    DtcpConnPacket *  dtcpConnPacket;
    dtcpConnPacket =  new DtcpConnPacket ();
        
    dtcpConnPacket->setPacketType (request.packet->getPacketType ());        
    status = dtcpConnPacket->readData (request.dataBuffer);

    if (!status) {
        // Invalid packet data...
#ifdef _VERBOSE
        Log::Debug ("readData failed for DtcpConnPacket in DtcpBaseConnMux::handleConnAcceptPacket.");
#endif

        return false;
    }
    // Lookup pending accept record for this peer.
    //
    ulong peerId;
    dtcpConnPacket->getPeerId (peerId);

    t_PendingAcceptRecord *  acceptRecord = nullptr;

    // scope lock
    {
        ReadLock  lock(indexLock_);

        auto iter = pendingAcceptIndex_->find (peerId);

        if (iter != pendingAcceptIndex_->end ()) {
            acceptRecord = (*iter).second;
        }
    }

    if (!acceptRecord) {
#ifdef _VERBOSE
        Log::Debug ("Received connection accept from unknown peer.");
#endif

        return false;
    }
    // Have acceptor create transport...
    //
    TransportInterface * transport;
    status = acceptor_->createTransport (transport);

    if (!status) {
        Log::Error ("Acceptor createTransport failed in "
                             "DtcpBaseConnMux::handleConnAcceptPacket.");

        // scope lock
        {
            WriteLock  lock(connMapLock_);
            connectionMap_->removePendingConnection (peerId);
        }

        return false;
    }
    // Verify correct transport type..
    //
    DtcpBaseConnTransport * connTransport;
    connTransport = dynamic_cast<DtcpBaseConnTransport *>(transport);

    if (!connTransport) {
        Log::Error ("Invalid transport from createTransport in "
                             "DtcpBaseConnMux::handleConnAcceptPacket.");

        // scope lock
        {
            WriteLock  lock(connMapLock_);
            connectionMap_->removePendingConnection (peerId);
        }

        return false;
    }
    // Initialize transport
    //
    connTransport->setParent (parentTransport_);
    connTransport->setMux (this);
    connTransport->setMyId (acceptRecord->myId);
    connTransport->setPeerId (peerId);
    connTransport->setPeerLocation (request.ipAddress, request.port);


    // Index new transport...
    //
    {
        WriteLock  lock(connMapLock_);
        status = connectionMap_->indexConnection (peerId, connTransport);
    }

    if (!status) {
        Log::Error ("Unable to index transport in DtcpBaseConnMux::handleConnAcceptPacket.");

        // scope lock
        {
            WriteLock  lock(connMapLock_);
            connectionMap_->removePendingConnection (peerId);
        }

        return false;
    }
    // cleanup request record now that processing complete
    //
    {
        WriteLock lock(indexLock_);
        connRequestIndex_->erase (peerId);
    }

    delete acceptRecord;


    // Send ACK packet for this request
    //
    status = sendAck (connTransport);

    if (!status) {
        Log::Error ("Sending ack packet failed, ignoring..");
        return false;
    }

    
    // Send transport to acceptor...
    //
    status = acceptor_->receiveTransport (connTransport);


    return status;
}



bool 
DtcpBaseConnMux::handleConnSuspendPacket (t_IncomingMessage & request)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnMux::handleConnSuspendPacket invoked.");
#endif


    return true;
}



bool 
DtcpBaseConnMux::handleConnResumePacket (t_IncomingMessage & request)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnMux::handleConnResumePacket invoked.");
#endif


    return true;
}



bool 
DtcpBaseConnMux::handleConnDataPacket (t_IncomingMessage & request)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnMux::handleConnDataPacket invoked.");
#endif


    // Parse connection packet information 
    //
    bool  status;
    DtcpConnPacket *  dtcpConnPacket;
    dtcpConnPacket =  new DtcpConnPacket ();

    dtcpConnPacket->setPacketType (request.packet->getPacketType ());        
    status = dtcpConnPacket->readData (request.dataBuffer);

    if (!status) {
        // Invalid packet data...
        return false;
    }
    // Locate transport for this connection
    //
    ulong peerId;
    dtcpConnPacket->getPeerId (peerId);

    DtcpBaseConnTransport *  transport;

    // scope lock
    {
        WriteLock  lock(connMapLock_);
        status = connectionMap_->locateConnection (peerId, transport);
    }

    if (!status) {
        // Invalid peer ID?
        return false;
    }
    // Send packet to transport...
    //
    status = transport->processPacket (dtcpConnPacket);


    return status;
}



bool 
DtcpBaseConnMux::handleConnReliableDataPacket (t_IncomingMessage & request)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnMux::handleConnReliableDataPacket invoked.");
#endif

    // Parse connection packet information 
    //
    bool  status;
    DtcpConnPacket *  dtcpConnPacket;
    dtcpConnPacket =  new DtcpConnPacket ();

    dtcpConnPacket->setPacketType (request.packet->getPacketType ());        
    status = dtcpConnPacket->readData (request.dataBuffer);

    if (!status) {
        // Invalid packet data...
        return false;
    }
    // Locate transport for this connection
    //
    ulong peerId;
    dtcpConnPacket->getPeerId (peerId);

    DtcpBaseConnTransport *  transport;

    // scope lock
    {
        WriteLock  lock(connMapLock_);
        status = connectionMap_->locateConnection (peerId, transport);
    }

    if (!status) {
        // Invalid peer ID?
        return false;
    }
    // First, verify that this is not a repeat packet.  If it is, 
    // we should resend the dataAck packet, but not pass the data
    // up to the transport.
    //
    ulong  currSequenceNum;
    ulong  recvSequenceNum;

    dtcpConnPacket->getSequenceNum (currSequenceNum);
    transport->getRecvSequenceNum (recvSequenceNum);

    if (recvSequenceNum == currSequenceNum) {
        // duplicate data packet, our ACK must have been lost.
        // Resend ack packet.
        //
        status = sendDataAck (transport);

        return status;
    }

    // Update current received sequence number, and send ACK for this data.
    //
    transport->setRecvSequenceNum (currSequenceNum);

    sendDataAck (transport);


    // Send data packet to transport...
    //
    status = transport->processPacket (dtcpConnPacket);


    return true;
}



bool 
DtcpBaseConnMux::handleConnDataAckPacket (t_IncomingMessage & request)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnMux::handleConnDataAckPacket invoked.");
#endif

    // Parse connection packet information 
    //
    bool  status;
    DtcpConnPacket *  dtcpConnPacket;
    dtcpConnPacket =  new DtcpConnPacket ();

    dtcpConnPacket->setPacketType (request.packet->getPacketType ());        
    status = dtcpConnPacket->readData (request.dataBuffer);

    if (!status) {
        // Invalid packet data...
        return false;
    }
    // Locate transport for this connection
    //
    ulong peerId;
    dtcpConnPacket->getPeerId (peerId);

    DtcpBaseConnTransport *  transport;

    // scope lock
    {
        WriteLock  lock(connMapLock_);
        status = connectionMap_->locateConnection (peerId, transport);
    }

    if (!status) {
        // Invalid peer ID?
        return false;
    }
    // Verify that we have a request ID associated with this
    // data acknowledgement...
    //
    if (!transport->pendingAck ()) {
        // We are not waiting on any aknowledgement.  Bad packet?
        // MRP_TEMP - DoS protect
        //
        return true;
    }
    ulong  requestId;
    transport->getRequestId (requestId);
    status = parentTransport_->reliableRequestComplete (requestId);

    if (!status) {
        Log::Error ("request complete call to parentTransport failed in"
                             " DtcpBaseConnMux::handleConnDataAckPacket.");
        return false;
    }

    status = transport->handleSendReceived (requestId);


    return status;
}



bool 
DtcpBaseConnMux::handleConnClosePacket (t_IncomingMessage & request)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnMux::handleConnClosePacket invoked.");
#endif


    return true;
}



bool 
DtcpBaseConnMux::handlePollPacket (t_IncomingMessage & request)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnMux::handlePollPacket invoked.");
#endif


    return true;
}



bool 
DtcpBaseConnMux::handleAckPacket (t_IncomingMessage & request)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnMux::handleAckPacket invoked.");
#endif

    // Verify pending acknowledgement record state
    //
    bool status;
    t_PendingAckRecord *  ackRecord;

    status = getPendingAckRecord (request.id, ackRecord);

    if (!status) {
#ifdef _VERBOSE
        Log::Debug ("Unable to locate pending ack record for request! Ignoring.");
#endif
        return true;
    }
    // Determine what previous packet was sent, and handle this
    // ack accordingly...
    //
    bool validState = false;

#ifdef _VERBOSE
    string packetTypeString;
    DtcpPacket::packetTypeAsString (ackRecord->msgType, packetTypeString);

    Log::Debug ("Checking sub handler for previous SENT packet type: "s +
                packetTypeString);
#endif

    switch (ackRecord->msgType) {

        case DtcpPacket::t_PacketType::connAccept :
               validState = true;
               status     = pendingConnAcceptComplete (request, ackRecord);
               break;

        case DtcpPacket::t_PacketType::connClose :
               validState = true;
               status     = pendingConnCloseComplete (request, ackRecord);
               break;

        case DtcpPacket::t_PacketType::connSuspend :
               validState = true;
               status     = pendingConnSuspendComplete (request, ackRecord);
               break;

        case DtcpPacket::t_PacketType::poll :
               validState = true;
               status     = pendingPollComplete (request, ackRecord);
               break;

      default:
          // unhandled pending type, invalid packet?
          break;
    };

    if (!validState) {
#ifdef _VERBOSE
        Log::Debug ("Invalid pending state for this request! Ignoring.");
#endif
        return true;
    }
    return status;
}



bool 
DtcpBaseConnMux::handleErrorPacket (t_IncomingMessage & request)
{   
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnMux::handleErrorPacket invoked.");
#endif

    // We may have a pending state for this error packet.  See what type
    // of error occured, and cleanup any state if nescessary.
    //
    bool status;
    t_PendingAckRecord *  ackRecord;

    status = getPendingAckRecord (request.id, ackRecord);

    if (!status) {
#ifdef _VERBOSE
        Log::Debug ("Unable to locate pending ack record for request! Ignoring.");
#endif
        return true;
    }
    // MRP_TEMP cleanup state


    return true;
}



bool
DtcpBaseConnMux::handleNatDiscoverPacket (t_IncomingMessage & request)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnMux::handleNatDiscoverPacket invoked.");
#endif


    return true;
}



bool 
DtcpBaseConnMux::handleNatSourcePacket (t_IncomingMessage & request)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnMux::handleNatSourcePacket invoked.");
#endif

    // Verify pending acknowledgement record state
    //
    bool status;
    t_PendingAckRecord *  ackRecord;

    status = getPendingAckRecord (request.id, ackRecord);

    if (!status) {
#ifdef _VERBOSE
        Log::Debug ("Unable to locate pending ack record for request! Ignoring.");
#endif
        return true;
    }
    if (ackRecord->msgType != DtcpPacket::t_PacketType::natDiscover) {
#ifdef _VERBOSE
        Log::Debug ("Request for this ID is not in correct state. Ignoring.");
#endif
        return true;
    }
    return true;
}



bool 
DtcpBaseConnMux::handleTxnPacket (t_IncomingMessage & request)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnMux::handleTxnPacket invoked.");
#endif


    return true;
}



bool 
DtcpBaseConnMux::checkAcceptAckRetry (t_IncomingMessage & request)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnMux::checkAcceptAckRetry invoked.");
#endif


    // Parse connection packet information 
    //
    bool  status;
    DtcpConnPacket *  dtcpConnPacket;
    dtcpConnPacket =  new DtcpConnPacket ();
        
    dtcpConnPacket->setPacketType (request.packet->getPacketType ());        
    status = dtcpConnPacket->readData (request.dataBuffer);

    if (!status) {
        // Invalid packet data...
#ifdef _VERBOSE
        Log::Debug ("readData failed for DtcpConnPacket in DtcpBaseConnMux::checkAcceptAckRetry.");
#endif

        return false;
    }
    // Try to locate transport for this connection.  If it is found, we need to
    // resend an ack packet...
// MRP_TEMP - add code to prevent DOS attacks.
    //
    ulong peerId;
    dtcpConnPacket->getPeerId (peerId);

    DtcpBaseConnTransport *  transport;

    // scope lock
    {
        WriteLock  lock(connMapLock_);
        status = connectionMap_->locateConnection (peerId, transport);
    }

    if (!status) {
        // Invalid peer ID?
#ifdef _VERBOSE
        Log::Debug ("Invalid peer ID.  Assuming bad resent packet.");
#endif
    
        return true;
    }
    // resend ack packet
    //
    status = sendAck (transport);


    return status;
}



bool 
DtcpBaseConnMux::pendingConnAcceptComplete (t_IncomingMessage &  request,
                                            t_PendingAckRecord * ackRecord)
{   
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnMux::pendingConnAcceptComplete invoked.");
#endif

    // delete pending ack information.. no longer needed.
    //
    t_NetId   netId = ackRecord->netId;

    bool status;
    status = removePendingAckRecord (ackRecord);

    if (!status) {
        // This shouldnt happen...
        Log::Error ("Error removing pending ack record in"
                             " DtcpBaseConnMux::pendingConnAcceptComplete.");

        // continue on anyway, asume that nothing critical occurred.
    }

    // Locate pending transport record for this request and return transport to requestor.
    //
    t_PendingTransportRecord *  transportRecord = nullptr;

    // scope lock
    {
        WriteLock  lock(indexLock_);

        auto iter = pendingTransportIndex_->find (netId);

        if (iter != pendingTransportIndex_->end ()) {
            transportRecord = (*iter).second;
            pendingTransportIndex_->erase (netId);
        }
    }

    if (!transportRecord) {
        Log::Error ("Error locating pending transport record for request in"
                             " DtcpBaseConnMux::pendingConnAcceptComplete");

        return false;
    }
    ConnectorInterface * connector = transportRecord->requestor;
    TransportInterface * transport = transportRecord->transport;
    delete transportRecord;

    status = connector->receiveTransport (transport);


    return status;
}



bool 
DtcpBaseConnMux::pendingConnCloseComplete (t_IncomingMessage &  request,
                                           t_PendingAckRecord * ackRecord)
{   
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnMux::pendingConnCloseComplete invoked.");
#endif


    // delete pending ack information.. no longer needed.
    //
    t_NetId   netId = ackRecord->netId;

    bool status;
    status = removePendingAckRecord (ackRecord);

    if (!status) {
        // This shouldnt happen...
        Log::Error ("Error removing pending ack record in"
                             " DtcpBaseConnMux::pendingConnCloseComplete.");

        // continue on anyway, asume that nothing critical occurred.
    }

// MRP_TEMP - silence compiler warning about variable
netId = 0;


    return true;
}



bool
DtcpBaseConnMux::pendingConnSuspendComplete (t_IncomingMessage &  request,
                                             t_PendingAckRecord * ackRecord)
{   
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnMux::pendingConnSuspendComplete invoked.");
#endif


    // delete pending ack information.. no longer needed.
    //
    t_NetId   netId = ackRecord->netId;

    bool status;
    status = removePendingAckRecord (ackRecord);

    if (!status) {
        // This shouldnt happen...
        Log::Error ("Error removing pending ack record in"
                             " DtcpBaseConnMux::pendingConnSuspendComplete.");

        // continue on anyway, asume that nothing critical occurred.
    }

// MRP_TEMP - silence compiler warning about variable
netId = 0;


    return true;
}



bool
DtcpBaseConnMux::pendingPollComplete (t_IncomingMessage &  request,
                                      t_PendingAckRecord * ackRecord)
{   
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnMux::pendingPollComplete invoked.");
#endif


    // delete pending ack information.. no longer needed.
    //
    t_NetId   netId = ackRecord->netId;

    bool status;
    status = removePendingAckRecord (ackRecord);

    if (!status) {
        // This shouldnt happen...
        Log::Error ("Error removing pending ack record in"
                             " DtcpBaseConnMux::pendingPollComplete.");

        // continue on anyway, asume that nothing critical occurred.
    }

// MRP_TEMP - silence compiler warning about variable
netId = 0;


    return true;
}



bool
DtcpBaseConnMux::requestTransport (ConnectorInterface * requestor,
                                   TransportInterface * transport)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnMux::requestTransport invoked.");
#endif

    if (!initialized_) {
        // must be initialized before processing packets...
        return false;
    }
    // setup transport...
    //
    bool status;
    DtcpBaseConnTransport *  connTransport;

    connTransport =  dynamic_cast<DtcpBaseConnTransport *>(transport);

    if (!connTransport) {
        // Have to have a DtcpBaseConnTransport type.
        Log::Error ("Invalid transport type passed to DtcpBaseConnMux::requestTransport.");
        
        return false;
    }
    status = createConnectionRequestEntry (requestor, connTransport);

    if (!status) {
        // This shouldnt happen
        Log::Error ("createConnectionEntry failed in DtcpBaseConnMux::requestTransport.");

        return false;
    }
    status = sendConnectionRequest (connTransport);

    if (!status) {
        // UDP Transport error?
#ifdef _VERBOSE
        Log::Debug ("sendConnectionRequest failed in DtcpBaseConnMux::requestTransport.");
#endif

        return false;
    }
    return true;
}



bool 
DtcpBaseConnMux::createConnectionAcceptEntry (DtcpConnPacket * packet,
                                              ulong            ipAddress,
                                              ushort           port)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnMux::createConnectionAcceptEntry invoked.");
#endif

    // Allocate a connection ID for this connection.
    //
    bool  status;
    ulong peerId;

    // scope lock
    {
        WriteLock  lock(connMapLock_);

        status = connectionMap_->createConnection (ipAddress,
                                                   port,
                                                   peerId); 
    }

    if (!status) {
        Log::Error ("Unable to create connection in DtcpBaseConnMux::createConnectionAcceptEntry.");
        return false;
    }
    // Create pending accept record for this request.
    //
    t_PendingAcceptRecord *  pendingAcceptRecord;
    pendingAcceptRecord = new t_PendingAcceptRecord;

    ulong myId;
    packet->getMyId (myId);

    packet->setPeerId (peerId);

    pendingAcceptRecord->myId           = myId;
    pendingAcceptRecord->tempPeerId     = peerId;


    // Insert into pending accept index
    //
    {
        WriteLock  lock(indexLock_);
        pendingAcceptIndex_->emplace (peerId, pendingAcceptRecord);
    }

#ifdef _VERBOSE
    Log::Debug ("CONNECTION ACCEPT CREATED with values: "s +
                "\nPeer ID: "s + std::to_string (peerId) +
                "\nMy ID: "s + std::to_string (myId));
#endif


    return true;
}



bool
DtcpBaseConnMux::createConnectionRequestEntry (ConnectorInterface *    requestor,
                                               DtcpBaseConnTransport * transport)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnMux::createConnectionRequestEntry invoked.");
#endif

    // Allocate a connection ID for this requested connection
    //
    ulong   destIpAddress;
    ushort  destPort;
    bool    status;
    ulong   peerId;

    transport->getPeerLocation (destIpAddress, destPort);
   
    // scope lock
    {
        WriteLock  lock(connMapLock_);
 
        status = connectionMap_->createConnection (destIpAddress,
                                                   destPort,
                                                   peerId);
    }

    if (!status) {
        Log::Error ("Unable to create connection in DtcpBaseConnMux::createConnectionRequestEntry.");
        return false;
    }
    transport->setPeerId (peerId);


    // Create pending accept record for this request.
    //
    t_ConnRequestRecord *  connRequestRecord;
    connRequestRecord = new t_ConnRequestRecord;

    connRequestRecord->requestor   = requestor;
    connRequestRecord->transport   = static_cast<TransportInterface *>(transport);
    connRequestRecord->tempPeerId  = peerId;


    // Insert into connection request index
    //
    {
        WriteLock  lock(indexLock_);
        connRequestIndex_->emplace (peerId, connRequestRecord);
    }

#ifdef _VERBOSE
    Log::Debug ("CONNECTION REQUEST CREATED with values: "s +
                "\nPeer ID: "s + std::to_string (peerId));
#endif


    return true;
}



bool
DtcpBaseConnMux::sendConnectionRequest (DtcpBaseConnTransport * transport)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnMux::sendConnectionRequest invoked.");
#endif

    // Set packet links for connection request.
    //
    bool   status;
    ulong  peerId;
    ulong  ipAddress;
    ushort port;

    DtcpConnPacket * packet = new DtcpConnPacket ();
    DtcpPacket * dtcpPacket = new DtcpPacket();

    transport->getPeerId (peerId);
    transport->getPeerLocation (ipAddress, port);

    dtcpPacket->setParent (packet);

    packet->setPeerLocation (ipAddress, port);
    packet->setPeerId (peerId);
    packet->setPacketType (DtcpPacket::t_PacketType::connRequest);

    ulong requestId;

    status = parentTransport_->sendReliablePacket (dtcpPacket,
                                                   (DtcpBaseConnTransport *)0, // no transport for ConnMux,
                                                   requestId);

    delete packet;
    delete dtcpPacket;

    if (!status) {
        // Error sending on socket???
        //
        Log::Error ("Error sending reliable packet in"
                             " DtcpBaseConnMux::sendConnectionRequest.");

        return false;
    }
    // Index this pending request...
    //
    t_PendingAckRecord * pendingRequest;
    pendingRequest =  new t_PendingAckRecord;

    ipPortToNetId (ipAddress,
                   port,
                   pendingRequest->netId);

    pendingRequest->msgType  = DtcpPacket::t_PacketType::connRequest;
    pendingRequest->requestId = requestId;

    // scope lock
    {
        WriteLock  lock(pendingAckLock_);

        pendingAckIndex_->emplace (pendingRequest->netId, pendingRequest);
    }
    

    return true;
}



bool 
DtcpBaseConnMux::sendConnectionOffer (DtcpConnPacket * packet)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnMux::sendConnectionOffer invoked.");
#endif

    // Set packet links for connection offer.
    //
    bool status;
    DtcpPacket * dtcpPacket = new DtcpPacket();
    dtcpPacket->setParent (packet);

    packet->setPacketType (DtcpPacket::t_PacketType::connOffer);

    ulong  ipAddress;
    ushort port;

    packet->getPeerLocation (ipAddress, port);


    ulong  requestId;

    status = parentTransport_->sendReliablePacket (dtcpPacket,
                                                   0, // no requestor for ConnMux
                                                   requestId); 

    delete dtcpPacket;

    if (!status) {
        // Error sending on socket???
        //
        Log::Error ("Error sending reliable packet in"
                             " DtcpBaseConnMux::sendConnectionOffer.");

        return false;
    }
    // create pending record and assign msg type
    //
    t_PendingAckRecord * pendingRequest;
    pendingRequest = new t_PendingAckRecord;
    pendingRequest->msgType   = DtcpPacket::t_PacketType::connOffer;
    pendingRequest->requestId = requestId;

    ipPortToNetId (ipAddress,
                   port,
                   pendingRequest->netId);

    // scope lock
    {
        WriteLock  lock(pendingAckLock_);

        pendingAckIndex_->emplace (pendingRequest->netId, pendingRequest);
    }


    return true;
}



bool 
DtcpBaseConnMux::sendConnectionAccept (DtcpBaseConnTransport * transport)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnMux::sendConnectionAccept invoked.");
#endif

    // Set packet links for connection accept.
    //
    bool   status;
    ulong  myId;
    ulong  ipAddress;
    ushort port;

    DtcpConnPacket * packet = new DtcpConnPacket ();
    DtcpPacket * dtcpPacket = new DtcpPacket();

    transport->getMyId (myId);
    transport->getPeerLocation (ipAddress, port);

    dtcpPacket->setParent (packet);

    packet->setPeerLocation (ipAddress, port);
    packet->setMyId (myId);
    packet->setPacketType (DtcpPacket::t_PacketType::connAccept);

    ulong  requestId;

    status = parentTransport_->sendReliablePacket (dtcpPacket,
                                                   0, // no requestor for ConnMux
                                                   requestId); 

    delete dtcpPacket;
    delete packet;

    if (!status) {
        // Error sending on socket???
        //
        Log::Error ("Error sending reliable packet in"
                             " DtcpBaseConnMux::sendConnectionAccept.");

        return false;
    }
    // create pending record and assign msg type
    //
    t_PendingAckRecord * pendingRequest;
    pendingRequest = new t_PendingAckRecord;
    pendingRequest->msgType   = DtcpPacket::t_PacketType::connAccept;
    pendingRequest->requestId = requestId;

    ipPortToNetId (ipAddress,
                   port,
                   pendingRequest->netId);

    // scope lock
    {
        WriteLock  lock(pendingAckLock_);

        pendingAckIndex_->emplace (pendingRequest->netId, pendingRequest);
    }


    return true;
}



bool 
DtcpBaseConnMux::sendAck (DtcpBaseConnTransport * transport)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnMux::sendAck invoked.");
#endif

    // Set packet links for ack message
    //
    bool   status;
    ulong  myId;
    ulong  ipAddress;
    ushort port;

    DtcpConnPacket * packet = new DtcpConnPacket ();
    DtcpPacket * dtcpPacket = new DtcpPacket();

    transport->getMyId (myId);
    transport->getPeerLocation (ipAddress, port);

    dtcpPacket->setParent (packet);

    packet->setPeerLocation (ipAddress, port);
    packet->setMyId (myId);
    packet->setPacketType (DtcpPacket::t_PacketType::ack);


    status = parentTransport_->sendPacket (dtcpPacket);

    delete packet;
    delete dtcpPacket;

    if (!status) {
        Log::Error ("Send packet failed in DtcpBaseConnMux::sendAck!");
        return false;
    }
    return true;
}



bool 
DtcpBaseConnMux::sendDataAck (DtcpBaseConnTransport * transport)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnMux::sendDataAck invoked.");
#endif

    // Set packet links for ack message
    //
    bool   status;
    ulong  myId;
    ulong  ipAddress;
    ushort port;
    ulong  sequenceNum;

    DtcpConnPacket * packet = new DtcpConnPacket ();
    DtcpPacket * dtcpPacket = new DtcpPacket();

    transport->getMyId (myId);
    transport->getPeerLocation (ipAddress, port);
    transport->getRecvSequenceNum (sequenceNum);

    dtcpPacket->setParent (packet);

    packet->setPeerLocation (ipAddress, port);
    packet->setMyId (myId);
    packet->setPacketType (DtcpPacket::t_PacketType::connDataAck);
    packet->setSequenceNum (sequenceNum);


    status = parentTransport_->sendPacket (dtcpPacket);

    delete packet;
    delete dtcpPacket;

    if (!status) {
        Log::Error ("Send packet failed in DtcpBaseConnMux::sendDataAck!");
        return false;
    }
    return true;
}



bool 
DtcpBaseConnMux::getPendingAckRecord (const t_NetId &        id,
                                      t_PendingAckRecord *&  record)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnMux::getPendingAckRecord invoked.");
#endif

    ReadLock  lock(pendingAckLock_);

    auto iter = pendingAckIndex_->find (id);

    if (iter == pendingAckIndex_->end ()) {
        return false;
    }
    record = (*iter).second;


    return true;
}



bool 
DtcpBaseConnMux::removePendingAckRecord (t_PendingAckRecord *  record)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnMux::removePendingAckRecord invoked.");
#endif

    if (!record) {
        return false;
    }
    // Remove record from pendingIndex and cleanup state in pending ack map
    //
    ReadLock  lock(pendingAckLock_);

    pendingAckIndex_->erase (record->netId);

    bool status;
    status = parentTransport_->reliableRequestComplete (record->requestId);

    if (!status) {
        Log::Error ("request complete call to parentTransport failed in"
                             " DtcpBaseConnMux::removePendingAckRecord.");
        return false;
    }

    delete record;


    return true;
}



bool 
DtcpBaseConnMux::acknowledgeTransfer (DtcpBaseConnTransport * transport)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpBaseConnMux::removePendingAckRecord invoked.");
#endif

    // Verify that we have a request ID associated with a reliable transfer
    //
    if (!transport->pendingAck ()) {
        // We are not waiting on any aknowledgement.  User error?
        //
        Log::Error ("DtcpBaseConnMux::acknowledgeTransfer invoked, but no reliable data "
                             "pending acknowledgement!");
          
        return false;
    }
    bool   status;
    ulong  requestId;
    transport->getRequestId (requestId);

    status = parentTransport_->reliableRequestComplete (requestId);

    if (!status) {
        Log::Error ("request complete call to parentTransport failed in "
                             "DtcpBaseConnMux::acknowledgeTransfer!");

        return false;
    }
    return true;
}



