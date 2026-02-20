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

class TcpTransport;


class TorHiddenService
{
  public:

    TorHiddenService () = default;
    ~TorHiddenService ();

    bool initialize (ushort localTargetPort,
                     ushort torControlPort,
                     ushort onionVirtualPort,
                     const string & controlAuth);
    void shutdown ();

    const string & onionAddress () const;
    bool isActive () const;


  private:

    bool authenticate (const string & auth);
    bool createHiddenService (ushort virtualPort, ushort targetPort);
    bool sendCommand (const string & command, string & response);
    bool readResponse (string & response);

    TcpTransport * controlConn_{nullptr};
    string         serviceId_;
    string         onionAddress_;
    bool           active_{false};
};
