/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AlpineDtcpConnTransport.h>
#include <AlpineStack.h>
#include <AlpinePacket.h>
#include <AlpineQueryPacket.h>
#include <AlpinePeerPacket.h>
#include <AlpineProxyPacket.h>
#include <AlpineQueryMgr.h>
#include <DataBuffer.h>
#include <DataBlock.h>
#include <Log.h>
#include <StringUtils.h>


AlpineDtcpConnTransport::AlpineDtcpConnTransport ()
{
#ifdef _VERBOSE
    Log::Debug ("AlpineDtcpConnTransport constructor invoked.");
#endif

    pendingIndex_ = nullptr;
}


    
AlpineDtcpConnTransport::~AlpineDtcpConnTransport ()
{
#ifdef _VERBOSE
    Log::Debug ("AlpineDtcpConnTransport destructor invoked.");
#endif

    cleanUp ();
}



bool 
AlpineDtcpConnTransport::handleData (const byte * data,
                                     uint         dataLength)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineDtcpConnTransport::handleData invoked.");
#endif

    // Attempt to marshal an AlpinePacket from received data
    //
    DataBuffer * buffer = new DataBuffer (data, dataLength);
    buffer->readReset ();

    bool  status;
    AlpinePacket  packet;

    status = packet.readData (buffer);

    if (!status) {
#ifdef _VERBOSE
        Log::Debug ("Invalid packet data received in AlpineDtcpConnTransport::handleData.");
#endif
        delete buffer;
        return true;
    }
    // Determine what kind packet this is, and process accordingly.
    //
    AlpinePacket::t_PacketType  packetType;
    packetType = packet.getPacketType ();

    switch (packetType) {

      // Query Packet
      //
      case AlpinePacket::t_PacketType::queryDiscover  :
      case AlpinePacket::t_PacketType::queryOffer  :
      case AlpinePacket::t_PacketType::queryRequest  :
      case AlpinePacket::t_PacketType::queryReply  :

          status = processAlpineQueryPacket (&packet, buffer);
          break;


      // Peer Packet
      //
      case AlpinePacket::t_PacketType::peerListRequest  :
      case AlpinePacket::t_PacketType::peerListOffer  :
      case AlpinePacket::t_PacketType::peerListGet  :
      case AlpinePacket::t_PacketType::peerListData  :

          status = processAlpinePeerPacket (&packet, buffer);
          break;


      // Proxy Packet
      //
      case AlpinePacket::t_PacketType::proxyRequest  :
      case AlpinePacket::t_PacketType::proxyAccepted  :
      case AlpinePacket::t_PacketType::proxyHalt  :

          status = processAlpineProxyPacket (&packet, buffer);
          break;


      // Invalid type
      case AlpinePacket::t_PacketType::none :
      default :

#ifdef _VERBOSE
          Log::Debug ("Invalid packet type received in AlpineDtcpConnTransport::handleData.");
#endif
          break;
    }

    delete buffer;


    return true;
}



bool 
AlpineDtcpConnTransport::handleConnectionClose ()
{
#ifdef _VERBOSE
    Log::Debug ("AlpineDtcpConnTransport::handleConnectionClose invoked.");
#endif

    bool   status;
    ulong  peerId;

    getTransportId (peerId);

    status = AlpineStack::handleConnectionClose (peerId);

    if (!status) {
        Log::Error ("DtcpStack handleConnectionClose failed in "
                             "AlpineDtcpConnTransport::handleConnectionClose!");
        return false;
    }
    cleanUp ();


    return true;
}



bool
AlpineDtcpConnTransport::handleSendReceived ()
{
#ifdef _VERBOSE
    Log::Debug ("AlpineDtcpConnTransport::handleSendReceived invoked.");
#endif

    // Send pending transfer if present
    //
    checkPendingRequests ();


    bool   status;
    ulong  peerId;

    getTransportId (peerId);

    status = AlpineStack::handleSendReceived (peerId, requestId_);

    if (!status) {
        Log::Error ("DtcpStack handleSendReceived failed in "
                             "AlpineDtcpConnTransport::handleSendReceived!");
        return false;
    }
    // If there are pending reliable transfers, send them now.
    //
    checkPendingRequests ();


    return true;
}



