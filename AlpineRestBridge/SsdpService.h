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


class SsdpService : public SysThread
{
  public:

    SsdpService ();
    ~SsdpService ();

    bool  initialize (const string & deviceUuid,
                      const string & locationUrl,
                      const string & serverString);

    void  threadMain ();


  private:

    void  sendNotifyAlive ();
    void  sendNotifyByebye ();
    void  handleMSearch (const byte * data, ulong len,
                         ulong srcIp, ushort srcPort);

    static const int   NOTIFY_INTERVAL_SEC = 30;
    static const int   POLL_TIMEOUT_MS     = 1000;
    static const ulong SSDP_MULTICAST_ADDR = 0xEFFFFFFA; // 239.255.255.250
    static const ushort SSDP_PORT          = 1900;

    int     ssdpFd_;
    string  deviceUuid_;
    string  locationUrl_;
    string  serverString_;

};
