/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AlpineStack.h>
#include <AlpinePacket.h>
#include <AlpinePeerPacket.h>
#include <AlpineProxyPacket.h>
#include <AlpineQueryPacket.h>
#include <AlpineGroupMgr.h>
#include <AlpinePeerMgr.h>
#include <AlpineQueryMgr.h>
#include <AlpineServiceThread.h>
#include <AlpineDtcpUdpTransport.h>
#include <AlpineDtcpConnTransport.h>
#include <AlpineMulticastUdpTransport.h>
#include <AlpineBroadcastUdpTransport.h>
#include <AlpineRawWifiUdpTransport.h>
#include <DtcpStack.h>
#include <DataBuffer.h>
#include <Log.h>
#include <StringUtils.h>
#include <WriteLock.h>
#include <ReadLock.h>
#include <Platform.h>
#include <thread>

#ifdef ALPINE_TLS_ENABLED
#include <PeerTlsVerifier.h>
#include <DtlsWrapper.h>
#endif



bool                              AlpineStack::initialized_s = false;
AlpineServiceThread *             AlpineStack::serviceThread_s = nullptr;
AlpineStackConfig *               AlpineStack::configuration_s = nullptr;
AlpineDtcpUdpTransport *          AlpineStack::baseUdpTransport_s = nullptr;
DtcpBaseUdpTransport *            AlpineStack::multicastTransport_s = nullptr;
DtcpBaseUdpTransport *            AlpineStack::broadcastTransport_s = nullptr;
DtcpBaseUdpTransport *            AlpineStack::rawWifiTransport_s = nullptr;
ReadWriteSem                      AlpineStack::dataLock_s;
std::condition_variable           AlpineStack::eventCV_s;
std::mutex                        AlpineStack::eventMutex_s;
bool                              AlpineStack::eventPending_s = false;
AlpineStack::CompletedQueryCallback  AlpineStack::completedQueryCallback_s;

static constexpr int  EVENT_LOOP_MAX_WAIT_MS = 100;  // max idle wait between iterations
static constexpr int  EVENT_LOOP_MIN_INTERVAL_MS = 10; // minimum time between iterations



// Ctor defaulted in header


// Dtor defaulted in header



