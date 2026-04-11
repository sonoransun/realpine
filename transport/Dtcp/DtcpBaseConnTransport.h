/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <Platform.h>
#include <TransportInterface.h>

#ifdef ALPINE_TLS_ENABLED
class DtlsWrapper;
class TlsContext;
#endif

class DtcpBaseUdpTransport;
class DtcpBaseConnMux;


class DtcpBaseConnTransport : public TransportInterface
{
  public:
    DtcpBaseConnTransport();

    virtual ~DtcpBaseConnTransport();


    ////
    //
    // Transport interface operations
    //
    virtual bool setParent(DtcpBaseUdpTransport * parent);

    virtual bool getParent(DtcpBaseUdpTransport *& parent);

    virtual bool getDataBuffer(DataBuffer *& dataBuffer);

    virtual bool processData(const byte * data, uint dataLength);

    virtual bool processPacket(StackLinkInterface * packet);

    virtual bool sendData(const byte * data, uint dataLength);

    virtual bool sendPacket(StackLinkInterface * packet);

    virtual bool sendReliableData(const byte * data, uint dataLength);

    virtual bool sendReliablePacket(StackLinkInterface * packet);

    virtual bool pendingAck();

    virtual bool acknowledgeTransfer();


    // Callback from PendingAckMap
    //
    virtual bool handleSendFailure(ulong id);

    virtual bool handleSendReceived(ulong id);


    ////
    //
    // Derived implementation handlers
    //
    virtual bool handleData(const byte * data, uint dataLength) = 0;

    virtual bool handleSendReceived() = 0;

    virtual bool handleSendFailure() = 0;

    virtual bool handleConnectionClose() = 0;

    virtual bool handleConnectionDeactivate() = 0;


    // Connection identification
    //
    bool getTransportId(ulong & transportId);

    bool getPeerId(ulong & peerId);

    bool getMyId(ulong & myId);

    bool getPeerLocation(ulong & ipAddress, ushort & port);

    bool getRequestId(ulong & requestId);


    // Connection lifetime
    //
    bool getLastRecvTime(struct timeval & lastRecv);

    bool getLastSendTime(struct timeval & lastSend);

    bool close();

    bool deactivate();


#ifdef ALPINE_TLS_ENABLED
    bool enableTls(TlsContext & tlsCtx);

    bool isTlsEnabled() const;
#endif


  private:
    DtcpBaseUdpTransport * parentTransport_;
    DtcpBaseConnMux * mux_;
    struct timeval lastRecv_;
    struct timeval lastSend_;
    ulong transportId_;
    ulong peerId_;
    ulong myId_;
    ulong peerIpAddress_;
    ushort peerPort_;

    bool pendingAck_;
    ulong requestId_;
    ulong sendSequenceNum_;
    ulong recvSequenceNum_;

#ifdef ALPINE_TLS_ENABLED
    DtlsWrapper * dtlsWrapper_{nullptr};
#endif

    static ulong currSequenceNum_s;


    // configuration methods for DtcpBaseConnMux
    //
    bool setMux(DtcpBaseConnMux * mux);

    bool setTransportId(ulong transportId);

    bool setPeerId(ulong peerId);

    bool setMyId(ulong myId);

    bool setPeerLocation(ulong ipAddress, ushort port);

    bool getSendSequenceNum(ulong & sendSequenceNum);

    bool getRecvSequenceNum(ulong & recvSequenceNum);

    bool setRecvSequenceNum(ulong recvSequenceNum);


    friend class DtcpBaseConnMux;
    friend class DtcpBaseConnConnector;
};
