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
#include <SysThread.h>
#include <UdpConnection.h>
#include <AlpineStackInterface.h>


class BroadcastQueryHandler : public SysThread
{
  public:

    BroadcastQueryHandler () = default;
    ~BroadcastQueryHandler ();

    bool initialize (ushort listenPort);

    void threadMain ();


  private:

    void handleMessage (const string & message, ulong senderIp, ushort senderPort);
    void handleQuery (const string & message, ulong senderIp, ushort senderPort);
    void handleCancel (const string & message);
    void sendResponse (ulong queryId, ulong destIp, ushort destPort,
                       const AlpineStackInterface::t_PeerResourcesIndex & results);

    static constexpr int RECV_TIMEOUT_MS  = 1000;
    static constexpr int QUERY_POLL_MS    = 100;
    static constexpr int QUERY_TIMEOUT_MS = 15000;
    static constexpr int RECV_BUFFER_SIZE = 4096;

    UdpConnection  udpSocket_;
    ushort         listenPort_{0};
    string         responderId_;

};