bool 
AlpineDtcpConnTransport::handleSendFailure ()
{
#ifdef _VERBOSE
    Log::Debug ("AlpineDtcpConnTransport::handleSendFailure invoked.");
#endif

    // Send pending transfer if present
    //
    checkPendingRequests ();


    bool   status;
    ulong  peerId;

    getTransportId (peerId);

    status = AlpineStack::handleSendFailure (peerId, requestId_);

    if (!status) {
        Log::Error ("DtcpStack handleSendFailure failed in "
                             "AlpineDtcpConnTransport::handleSendFailure!");
        return false;
    }
    // If there are pending reliable transfers, send them now.
    //
    checkPendingRequests ();


    return true;
}



bool 
AlpineDtcpConnTransport::handleConnectionDeactivate ()
{
#ifdef _VERBOSE
    Log::Debug ("AlpineDtcpConnTransport::handleConnectionDeactivate invoked.");
#endif

    bool   status;
    ulong  peerId;

    getTransportId (peerId);

    status = AlpineStack::handleConnectionDeactivate (peerId);

    if (!status) {
        Log::Error ("DtcpStack handleConnectionDeactivate failed in "
                             "AlpineDtcpConnTransport::handleConnectionDeactivate!");
        return false;
    }
    return true;
}



bool 
AlpineDtcpConnTransport::sendReliableData (const byte * data,
                                           uint         dataLength,
                                           ulong        requestId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineDtcpConnTransport::sendReliableData invoked.");
#endif

    // If there is already a pending transfer, we have to queue this request.
    // Check for the easy case first: nothing pending.
    //
    if (!pendingAck ()) {
        // Nothing pending, send straight away...
        //
        bool status;
        status = DtcpBaseConnTransport::sendReliableData (data,
                                                          dataLength);

        if (!status) {
            Log::Error ("sendReliableData failed in call to "
                                 "AlpineDtcpConnTransport::sendReliableData!");
            return false;
        }
        requestId_++;
        requestId = requestId_;

        return true;
    }
    // If the pending request index has not been allocated, initialized and set current ID
    // 
    if (!pendingIndex_) {
        pendingIndex_ = new t_RequestStateIndex;
        pendingIndex_->currRequestId = requestId_ + 1; // start with next ID
    }

    DataBlock *  requestData;
    requestData = new DataBlock (dataLength);
    memcpy (requestData->buffer_, data, dataLength);

    t_ReliableRequest *  newRequest;
    newRequest = new t_ReliableRequest;

    newRequest->requestId = pendingIndex_->currRequestId++;
    newRequest->data      = requestData;
    requestId             = newRequest->requestId;

    pendingIndex_->pendingRequests.push_back (newRequest);


    return true;
}



bool 
AlpineDtcpConnTransport::acknowledgeTransfer (ulong  requestId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineDtcpConnTransport::acknowledgeTransfer invoked.  Request ID: "s +
                std::to_string (requestId));
#endif

    // Verify two things before telling the DtcpBaseConnTransport to remove pending ACK
    // state.  First, verify that we have soemthing pending, and second verify that the
    // requestID being acknowledged is what is currently in progress.
    //
    if ( !pendingAck () || (requestId != requestId_) ) {
#ifdef _VERBOSE
        Log::Debug ("acknowledgeTransfer invoked on non-active request.  Ignoring.");
#endif
        return false;
    }
    // Acknowledge transfer in DTCP transport, then check our pending list to see if
    // anything should be sent.
    //
    DtcpBaseConnTransport::acknowledgeTransfer ();

    checkPendingRequests ();
   

    return true;
}



bool  
AlpineDtcpConnTransport::processAlpineQueryPacket (AlpinePacket *  packet,
                                                   DataBuffer *    buffer)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineDtcpConnTransport::processAlpineQueryPacket invoked.");
