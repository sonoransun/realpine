/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <AlpineStackConfig.h>
#include <ReadWriteSem.h>


class AlpinePacket;
class AlpineServiceThread;
class AlpineDtcpUdpTransport;
class AlpineDtcpConnTransport;
class DtcpBaseUdpTransport;


class AlpineStack
{
  public:

    AlpineStack () = default;
    ~AlpineStack () = default;


    static bool  initialize (AlpineStackConfig &  configuration);


  private:

    static bool                       initialized_s;
    static AlpineServiceThread *      serviceThread_s;
    static AlpineStackConfig *        configuration_s;
    static AlpineDtcpUdpTransport *   baseUdpTransport_s;  // primary unicast endpoint
    static DtcpBaseUdpTransport *     multicastTransport_s;
    static DtcpBaseUdpTransport *     broadcastTransport_s;
    static DtcpBaseUdpTransport *     rawWifiTransport_s;
    static ReadWriteSem               dataLock_s;


    static void  processEvents ();

    static void  cleanUp ();


    // The AlpineDtcpConn classes use the AlpineStack to coordinate
    // processing of requests and responses.
    //
    // The following methods provide the entry points used by the derived
    // classes to handle various events they receive from the DTPC stack.
    //
    static bool  registerTransport (ulong                      peerId,
                                    AlpineDtcpConnTransport *  transport);

    static bool  handleConnectionClose (ulong  peerId);

    static bool  handleConnectionDeactivate (ulong  peerId);

    static bool  handleBadDataEvent (ulong  peerId);

    static bool  handleSendReceived (ulong  peerId,
                                     ulong  requestId);

    static bool  handleSendFailure (ulong  peerId,
                                    ulong  requestId);


    // Utility methods for various AlpineStack components.
    //
    static bool  sendReliablePacket (ulong           peerId,
                                     AlpinePacket *  packet,
                                     ulong &         requestId);

    static bool  acknowledgeTransfer (ulong  peerId,
                                      ulong  requestId);



    friend class AlpineDtcpConnConnector;
    friend class AlpineDtcpConnAcceptor;
    friend class AlpineDtcpConnTransport;
    friend class AlpineServiceThread;
    friend class AlpineQueryMgr;
};

