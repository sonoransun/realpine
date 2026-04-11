/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <DtcpBaseConnMux.h>
#include <DtcpBaseConnTransport.h>
#include <DtcpBaseUdpTransport.h>
#include <DtcpIORecord.h>
#include <DtcpMonitorThread.h>
#include <DtcpPacket.h>
#include <DtcpPendingAckMap.h>
#include <DtcpSendQueue.h>
#include <DtcpSocketThread.h>
#include <DtcpStack.h>
#include <DtcpThreadTable.h>

#include <DataBlock.h>
#include <DataBuffer.h>
#include <Log.h>
#include <ReadLock.h>
#include <StackLinkInterface.h>
#include <StringUtils.h>
#include <ThreadUtils.h>
#include <UdpConnection.h>
#include <WriteLock.h>
#include <cstring>


DtcpBaseUdpTransport::DtcpBaseUdpTransport(const ulong ipAddress, const ushort port)
{
#ifdef _VERBOSE
    string strIpAddress;
    NetUtils::longIpToString(ipAddress, strIpAddress);

    Log::Debug("DtcpBaseUdpTransport constructor invoked.  Parameters:"s + "\nipAddress: "s + strIpAddress +
               "\nport: "s + std::to_string(ntohs(port)) + "\n");
#endif

    udpConnection_ = nullptr;
    initialized_ = false;
    connMux_ = nullptr;
    initialized_ = false;
    active_ = false;

    localIpAddress_ = ipAddress;
    localPort_ = port;

    socketThread_ = nullptr;
    monitorThread_ = nullptr;

    ackMap_ = nullptr;
    connAckIndex_ = nullptr;
    muxAckIndex_ = nullptr;

    sendQueue_ = nullptr;
}


bool
DtcpBaseUdpTransport::initialize()
{
#ifdef _VERBOSE
    Log::Debug("DtcpBaseUdpTransport::initialize invoked.");
#endif

    // Lock everything, nothing else should be using the transport at this point
    //
    WriteLock udpLock(udpConnectionLock_);
    WriteLock dataLock(dataLock_);
    WriteLock threadLock(threadLock_);
    WriteLock ackLock(ackMapLock_);
    WriteLock queueLock(sendQueueLock_);


    bool status;


    // allocate null buffer for packets that must be
    // dropped.
    nullBuffer_ = new DataBuffer(64 * 1024);  // max UDP packet size


    // create ConnMux for handling connection packets.
    //
    status = createMux(connMux_);

    if (!status) {
        Log::Error("createMux failed in DtcpBaseUdpTransport::initialize.");
        return false;
    }
    status = connMux_->initialize(static_cast<TransportInterface *>(this));

    if (!status) {
        Log::Error("connMux_ initialize failed in DtcpBaseUdpTransport::initialize.");
        return false;
    }
    // allocate threads to process timed events and udp input.
    //
    threadTable_ = new DtcpThreadTable(this);

    socketThread_ = new DtcpSocketThread(this);
    monitorThread_ = new DtcpMonitorThread(this);

    status = socketThread_->create();

    if (!status) {
        Log::Error("Unable to create socket thread in DtcpBaseUdpTransport::initialize.");
        return false;
    }
    status = monitorThread_->create();

    if (!status) {
        Log::Error("Unable to create monitor thread in DtcpBaseUdpTransport::initialize.");
        return false;
    }
    // Allocate pending ack map and indexes for reliable transfers
    //
    ackMap_ = new DtcpPendingAckMap(this);
    connAckIndex_ = new t_ConnPendingAckIndex;
    muxAckIndex_ = new t_MuxPendingAckIndex;


    // Allocate DtcpSendQueue for this UDP transport
    //
    ulong uploadRate = 0;  // MRP_TEMP  get upload rate from somewhere...
    sendQueue_ = new DtcpSendQueue(this, uploadRate);


    // initialization complete
    //
    initialized_ = true;


    return true;
}


