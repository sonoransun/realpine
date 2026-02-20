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


#pragma once
#include <Common.h>
#include <AlpineStackConfig.h>
#include <ReadWriteSem.h>


class AlpinePacket;
class AlpineServiceThread;
class AlpineDtcpUdpTransport;
class AlpineDtcpConnTransport;


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
    static AlpineDtcpUdpTransport *   baseUdpTransport_s;  // for now support a single endpoint
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

