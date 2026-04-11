/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <DtcpBaseConnTransport.h>
#include <OptHash.h>
#include <list>


class AlpinePacket;
class DataBuffer;
class DataBlock;


class AlpineDtcpConnTransport : public DtcpBaseConnTransport
{
  public:
    AlpineDtcpConnTransport();

    virtual ~AlpineDtcpConnTransport();


    virtual bool handleData(const byte * data, uint dataLength);

    virtual bool handleConnectionClose();

    virtual bool handleSendReceived();

    virtual bool handleSendFailure();

    virtual bool handleConnectionDeactivate();


    // Provide some methods used for reliable transfer of data.
    // The Alpine stack requires a queue for reliable transfers which
    // is implemented at this layer.  The underlying DTCP transports
    // only keep track of a single reliable request.
    //
    virtual bool sendReliableData(const byte * data, uint dataLength, ulong requestId);

    virtual bool acknowledgeTransfer(ulong requestId);


    // Types and data structures for management of reliable transfer queue
    //
    struct t_ReliableRequest
    {
        ulong requestId;
        DataBlock * data;
    };

    using t_RequestList = list<t_ReliableRequest *>;

    struct t_RequestStateIndex
    {
        t_RequestList pendingRequests;
        ulong currRequestId;
    };


  private:
    bool processAlpineQueryPacket(AlpinePacket * packet, DataBuffer * buffer);

    bool processAlpinePeerPacket(AlpinePacket * packet, DataBuffer * buffer);

    bool processAlpineProxyPacket(AlpinePacket * packet, DataBuffer * buffer);

    void checkPendingRequests();

    void cleanUp();


    ulong requestId_;
    t_RequestStateIndex * pendingIndex_;  // this is usually unused.  Only allocate if needed.
};