bool
DtcpBaseUdpTransport::activate()
{
    Log::Debug("DtcpBaseUdpTransport::activate invoked.");

    if (!initialized_) {
        Log::Error("DtcpBaseUdpTransport::activate invoked before initialize.");
        return false;
    }
    // create UDP connection for sending/receiving DTCP packets.
    //
    WriteLock udpLock(udpConnectionLock_);
    udpConnection_ = createConnection();

    if (!udpConnection_) {
        Log::Error("createConnection returned null in DtcpBaseUdpTransport::activate.");
        return false;
    }

    bool status;
    status = udpConnection_->create(localIpAddress_, localPort_);

    if (!status) {
        delete udpConnection_;
        udpConnection_ = nullptr;

        Log::Error("Could not create UDP connection in DtcpBaseUdpTransport::activate.");
        return false;
    }
    // activate socket and monitor threads...
    //
    WriteLock threadLock(threadLock_);

    socketThread_->resume();
    monitorThread_->resume();


    // Register with DtcpStack — primary or additional
    //
    status = DtcpStack::registerUdpTransport(this, connMux_);

    if (!status) {
        // Primary already registered — try as additional transport
        status = DtcpStack::registerAdditionalUdpTransport(this, connMux_);

        if (!status) {
            delete udpConnection_;
            udpConnection_ = nullptr;

            Log::Error("Could not register with DtcpStack in DtcpBaseUdpTransport::activate.");
            return false;
        }
    }
    // activation complete
    //
    active_ = true;


    return true;
}


bool
DtcpBaseUdpTransport::shutdown()
{
    Log::Debug("DtcpBaseUdpTransport::shutdown invoked.");

    if (!active_) {
        // not active, can not shutdown.
        return false;
    }
    // MRP_TEMP perform complete cleanup


    WriteLock udpLock(udpConnectionLock_);
    WriteLock threadLock(threadLock_);


    delete udpConnection_;
    udpConnection_ = nullptr;

    active_ = false;


    return true;
}


UdpConnection *
DtcpBaseUdpTransport::createConnection()
{
    return new UdpConnection();
}


DtcpBaseUdpTransport::~DtcpBaseUdpTransport()
{
#ifdef _VERBOSE
    Log::Debug("DtcpBaseUdpTransport desctructor invoked.");
#endif

    // MRP_TEMP check for contention!
    WriteLock udpLock(udpConnectionLock_);
    WriteLock dataLock(dataLock_);
    WriteLock threadLock(threadLock_);
    WriteLock ackLock(ackMapLock_);
    WriteLock queueLock(sendQueueLock_);


    delete nullBuffer_;

    delete udpConnection_;

    delete connMux_;

    delete threadTable_;

    delete socketThread_;

    delete monitorThread_;

    delete ackMap_;

    delete connAckIndex_;

    delete muxAckIndex_;
}


bool
DtcpBaseUdpTransport::createConnector(DtcpBaseConnConnector *& connector)
{
#ifdef _VERBOSE
    Log::Debug("DtcpBaseUdpTransport::createConnector invoked.");
#endif

    bool status;

    status = connMux_->createConnector(connector);


    return status;
}


bool
DtcpBaseUdpTransport::getDataBuffer(DataBuffer *& dataBuffer)
{
#ifdef _VERBOSE
    Log::Debug("DtcpBaseUdpTransport::getDataBuffer invoked.");
#endif

    if (!active_) {
        return false;
    }
    t_ThreadId id;
    id = ThreadUtils::getThreadId();

    bool status;
    DtcpIORecord * record;

    status = threadTable_->locateRecord(id, record);

    if (!status) {
        // no storage for this thread?
        Log::Error("Unable to locate storage for thread in DtcpBaseUdpTransport::getDataBuffer.");

        return false;
    }
    dataBuffer = record->buffer_;


    return true;
}


bool
DtcpBaseUdpTransport::getTempDataBuffer(DataBuffer *& dataBuffer)
{
#ifdef _VERBOSE
    Log::Debug("DtcpBaseUdpTransport::getTempDataBuffer invoked.");
#endif

    bool status;

    status = threadTable_->allocateBuffer(dataBuffer);

    if (!status) {
        // no storage available?
        //
        Log::Error("Unable to allocate temporary storage "
                   "in DtcpBaseUdpTransport::getTempDataBuffer.");
        return false;
    }
    return true;
}