#endif

    bool  status;
    AlpinePacket::t_PacketType  packetType;
    AlpineQueryPacket  queryPacket;


    // Marshall query packet data
    //
    packetType = packet->getPacketType();
    queryPacket.setPacketType (packetType);

    status = queryPacket.readData (buffer);
    if (!status) {
#ifdef _VERBOSE
        Log::Debug ("Invalid query packet data received in call to "
                             "AlpineDtcpConnTransport::processAlpineQueryPacket.");
#endif
        return false;
    }
    ulong  peerId;
    getTransportId (peerId);


    // Dispatch packet to the AlpineQueryMgr for the appropriate query packet type
    //
    switch (packetType) {

      case AlpinePacket::t_PacketType::queryDiscover  :
          AlpineQueryMgr::handleQueryDiscover (peerId, &queryPacket);
          break;

      case AlpinePacket::t_PacketType::queryOffer  :
          AlpineQueryMgr::handleQueryOffer (peerId, &queryPacket);
          break;

      case AlpinePacket::t_PacketType::queryRequest  :
          AlpineQueryMgr::handleQueryRequest (peerId, &queryPacket);
          break;

      case AlpinePacket::t_PacketType::queryReply  :
          AlpineQueryMgr::handleQueryReply (peerId, &queryPacket);
          break;


      // GNU g++ is very annoyed if you do not handle ALL enum values :/
      //
      case AlpinePacket::t_PacketType::peerListRequest  :
      case AlpinePacket::t_PacketType::peerListOffer  :
      case AlpinePacket::t_PacketType::peerListGet  :
      case AlpinePacket::t_PacketType::peerListData  :
      case AlpinePacket::t_PacketType::proxyRequest  :
      case AlpinePacket::t_PacketType::proxyAccepted  :
      case AlpinePacket::t_PacketType::proxyHalt  :
      default :
          Log::Error ("Invalid packet type passed in call to "
                               "AlpineDtcpConnTransport::processAlpineQueryPacket.");
          break;
    }


    return true;
}



bool  
AlpineDtcpConnTransport::processAlpinePeerPacket (AlpinePacket *  packet,
                                                  DataBuffer *    buffer)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineDtcpConnTransport::processAlpinePeerPacket invoked.");
#endif

    bool  status;
    AlpinePeerPacket  peerPacket;
    peerPacket.setPacketType (packet->getPacketType());

    status = peerPacket.readData (buffer);
    if (!status) {
#ifdef _VERBOSE
        Log::Debug ("Invalid peer packet data received in AlpineDtcpConnTransport::handleData.");
#endif
        return false;
    }
    return true;
}



bool  
AlpineDtcpConnTransport::processAlpineProxyPacket (AlpinePacket *  packet,
                                                   DataBuffer *    buffer)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineDtcpConnTransport::processAlpineProxyPacket invoked.");
#endif

    bool  status;
    AlpineProxyPacket  proxyPacket;
    proxyPacket.setPacketType (packet->getPacketType());

    status = proxyPacket.readData (buffer);
    if (!status) {
#ifdef _VERBOSE
        Log::Debug ("Invalid proxy packet data received in "
                             "AlpineDtcpConnTransport::handleData.");
#endif
        return false;
    }
    return true;
}



void  
AlpineDtcpConnTransport::checkPendingRequests ()
{
#ifdef _VERBOSE
    Log::Debug ("AlpineDtcpConnTransport::checkPendingRequests invoked.");
#endif

    // Before proceeding, verify that there is no request currently in progress.
    //
    if (pendingAck ())
        return;

    if (pendingIndex_) {

        t_ReliableRequest *  currRequest;
        currRequest = pendingIndex_->pendingRequests.front ();
        pendingIndex_->pendingRequests.pop_front ();
        
        requestId_ = currRequest->requestId;

        bool status;
        status = DtcpBaseConnTransport::sendReliableData (currRequest->data->buffer_,
                                                         currRequest->data->length_);

        delete currRequest->data;
        delete currRequest;

        if (!status) {
            Log::Error ("sendReliableData failed in call to "
                                 "AlpineDtcpConnTransport::checkPendingRequests!");

            // remove all other pending transfer, as this should only occur with a transport error.
            //
            cleanUp ();
            return;
        }

        // If this was the last request, we can delete the rest of the state information.
        //
        if (pendingIndex_->pendingRequests.empty()) {
            delete pendingIndex_;
            pendingIndex_ = nullptr;
            return;
        }
    }
}



void  
AlpineDtcpConnTransport::cleanUp ()
{
#ifdef _VERBOSE
    Log::Debug ("AlpineDtcpConnTransport::cleanUp invoked.");
#endif

    if (pendingIndex_) {
        for (const auto& item : pendingIndex_->pendingRequests) {
            delete item->data;
            delete item;
        }

        delete pendingIndex_;
        pendingIndex_ = nullptr;
    }
}



