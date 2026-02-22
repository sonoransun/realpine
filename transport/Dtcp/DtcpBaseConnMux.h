/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <DtcpNetId.h>
#include <DtcpPacket.h>
#include <MuxInterface.h>
#include <ReadWriteSem.h>
#include <OptHash.h>
#include <functional>


class DtcpConnectionMap;
class TransportInterface;
class DtcpBaseConnAcceptor;
class DtcpBaseConnConnector;
class DtcpConnPacket;
class DataBuffer;
class DtcpBaseUdpTransport;
class DtcpBaseConnTransport;


class DtcpBaseConnMux : public MuxInterface
{
  public:
   
    DtcpBaseConnMux ();
 
    virtual ~DtcpBaseConnMux ();



    // Derived stack configuration methods
    //
    virtual bool createAcceptor (DtcpBaseConnAcceptor *&  acceptor) = 0;

    virtual bool createConnector (DtcpBaseConnConnector *&  connector) = 0;



    ////
    //
    // MuxInterface operations
    //
    virtual bool initialize (TransportInterface *  parentTransport);

    virtual bool processPacket (StackLinkInterface * packet);

    virtual bool requestTransport (ConnectorInterface * requestor,
                                   TransportInterface * transport);

    bool getParentTransport (TransportInterface *& parentTransport);



    // Locate transport by IP/Port
    //
    bool exists (ulong   ipAddress,
                 ushort  port);

    bool locateConnection (ulong                     ipAddress,
                           ushort                    port,
                           DtcpBaseConnTransport *&  connection);



    // Allow transports to cancel or explicitly acknowledge a reliable
    // data transfer.
    //
    bool acknowledgeTransfer (DtcpBaseConnTransport * transport);



    ////
    //
    // ConnMux implementation types
    //


    // pending request information - outside scope of reliable transport
    //
    struct t_ConnRequestRecord {
        ConnectorInterface * requestor;
        TransportInterface * transport;
        ulong                tempPeerId;
    };

    using t_ConnRequestIndex = std::unordered_map < ulong,
                       t_ConnRequestRecord *,
                       OptHash<ulong>,
                       equal_to<ulong> >;

    using t_ConnRequestIndexPair = std::pair <ulong, t_ConnRequestRecord *>;

  
 
    struct t_PendingAcceptRecord {
        ulong             myId;
        ulong             tempPeerId;
    };

    using t_PendingAcceptIndex = std::unordered_map < ulong,
                       t_PendingAcceptRecord *,
                       OptHash<ulong>,
                       equal_to<ulong> >;

    using t_PendingAcceptIndexPair = std::pair <ulong, t_PendingAcceptRecord *>;


    struct t_PendingTransportRecord {
        t_NetId               netId;
        TransportInterface *  transport;
        ConnectorInterface *  requestor;
    };

    using t_PendingTransportIndex = std::unordered_map < t_NetId,
                       t_PendingTransportRecord *,
                       OptHash<t_NetId>,
                       equal_to<t_NetId> >;

    using t_PendingTransportIndexPair = std::pair <t_NetId, t_PendingTransportRecord *>;



    // reliable transport indexes - pending acknowledgement
    //
    struct t_PendingAckRecord {
        t_NetId                   netId;
        DtcpPacket::t_PacketType  msgType;
        ulong                     requestId;
    };

    using t_PendingAckRecordIndex = std::unordered_map < t_NetId,
                       t_PendingAckRecord *,
                       OptHash<t_NetId>,
                       equal_to<t_NetId> >;

    using t_PendingAckRecordIndexPair = std::pair <t_NetId, t_PendingAckRecord *>;



    // incoming data message
    //
    struct t_IncomingMessage {
        DataBuffer * dataBuffer;
        DtcpPacket * packet;
        ulong        ipAddress;
        ushort       port;
        t_NetId      id;
    };




  private:


    // Data members
    //
    DtcpConnectionMap *        connectionMap_;
    ReadWriteSem               connMapLock_;

    t_ConnRequestIndex *       connRequestIndex_;
    t_PendingAcceptIndex *     pendingAcceptIndex_;
    t_PendingTransportIndex *  pendingTransportIndex_;
    ReadWriteSem               indexLock_;

    // The parent transport and acceptor must synchronize
    //
    DtcpBaseUdpTransport *     parentTransport_;
    DtcpBaseConnAcceptor *     acceptor_;
    bool                       initialized_;

    // Keep track of packets requiring acknowledgement.
    // (must be synchronized, shared among threads)
    //
    t_PendingAckRecordIndex *  pendingAckIndex_;
    ReadWriteSem               pendingAckLock_;



    // incoming packet handlers
    //
    bool handleConnRequestPacket (t_IncomingMessage & request);

    bool handleConnOfferPacket (t_IncomingMessage & request);

    bool handleConnAcceptPacket (t_IncomingMessage & request);

    bool handleConnSuspendPacket (t_IncomingMessage & request);

    bool handleConnResumePacket (t_IncomingMessage & request);

    bool handleConnDataPacket (t_IncomingMessage & request);

    bool handleConnReliableDataPacket (t_IncomingMessage & request);

    bool handleConnDataAckPacket (t_IncomingMessage & request);

    bool handleConnClosePacket (t_IncomingMessage & request);

    bool handlePollPacket (t_IncomingMessage & request);

    bool handleAckPacket (t_IncomingMessage & request);

    bool handleErrorPacket (t_IncomingMessage & request);

    bool handleNatDiscoverPacket (t_IncomingMessage & request);

    bool handleNatSourcePacket (t_IncomingMessage & request);

    bool handleTxnPacket (t_IncomingMessage & request);

    bool checkAcceptAckRetry (t_IncomingMessage & request);


    // sub handlers for ack packet for specific previous requests
    //
    bool pendingConnAcceptComplete (t_IncomingMessage &  request,
                                    t_PendingAckRecord * ackRecord);

    bool pendingConnCloseComplete (t_IncomingMessage &  request,
                                   t_PendingAckRecord * ackRecord);

    bool pendingConnSuspendComplete (t_IncomingMessage &  request,
                                     t_PendingAckRecord * ackRecord);

    bool pendingPollComplete (t_IncomingMessage &  request,
                              t_PendingAckRecord * ackRecord);



    // Pending entry creation methods
    //
    bool createConnectionAcceptEntry (DtcpConnPacket * packet,
                                      ulong            ipAddress,
                                      ushort           port);

    bool createConnectionRequestEntry (ConnectorInterface *    requestor,
                                       DtcpBaseConnTransport * transport);



    // Packet send methods
    //
    bool sendConnectionOffer (DtcpConnPacket * packet);

    bool sendConnectionRequest (DtcpBaseConnTransport * transport);

    bool sendConnectionAccept (DtcpBaseConnTransport * transport);

    bool sendAck (DtcpBaseConnTransport * transport);

    bool sendDataAck (DtcpBaseConnTransport * transport);



    // Pending ack record index methods
    //
    bool getPendingAckRecord (const t_NetId &        id,
                              t_PendingAckRecord *&  record);

    bool removePendingAckRecord (t_PendingAckRecord *  record);



};