bool
DtcpBaseUdpTransport::releaseTempDataBuffer(DataBuffer * dataBuffer)
{
#ifdef _VERBOSE
    Log::Debug("DtcpBaseUdpTransport::releaseTempDataBuffer invoked.");
#endif

    bool status;

    status = threadTable_->releaseBuffer(dataBuffer);

    if (!status) {
        // This should never occur...
        //
        Log::Error("Unable to release temporary storage "
                   "in DtcpBaseUdpTransport::releaseTempDataBuffer.");
        return false;
    }
    return true;
}


bool
DtcpBaseUdpTransport::processSocketEvents()
{
#ifdef _VERY_VERBOSE
    Log::Debug("DtcpBaseUdpTransport::processSocketEvents invoked.");
#endif


    // First have send queue process any pending requests.  Use
    // the delay returned to calculate our timeout value for poll.
    // This allows us to return to the send queue in an appropriate
    // timeframe for each send packet.
    //
    bool status;
    ulong requestsPending;
    int timeout;
    struct timeval delay;

    // scope lock
    {
        WriteLock lock(sendQueueLock_);

        requestsPending = 0;
        status = sendQueue_->requestsPending(requestsPending);

        if ((status) && (requestsPending > 0)) {
            status = sendQueue_->processEvents(delay);
        }
    }

    // Default timeout period.
    timeout = 100;

    if (requestsPending) {
        if (!status) {
            // Hopefully this will never happen
            Log::Error("SendQueue processEvents failed in DtcpBaseUdpTransport::processSocketEvents!");

            // ignore for now... (continue processing?)
        } else {
            // Calculate the timeout in milliseconds to block in poll
            //
            timeout = delay.tv_usec / 1000;
            timeout += (1000 * delay.tv_sec);
            return false;
        }
    }


    // Poll on socket FD, see if we have data waiting.
    //
    PollSet::t_FileDescList activeFds;

    status = pollSocket(timeout, activeFds);

    if (!status) {
        Log::Debug("Poll failed...");
        return false;
    }
    if (activeFds.empty()) {
        // no active FD's, timeout...
        return true;
    }
#ifdef _VERBOSE
    Log::Debug("Pending data packets in UDP connection socket.  Processing...");
#endif

    // get an available buffer to hold UDP data
    //
    DtcpIORecord * record;
    status = threadTable_->allocateRecord(record);

    if (!status) {
        // could not obtain free record, probably out of memory.
        Log::Error("Could not obtain IORecord in DtcpBaseUdpTransport::processSocketEvents.");

        // receive udp data into null buffer.
        //
        ulong tempIp;
        ushort tempPort;

        readSocketData(nullBuffer_, tempIp, tempPort);

        threadTable_->returnRecord(record);

        return false;
    }
    ulong sourceIpAddress;
    ushort sourcePort;

    status = readSocketData(record->buffer_, sourceIpAddress, sourcePort);

    if (!status) {
        // error reading udp data from socket.
        //
        Log::Error("Could not read UDP data from socket in DtcpBaseUdpTransport::processSocketEvents.");

        threadTable_->returnRecord(record);

        return false;
    }
    record->sourceIpAddress_ = sourceIpAddress;
    record->sourcePort_ = sourcePort;

#if 0
// MRP_TEMP  Major error here, fix!!!!!

    // Check the transPendingAckIndex at this point to prevent needless rexmits
    //
    t_NetId   netId;

    ipPortToNetId (sourceIpAddress,
                   sourcePort,
                   netId);

    {
        WriteLock  lock(ackMapLock_);

        auto iter = connAckIndex_->find (netId);

        if (iter != connAckIndex_->end ()) {

            // remove pending status for this source peer.
            //
#ifdef _VERBOSE
            Log::Debug ("+ Pending response received, removing state...");
#endif

            ulong pendingId;
            pendingId = (*iter).second;

            ackMap_->remove (pendingId);

            connAckIndex_->erase (netId);
            return;
        }
    }
#endif


    // put record in queue to be processed by first available thread
    //
#ifdef _VERBOSE
    Log::Debug("UDP Receive: placing record in thread table queue...");
#endif

    status = threadTable_->processRecord(record);


    return true;
}