bool  
AlpineStack::initialize (AlpineStackConfig &  configuration)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineStack::initialize invoked.");
#endif

    WriteLock  lock(dataLock_s);

    if (initialized_s) {
        Log::Error ("Attempt to re-initialize AlpineStack!");
        return false;
    }
    // Verify that all configuration data is correct, get required values...
    //
    bool   status;
    ulong  localIp;
    ushort localPort;

    status = configuration.getLocalEndpoint (localIp, localPort);

    if (!status) {
        Log::Error ("Invalid endpoint in stack configuration passed in call to "
                             "AlpineStack::initialize!");
        return false;
    }
    ulong  maxConcurrentQueries;
    status = configuration.getMaxConcurrentQueries (maxConcurrentQueries);

    if (!status) {
        Log::Error ("Invalid concurrent query limit in stack configuration passed in call to "
                             "AlpineStack::initialize!");
        return false;
    }
    configuration_s = new AlpineStackConfig (configuration);


    // Initialize DTCP stack and base UDP transport for local endpoint
    //
    status = DtcpStack::initialize ();

    if (!status) {
        Log::Error ("Initializing DtcpStack failed in call to AlpineStack::initialize!");
        return false;
    }
    baseUdpTransport_s = new AlpineDtcpUdpTransport (localIp, localPort);   

    status = baseUdpTransport_s->initialize ();

    if (!status) {
        Log::Error ("Initialization of base UDP transport failed in call to "
                             "AlpineStack::initialize!");
        cleanUp ();
        return false;
    }
    // Initialize various ALPINE stack components
    //
    status = AlpineGroupMgr::initialize ();

    if (!status) {
        Log::Error ("Initialization of AlpineGroupMgr failed in call to "
                             "AlpineStack::initialize!");
        cleanUp ();
        return false;
    }
    status = AlpinePeerMgr::initialize ();

    if (!status) {
        Log::Error ("Initialization of AlpinePeerMgr failed in call to "
                             "AlpineStack::initialize!");
        cleanUp ();
        return false;
    }
    status = AlpineQueryMgr::initialize (maxConcurrentQueries);

    if (!status) {
        Log::Error ("Initialization of AlpineQueryMgr failed in call to "
                             "AlpineStack::initialize!");
        cleanUp ();
        return false;
    }
    // Create service thread to handle various events in the ALPINE stack outside
    // of incoming requests.
    //
    serviceThread_s = new AlpineServiceThread;
    serviceThread_s->setDeleteOnExit (true);

    status = serviceThread_s->run ();

    if (!status) {
        Log::Error ("Starting AlpineServiceThread failed in call to "
                             "AlpineStack::initialize!");
        cleanUp ();
        return false;
    }
    // Everything initialized, activate base UDP transport and start processing traffic...
    //
    status = baseUdpTransport_s->activate ();

    if (!status) {
        Log::Error ("Activation of base UDP transport failed in call to "
                             "AlpineStack::initialize!");
        cleanUp ();
        return false;
    }

    // Optionally create and activate wireless transports
    //
    if (configuration.multicastEnabled()) {
        string group;
        ushort mport;
        configuration.getMulticastEndpoint (group, mport);

        multicastTransport_s = new AlpineMulticastUdpTransport (localIp, localPort,
                                                                 group, mport);
        status = multicastTransport_s->initialize ();

        if (status) {
            status = multicastTransport_s->activate ();
        }

        if (status) {
            Log::Info ("Multicast transport activated on "s + group +
                       ":" + std::to_string (mport));
        }
        else {
            Log::Error ("Failed to activate multicast transport.");
            delete multicastTransport_s;
            multicastTransport_s = nullptr;
        }
    }

    if (configuration.broadcastEnabled()) {
        ushort bport;
        configuration.getBroadcastEndpoint (bport);

        broadcastTransport_s = new AlpineBroadcastUdpTransport (localIp, htons(bport));

        status = broadcastTransport_s->initialize ();

        if (status) {
            status = broadcastTransport_s->activate ();
        }

        if (status) {
            Log::Info ("Broadcast transport activated on port "s +
                       std::to_string (bport));
        }
        else {
            Log::Error ("Failed to activate broadcast transport.");
            delete broadcastTransport_s;
            broadcastTransport_s = nullptr;
        }
    }

    if (configuration.rawWifiEnabled()) {
        string ifName;
        configuration.getRawWifiInterface (ifName);

        rawWifiTransport_s = new AlpineRawWifiUdpTransport (localIp, localPort, ifName);

        status = rawWifiTransport_s->initialize ();

        if (status) {
            status = rawWifiTransport_s->activate ();
        }

        if (status) {
            Log::Info ("Raw 802.11 transport activated on interface "s + ifName);
        }
        else {
            Log::Error ("Failed to activate raw 802.11 transport.");
            delete rawWifiTransport_s;
            rawWifiTransport_s = nullptr;
        }
    }

    return true;
}



