/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <TransportInterface.h>
#include <DtcpNetId.h>
#include <AutoThread.h>
#include <ReadWriteSem.h>
#include <PollSet.h>
#include <OptHash.h>


class UdpConnection;
class StackLinkInterface;
class DataBuffer;
class DtcpPacket;
class DtcpIORecord;
class DtcpBaseConnMux;
class DtcpBaseConnConnector;
class DtcpBaseConnTransport;
class DtcpThreadTable;
class DtcpMonitorThread;
class DtcpSocketThread;
class DtcpSendQueue;
class DtcpPendingAckMap;


class DtcpBaseUdpTransport : public TransportInterface
{
  public:

    DtcpBaseUdpTransport (const ulong   ipAddress,
                          const ushort  port);

    virtual ~DtcpBaseUdpTransport ();
                          


    // Initialize - create data members, prepare stack layers
    //
    bool  initialize ();

    // Active/Deactivate - start/stop processing of stack data and threads
    //
    bool  activate ();

    bool  shutdown ();


  
    // Derived stack link creation
    //
    virtual bool  createMux (DtcpBaseConnMux *& connMux) = 0;

    // Interface to Mux connection creation
    //
    virtual bool  createConnector (DtcpBaseConnConnector *& connector);



    // Any packets that are not DTCP go here...
    //
    virtual bool  handleData (const byte * data,
                              uint         dataLength) = 0;



    // Transport interface operations
    //
    virtual bool  getDataBuffer (DataBuffer *& dataBuffer);

    virtual bool  processData (const byte * data,
                               uint         dataLength);

    virtual bool  processPacket (StackLinkInterface * packet);


    virtual bool  sendPacket (StackLinkInterface * packet);

    virtual bool  sendReliablePacket (StackLinkInterface *     packet,
                                      DtcpBaseConnTransport *  requestor,
                                      ulong &                  requestId);


    bool  sendData (const ulong   ipAddress,
                    const ushort  port,
                    const byte *  data,
                    uint          dataLength);

    bool  sendReliableData (const ulong   ipAddress,
                            const ushort  port,
                            const byte *  data,
                            uint          dataLength,
                            DtcpBaseConnTransport *  requestor,
                            ulong &                  requestId);

    bool  reliableRequestComplete (ulong  requestId);

    bool  handleSendFailure (ulong  id);



    // Types
    //
    using t_ConnPendingAckIndex = std::unordered_map < t_NetId,
                       ulong,
                       OptHash<ulonglong>,
                       equal_to<ulonglong> >;

    using t_ConnPendingAckIndexPair = std::pair <t_NetId, ulong>;


    using t_MuxPendingAckIndex = std::unordered_map < t_NetId,
                       ulong,
                       OptHash<ulonglong>,
                       equal_to<ulonglong> >;

    using t_MuxPendingAckIndexPair = std::pair <t_NetId, ulong>;



  protected:

    // Factory method — override to supply a different connection type
    //
    virtual UdpConnection *  createConnection ();

    ulong         localIpAddress_;
    ushort        localPort_;


private:

    // Udp interface
    //
    UdpConnection *  udpConnection_;
    ReadWriteSem     udpConnectionLock_;


    // ConnMux must synchronize at interface
    //
    DtcpBaseConnMux *  connMux_;


    // Data members
    //
    bool          initialized_;
    bool          active_;
    DataBuffer *  nullBuffer_;
    ReadWriteSem  dataLock_;


    // threads to process timed events and socket input
    //
    DtcpSocketThread *  socketThread_;
    DtcpMonitorThread * monitorThread_;
    ReadWriteSem        threadLock_;


    // Thread table maps thread specific data storage per invocation
    //
    DtcpThreadTable *  threadTable_;

    
    // the pending ack map is used for retransmission and timeouts
    // of reliable packet transfers.
    //
    DtcpPendingAckMap *      ackMap_;
    t_ConnPendingAckIndex *  connAckIndex_;
    ReadWriteSem             ackMapLock_;


    // indexes and sets to ensure proper delivery of reliable packets
    // for use in ConnMux (to establish connections)
    //
    t_MuxPendingAckIndex *  muxAckIndex_;
    ReadWriteSem            muxAckSetLock_;


    // Send Queue to shape outgoing bandwidth and process send
    // requests according to preferences.
    //
    DtcpSendQueue *   sendQueue_;
    ReadWriteSem      sendQueueLock_;



    // Get temporary data buffer for sending packets
    //
    bool  getTempDataBuffer (DataBuffer *& dataBuffer);

    bool  releaseTempDataBuffer (DataBuffer * dataBuffer);


    // for DtcpSocketThread
    //
    bool  processSocketEvents ();

    bool  pollSocket (int                       timeout,
                      PollSet::t_FileDescList & activeFds);

    bool  readSocketData (DataBuffer * buffer,
                          ulong &      ipAddress,
                          ushort &     port);


    // for DtcpServiceThread
    //
    bool  processTimedEvents ();


    // for DtcpStackThreads
    //
    bool  processRecord (DtcpIORecord * record);

    bool  processEvents (t_ThreadId  threadId);

    void  threadIdle (t_ThreadId  threadId);


    // For DtcpSendQueue to transmits UDP packets
    //
    bool  sendUdpPacket (const ulong   ipAddress,
                         const ushort  port,
                         const byte *  data,
                         uint          dataLength);



    // Allow threads and utility objects access to private member methods 
    //
    friend class DtcpSocketThread;
    friend class DtcpMonitorThread;
    friend class DtcpStackThread;
    friend class DtcpBaseConnMux;
    friend class DtcpSendQueue;

};