bool
DtcpBaseUdpTransport::pollSocket(int timeout, PollSet::t_FileDescList & activeFds)
{
#ifdef _VERY_VERBOSE
    Log::Debug("DtcpBaseUdpTransport::pollSocket invoked.");
#endif

    if (!active_) {
        return false;
    }
    static PollSet * pollSet = nullptr;

    if (!pollSet) {

#ifdef _VERBOSE
        Log::Debug("Creating poll set.");
#endif

        int udpFd;

        pollSet = new PollSet();

        // scope lock
        {
            WriteLock lock(udpConnectionLock_);

            udpFd = udpConnection_->getFd();
        }

        if (!pollSet->add(udpFd)) {
            delete pollSet;
            pollSet = nullptr;

            return false;
        }
        return false;
    }

    bool status;

    status = pollSet->poll(timeout, activeFds);

    if (!status) {
        Log::Error("Poll failed in DtcpBaseUdpTransport::processEvents.");
        return false;
    }
    return true;
}


bool
DtcpBaseUdpTransport::readSocketData(DataBuffer * buffer, ulong & ipAddress, ushort & port)
{
#ifdef _VERBOSE
    Log::Debug("DtcpBaseUdpTransport::readSocketData invoked.");
#endif

    if (!active_) {
        return false;
    }
    byte * rawBuffer;
    uint bufferSize;
    uint receivedLength;
    bool status;

    buffer->writeReset();

    status = buffer->getWriteBuffer(rawBuffer, bufferSize);

    if (!status) {
        return false;
    }
    // scope write lock for read from socket
    //
    {
        WriteLock lock(udpConnectionLock_);

        status = udpConnection_->receiveData(rawBuffer, bufferSize, ipAddress, port, receivedLength);
    }

    if (!status) {
        return false;
    }
    status = buffer->addWriteBytes(receivedLength);

    if (!status) {
#ifdef _VERBOSE
        Log::Error("addWriteBytes failed in DtcpBaseUdpTransport::processEvents.");
#endif
        return false;
    }
#ifdef _VERBOSE
    string strIpAddress;
    NetUtils::longIpToString(ipAddress, strIpAddress);

    Log::Debug("DtcpBaseUdpTransport::receiveData values:"s + "\nsourceIpAddress: "s + strIpAddress +
               "\nsourcePort: "s + std::to_string(ntohs(port)) + "\nreceivedLength: "s +
               std::to_string(receivedLength));
#endif


    return true;
}


bool
DtcpBaseUdpTransport::processTimedEvents()
{
#ifdef _VERBOSE
    Log::Debug("DtcpBaseUdpTransport::processTimedEvents invoked.");
#endif

    // process pending ack states every 100ms to ensure
    // prompt repsonse.
    //

    struct timeval timeout;

    while (true) {

        // must reassign for each invocation, otherwise timeout gets mangled...
        // (see the linux man page for select.  Only affects linux systems)
        //
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;

        select(0, nullptr, nullptr, nullptr, &timeout);

        // scope lock
        {
            WriteLock lock(ackMapLock_);

            ackMap_->processTimers();
        }
    }


    return true;
}


bool
DtcpBaseUdpTransport::processEvents(t_ThreadId threadId)
{
#ifdef _VERBOSE
    Log::Debug("DtcpBaseUdpTransport::processEvents invoked for thread ID: "s + threadIdToString(threadId));
#endif

    // get next record in queue, process, repeat.
    //
    bool firstRequest = true;
    bool finished = false;
    bool status;

    DtcpIORecord * currRecord;

    while (!finished) {

        status = threadTable_->getNextRecord(threadId, currRecord);

        if (!status) {
#ifdef _VERBOSE
            Log::Debug("No more records to process in DtcpBaseUdpTransport::processEvents.  Done.");
#endif

            finished = true;
            continue;
        }

        firstRequest = false;

        // process record
        //
        status = processRecord(currRecord);

        if (!status) {
            Log::Error("processRecord failed in DtcpBaseUdpTransport::processEvents.");
            return false;
        }
    }

    // cleanup any remaining state while we pause for incoming buffers.
    // NOTE: If this is the first request that failed, another thread beat
    // us to this record, no need to cleanup state with processComplete() call.
    //
    if (!firstRequest) {
        threadTable_->processComplete(threadId);
    }


    // return false to indicate end of processing.
    //
    return false;
}