void
AlpineStack::processEvents ()
{
#ifdef _VERBOSE
    Log::Debug ("AlpineStack::processEvents invoked.");
#endif

    auto lastRun = std::chrono::steady_clock::now();

    while (true) {

        // Wait for an event notification or the max poll timeout, whichever
        // comes first.  This replaces the old sleep(1) polling loop and
        // reduces event processing latency from ~1 second to <100ms while
        // still avoiding busy-spinning.
        //
        {
            std::unique_lock lock(eventMutex_s);
            eventCV_s.wait_for(lock,
                               std::chrono::milliseconds(EVENT_LOOP_MAX_WAIT_MS),
                               [] { return eventPending_s; });
            eventPending_s = false;
        }

        // Enforce a minimum interval between iterations to prevent
        // busy-spinning under sustained burst load.
        //
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastRun);

        if (elapsed.count() < EVENT_LOOP_MIN_INTERVAL_MS) {
            std::this_thread::sleep_for(
                std::chrono::milliseconds(EVENT_LOOP_MIN_INTERVAL_MS - elapsed.count()));
        }
        lastRun = std::chrono::steady_clock::now();

        // There are various parts of the Alpine stack which require timed
        // event processing to clean up any timeout conditions and so forth.
        // This is all done from this thread, as all of these processes are
        // fast, and high tolerance.
        //
        AlpinePeerMgr::processTimedEvents ();

        auto completedIds = AlpineQueryMgr::processTimedEvents ();

        if (!completedIds.empty() && completedQueryCallback_s) {
            completedQueryCallback_s(completedIds);
        }
    }
}



void
AlpineStack::notifyEvent ()
{
    {
        std::lock_guard lock(eventMutex_s);
        eventPending_s = true;
    }
    eventCV_s.notify_one();
}



void
AlpineStack::setCompletedQueryCallback (CompletedQueryCallback callback)
{
    completedQueryCallback_s = std::move(callback);
}



void
AlpineStack::cleanUp ()
{
#ifdef _VERBOSE  
    Log::Debug ("AlpineStack::cleanUp invoked.");
#endif

    if (serviceThread_s) {
        serviceThread_s->stop ();
        serviceThread_s = nullptr;  // thread will delete itself when finished.
        return;
    }

    if (configuration_s) {
        delete configuration_s;
        configuration_s = nullptr;
    }

    if (multicastTransport_s) {
        multicastTransport_s->shutdown ();
        delete multicastTransport_s;
        multicastTransport_s = nullptr;
    }

    if (broadcastTransport_s) {
        broadcastTransport_s->shutdown ();
        delete broadcastTransport_s;
        broadcastTransport_s = nullptr;
    }

    if (rawWifiTransport_s) {
        rawWifiTransport_s->shutdown ();
        delete rawWifiTransport_s;
        rawWifiTransport_s = nullptr;
    }

    if (baseUdpTransport_s) {
        baseUdpTransport_s->shutdown ();
        delete baseUdpTransport_s;
        baseUdpTransport_s = nullptr;
    }

    initialized_s = false;
}



bool
AlpineStack::registerTransport (ulong                      peerId,
                                AlpineDtcpConnTransport *  transport)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineStack::registerTransport invoked.  Peer ID: "s +
                std::to_string (peerId));
#endif

#ifdef ALPINE_TLS_ENABLED
    // Mutual TLS verification chokepoint — reject unverified peers
    if (PeerTlsVerifier::isMutualTlsEnabled()) {
        if (!transport->isTlsEnabled()) {
            Log::Error("AlpineStack::registerTransport: mutual TLS required but "
                       "peer transport has no TLS. Rejecting peer "s +
                       std::to_string(peerId));
            return false;
        }

        Log::Info("AlpineStack::registerTransport: peer "s +
                  std::to_string(peerId) + " passed mutual TLS verification"s);
    }
#endif

    // The PeerMgr is the only one that requires this information at the moment...
    //
    bool status;
    status = AlpinePeerMgr::registerTransport (peerId, transport);

    if (status)
        notifyEvent();

    return status;
}



bool  
AlpineStack::handleConnectionClose (ulong  peerId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineStack::handleConnectionClose invoked.  Peer ID: "s +
                std::to_string (peerId));
#endif

    // We need to clean up any queries that may involve this peer.  Dont care
    // about result, failure will probably be due to no peer queries.
    //
    AlpineQueryMgr::cancelAll (peerId);

    AlpinePeerMgr::deletePeer (peerId);

    // MRP_TEMP connect to new peer to fill position?


    return true;
}



bool  
AlpineStack::handleConnectionDeactivate (ulong  peerId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineStack::handleConnectionDeactivate invoked.  Peer ID: "s +
                std::to_string (peerId));
