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


class MdnsService : public SysThread
{
  public:

    MdnsService ();
    ~MdnsService ();

    bool  initialize (const string & hostname,
                      const string & ipAddress,
                      ushort port);

    void  threadMain ();


  private:

    void   sendAnnouncement ();
    ulong  writeDnsName (byte * buf, ulong offset, const string & name);
    ulong  writePtrRecord (byte * buf, ulong offset,
                           const string & serviceName, const string & instanceName);
    ulong  writeSrvRecord (byte * buf, ulong offset,
                           const string & instanceName, const string & target,
                           ushort port);
    ulong  writeTxtRecord (byte * buf, ulong offset,
                           const string & instanceName, const string & txt);
    ulong  writeARecord (byte * buf, ulong offset,
                         const string & hostname, ulong ipAddr);

    static const int    ANNOUNCE_INTERVAL_SEC = 60;
    static const int    POLL_TIMEOUT_MS       = 1000;
    static const ushort MDNS_PORT             = 5353;

    int     mdnsFd_;
    string  hostname_;
    string  ipAddress_;
    ulong   ipAddrBinary_;
    ushort  port_;

};