void
DtcpBaseUdpTransport::threadIdle(t_ThreadId threadId)
{
#ifdef _VERBOSE
    Log::Debug("DtcpBaseUdpTransport::threadIdle invoked.");
#endif

    threadTable_->threadIdle(threadId);
}


bool
DtcpBaseUdpTransport::processRecord(DtcpIORecord * record)
{
#ifdef _VERBOSE
    Log::Debug("DtcpBaseUdpTransport::processRecord invoked.");
#endif

    bool status;

    // determine if this is a DtcpPacket or not...
    //
    DataBuffer * buffer;
    DtcpPacket * packet;
    buffer = record->buffer_;
    packet = record->packet_;

    buffer->readReset();

    if ((!connMux_) || !packet->readData(buffer)) {

#ifdef _VERBOSE
        Log::Debug("Not a DTCP packet or no MUX, passing to data handler.");
#endif

        // Not a DTCP packet...
        //
        byte * data;
        uint dataLength;

        status = buffer->getReadBuffer(data, dataLength);

        if (!status) {
            Log::Error("getReadBuffer failed in DtcpBaseUdpTransport::processBuffer.");
            return false;
        }
        status = processData(data, dataLength);
    } else {

#ifdef _VERBOSE
        Log::Debug("Received DTCP packet, processing...");
#endif

        // Pass this packet up the protocol stack...
        //
        packet->setPeerLocation(record->sourceIpAddress_, record->sourcePort_);

        status = processPacket(static_cast<StackLinkInterface *>(packet));
    }


    return status;
}


bool
DtcpBaseUdpTransport::processData(const byte * data, uint dataLength)
{
#ifdef _VERBOSE
    Log::Debug("DtcpBaseUdpTransport::processData invoked.");
#endif

    if (!initialized_) {
        return false;
    }
    // Invoke derived handler (if present) to handle non DTCP packets.
    //
    bool status;

    status = handleData(data, dataLength);

    return status;
}


bool
DtcpBaseUdpTransport::processPacket(StackLinkInterface * packet)
{
#ifdef _VERBOSE
    Log::Debug("DtcpBaseUdpTransport::processPacket invoked.");
#endif

    // Pass to connection mux
    //
    bool status;

    status = connMux_->processPacket(packet);

    if (!status) {
#ifdef _VERBOSE
        Log::Debug("processPacket failed for DTCP connection multiplexor.");
#endif
        return false;
    }
    return true;
}


bool
DtcpBaseUdpTransport::sendPacket(StackLinkInterface * packet)
{
#ifdef _VERBOSE
    Log::Debug("DtcpBaseUdpTransport::sendPacket invoked.");
#endif

    if (!initialized_) {
        return false;
    }
    DataBuffer * buffer;
    bool status;

    status = getTempDataBuffer(buffer);

    if (!status) {
        // should not happen.
        Log::Debug("Error getting data buffer in DtcpBaseUdpTransport::sendPacket.");

        return false;
    }
    buffer->writeReset();

    status = packet->writeData(buffer);

    if (!status) {
        Log::Debug("writeData failed for packet in DtcpBaseUdpTransport::sendPacket.");
        releaseTempDataBuffer(buffer);
        return false;
    }
    // If this is a DtcpPacket, update destination information...
    //
    DtcpPacket * dtcpPacket;
    dtcpPacket = dynamic_cast<DtcpPacket *>(packet);

    ulong destIpAddress;
    ushort destPort;

    if (dtcpPacket) {

        status = dtcpPacket->getPeerLocation(destIpAddress, destPort);

        if (!status) {
            Log::Debug("getPeerLocation failed in DtcpBaseUdpTransport::sendPacket.");
            releaseTempDataBuffer(buffer);
            return false;
        }
    } else {
        // For now, only DtcpPackets are supported.
        //
        Log::Error("Invalid packet type passed to DtcpBaseUdpTransport::sendReliablePacket.");
        releaseTempDataBuffer(buffer);

        return false;
    }
    // Send packet buffer data to the DtcpSendQueue
    //
    WriteLock lock(sendQueueLock_);

    status = sendQueue_->addRequest(false, destIpAddress, destPort, buffer);

    if (!status) {
        Log::Error("DtcpSendQueue addRequest failed in DtcpBaseUdpTransport::sendPacket!");
        releaseTempDataBuffer(buffer);
        return false;
    }
    return true;
}


