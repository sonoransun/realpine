///////
///
///  Copyright (C) 2026  sonoransun
///
///  Permission is hereby granted, free of charge, to any person obtaining a copy
///  of this software and associated documentation files (the "Software"), to deal
///  in the Software without restriction, including without limitation the rights
///  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
///  copies of the Software, and to permit persons to whom the Software is
///  furnished to do so, subject to the following conditions:
///
///  The above copyright notice and this permission notice shall be included in all
///  copies or substantial portions of the Software.
///
///  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
///  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
///  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
///  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
///  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
///  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
///  SOFTWARE.
///
///////


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
#include <DtcpStack.h>
#include <DataBuffer.h>
#include <Log.h>
#include <StringUtils.h>
#include <WriteLock.h>
#include <ReadLock.h>
#include <Platform.h>



bool                              AlpineStack::initialized_s = false;
AlpineServiceThread *             AlpineStack::serviceThread_s = nullptr;
AlpineStackConfig *               AlpineStack::configuration_s = nullptr;
AlpineDtcpUdpTransport *          AlpineStack::baseUdpTransport_s = nullptr;
ReadWriteSem                      AlpineStack::dataLock_s;

static const int   eventIterationDelay = 1; // one second delay between interations



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
    return true;
}



void  
AlpineStack::processEvents ()
{
#ifdef _VERBOSE
    Log::Debug ("AlpineStack::processEvents invoked.");
#endif

    while (true) {

        // There are various parts of the Alpine stack which require timed
        // event processing to clean up any timeout conditions and so forth.
        // This is all done from this thread, as all of these processes are
        // fast, and high tolerance.
        //
        AlpinePeerMgr::processTimedEvents ();
    
        AlpineQueryMgr::processTimedEvents ();


        // Delay for the configured period of time before looping through events again...
        //
        sleep (eventIterationDelay);
    }
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

    // The PeerMgr is the only one that requires this information at the moment...
    //
    bool status;
    status = AlpinePeerMgr::registerTransport (peerId, transport);


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