#endif

    // We need to clean up any queries that may involve this peer.  Dont care
    // about result, failure will probably be due to no peer queries.
    //
    AlpineQueryMgr::cancelAll (peerId);

    AlpinePeerMgr::deactivatePeer (peerId);


    return true;
}



bool  
AlpineStack::handleBadDataEvent (ulong  peerId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineStack::handleBadDataEvent invoked.  Peer ID: "s +
                std::to_string (peerId));
#endif


    return true;
}



bool
AlpineStack::handleSendReceived (ulong  peerId,
                                 ulong  requestId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineStack::handleSendReceived invoked.  Peer ID: "s +
                std::to_string (peerId));
#endif

    // MRP_TEMP handle correctly
    AlpineQueryMgr::handleSendReceived (peerId, requestId);


    return true;
}



bool  
AlpineStack::handleSendFailure (ulong  peerId,
                                ulong  requestId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineStack::handleSendFailure invoked.  Peer ID: "s +
                std::to_string (peerId));
#endif

    // MRP_TEMP handle correctly
    AlpineQueryMgr::handleSendFailure (peerId, requestId);


    return true;
}



bool  
AlpineStack::sendReliablePacket (ulong           peerId,
                                 AlpinePacket *  packet,
                                 ulong &         requestId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineStack::sendReliablePacket invoked.  Peer ID: "s +
                std::to_string (peerId));
#endif
  
    bool  status; 
    DtcpBaseConnTransport *  dtcpTransport;
    status = DtcpStack::locateTransport (peerId, dtcpTransport);

    if (!status) {
        Log::Error ("Could not locate transport for peer ID passed in call to "
                             "AlpineStack::sendReliablePacket!");
        return false;
    }
    AlpineDtcpConnTransport *  peerTransport;
    peerTransport = dynamic_cast<AlpineDtcpConnTransport *>(dtcpTransport);

    if (!peerTransport) {
        Log::Error ("Invalid transport type returned by DtcpStack in call to "
                             "AlpineStack::sendReliablePacket!");
        return false;
    }
    // Write packet data and send to transport
    //
    // MRP_TEMP implement constants!
    DataBuffer * buffer = new DataBuffer (1024*34);
    buffer->writeReset ();

    status = packet->writeData (buffer);

    if (!status) {
        Log::Error ("Could not write packet data in call to "
                             "AlpineStack::sendReliablePacket!");
        return false;
    }
    byte * data;
    uint   length;
    buffer->readReset ();
    buffer->getReadBuffer (data, length);

    status = peerTransport->sendReliableData (data,
                                              length,
                                              requestId);

    if (!status) {
        Log::Error ("sendReliableData for response packet data failed in call to "
                             "AlpineStack::sendReliablePacket!");
        return false;
    }
    return true;
}



bool  
AlpineStack::acknowledgeTransfer (ulong  peerId,
                                  ulong  requestId)
{
#ifdef _VERBOSE
    Log::Debug ("AlpineStack::acknowledgeTransfer invoked."s +
                "\n Peer ID: "s + std::to_string (peerId) +
                "\n Request ID: "s + std::to_string (requestId) +
                "\n");
#endif

    // Locate AlpineDtcpConnTransport and invoke acknowledgement.
    //
    bool  status; 
    DtcpBaseConnTransport *  dtcpTransport;
    status = DtcpStack::locateTransport (peerId, dtcpTransport);

    if (!status) {
        Log::Error ("Could not locate transport for peer ID passed in call to "
                             "AlpineStack::acknowledgeTransfer!");
        return false;
    }
    AlpineDtcpConnTransport *  peerTransport;
    peerTransport = dynamic_cast<AlpineDtcpConnTransport *>(dtcpTransport);

    if (!peerTransport) {
        Log::Error ("Invalid transport type returned by DtcpStack in call to "
                             "AlpineStack::acknowledgeTransfer!");
        return false;
    }
    peerTransport->acknowledgeTransfer (requestId);


    return true;
}   