bool
DtcpBaseUdpTransport::sendReliablePacket(StackLinkInterface * packet,
                                         DtcpBaseConnTransport * requestor,
                                         ulong & requestId)
{
#ifdef _VERBOSE
    Log::Debug("DtcpBaseUdpTransport::sendReliablePacket invoked.");
#endif

    if (!initialized_) {
        return false;
    }
    DataBuffer * buffer;
    bool status;

    status = getTempDataBuffer(buffer);

    if (!status) {
        // should not happen.
        Log::Debug("Error getting data buffer in DtcpBaseUdpTransport::sendReliablePacket.");

        return false;
    }
    buffer->writeReset();

    status = packet->writeData(buffer);

    if (!status) {
        Log::Debug("writeData failed for packet in DtcpBaseUdpTransport::sendReliablePacket.");
        releaseTempDataBuffer(buffer);
        return false;
    }
    // If this is a DtcpPacket, update destination information...
    //
    DtcpPacket * dtcpPacket;
    dtcpPacket = dynamic_cast<DtcpPacket *>(packet);

    ulong destIpAddress;
    ushort destPort;

    if (dtcpPacket) {

        status = dtcpPacket->getPeerLocation(destIpAddress, destPort);

        if (!status) {
            Log::Debug("getPeerLocation failed in DtcpBaseUdpTransport::sendPacket.");
            releaseTempDataBuffer(buffer);
            return false;
        }
    } else {
        // For now, only DtcpPackets are supported.
        //
        Log::Error("Invalid packet type passed to DtcpBaseUdpTransport::sendReliablePacket.");
        releaseTempDataBuffer(buffer);

        return false;
    }
    // Copy data for use in PendingAckMap.
    // (need to do this before sending to request queue)
    //
    byte * data;
    uint dataLength;

    status = buffer->getData(data, dataLength);

    if (!status) {
        Log::Error("DataBuffer getData call failed in DtcpBaseUdpTransport::sendReliablePacket!");
        return false;
    }
    DataBlock * dataBlock;
    dataBlock = new DataBlock(dataLength);

    memcpy(dataBlock->buffer_.get(), data, dataLength);
    dataBlock->length_ = dataLength;


    // Send packet buffer data to the DtcpSendQueue
    //
    {
        WriteLock lock(sendQueueLock_);

        status = sendQueue_->addRequest(true, destIpAddress, destPort, buffer);
    }

    if (!status) {
        Log::Error("DtcpSendQueue addRequest failed in DtcpBaseUdpTransport::sendReliablePacket!");
        releaseTempDataBuffer(buffer);
        delete dataBlock;
        return false;
    }
    // Index reliable request
    {
        WriteLock lock(ackMapLock_);

        status = ackMap_->add(requestor, dataBlock, destIpAddress, destPort, requestId);
    }

    if (!status) {
        // ? this should never happen...
        //
        Log::Error("Unable to add request to pending ack map "
                   "in DtcpBaseUdpTransport::sendReliablePacket!");
        delete dataBlock;

        return false;
    }
    return true;
}


bool
DtcpBaseUdpTransport::sendData(const ulong ipAddress, const ushort port, const byte * data, uint dataLength)
{
#ifdef _VERBOSE
    string strIpAddress;
    NetUtils::longIpToString(ipAddress, strIpAddress);

    Log::Debug("DtcpBaseUdpTransport::sendData invoked.  Parameters:"s + "\nipAddress: "s + strIpAddress + "\nport: "s +
               std::to_string(ntohs(port)) + "\ndataLength: "s + std::to_string(dataLength));
#endif


    // Create temporary send buffer for this request to pass
    // to DtcpSendQueue
    //
    DataBuffer * buffer;
    bool status;

    status = getTempDataBuffer(buffer);

    if (!status) {
        // should not happen.
        Log::Debug("Error getting data buffer in DtcpBaseUdpTransport::sendData.");

        return false;
    }
    buffer->setData(data, dataLength);


    // Put in send queue
    //
    WriteLock lock(sendQueueLock_);

    status = sendQueue_->addRequest(false, ipAddress, port, buffer);

    if (!status) {
        Log::Error("DtcpSendQueue addRequest failed in DtcpBaseUdpTransport::sendData!");
        releaseTempDataBuffer(buffer);
        return false;
    }
    return true;
}


bool
DtcpBaseUdpTransport::sendReliableData(const ulong ipAddress,
                                       const ushort port,
                                       const byte * data,
                                       uint dataLength,
                                       DtcpBaseConnTransport * requestor,
                                       ulong & requestId)
{
#ifdef _VERBOSE
    string strIpAddress;
    NetUtils::longIpToString(ipAddress, strIpAddress);

    Log::Debug("DtcpBaseUdpTransport::sendReliableData invoked.  Parameters:"s + "\nipAddress: "s + strIpAddress +
               "\nport: "s + std::to_string(ntohs(port)) + "\ndataLength: "s + std::to_string(dataLength));
#endif

    // Create temporary send buffer for this request to pass
    // to DtcpSendQueue
    //
    DataBuffer * buffer;
    bool status;

    status = getTempDataBuffer(buffer);

    if (!status) {
        // should not happen.
        Log::Debug("Error getting data buffer in DtcpBaseUdpTransport::sendReliableData.");

        return false;
    }
    buffer->setData(data, dataLength);


    // Put in send queue
    //
    {
        WriteLock lock(sendQueueLock_);

        status = sendQueue_->addRequest(true, ipAddress, port, buffer);
    }

    if (!status) {
        Log::Error("DtcpSendQueue addRequest failed in DtcpBaseUdpTransport::sendReliableData!");
        releaseTempDataBuffer(buffer);
        return false;
    }
    // Copy data for use in PendingAckMap.
    //
    DataBlock * dataBlock;
    dataBlock = new DataBlock(dataLength);

    memcpy(dataBlock->buffer_.get(), data, dataLength);
    dataBlock->length_ = dataLength;

    // scope lock
    {
        WriteLock lock(ackMapLock_);

        status = ackMap_->add(requestor, dataBlock, ipAddress, port, requestId);
    }

    if (!status) {
        // ? this should never happen...
        //
        Log::Error("Unable to add request to pending ack map "
                   "in DtcpBaseUdpTransport::sendReliableData.");
        delete dataBlock;

        return false;
    }
    return status;
}


bool
DtcpBaseUdpTransport::handleSendFailure(ulong id)
{
#ifdef _VERBOSE
    Log::Debug("DtcpBaseUdpTransport::handleSendFailure invoked.");
#endif

    return true;
}


bool
DtcpBaseUdpTransport::reliableRequestComplete(ulong requestId)
{
#ifdef _VERBOSE
    Log::Debug("DtcpBaseUdpTransport::reliableRequestComplete invoked.");
#endif

    WriteLock lock(ackMapLock_);

    bool status;
    status = ackMap_->remove(requestId);


    return status;
}


bool
DtcpBaseUdpTransport::sendUdpPacket(const ulong ipAddress, const ushort port, const byte * data, uint dataLength)
{
#ifdef _VERBOSE
    string strIpAddress;
    NetUtils::longIpToString(ipAddress, strIpAddress);

    Log::Debug("DtcpBaseUdpTransport::sendUdpPacket invoked.  Parameters:"s + "\nipAddress: "s + strIpAddress +
               "\nport: "s + std::to_string(ntohs(port)) + "\ndataLength: "s + std::to_string(dataLength));
#endif

    bool status;

    // scope lock
    {
        WriteLock lock(udpConnectionLock_);

        status = udpConnection_->sendData(ipAddress, port, data, dataLength);
    }

    if (!status) {
        Log::Error("Send failed in DtcpBaseUdpTransport::sendUdpPacket!");
        return false;
    }
    return status;
}
