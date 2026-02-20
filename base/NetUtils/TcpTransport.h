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
#include <Platform.h>
#include <NetUtils.h>


class TcpTransport
{
  public:

    ~TcpTransport ();


    bool getDestination (ulong &   ipAddress,
                         ushort &  port);

    bool getEndpoint (ulong &   ipAddress,
                      ushort &  port);

    void close ();

    int  getFd ();

    bool nonBlocking ();

    bool blocking ();

    bool send (const byte *  data,
               ulong         dataLength);

    bool receive (byte *   buffer,
                  ulong    bufferSize,
                  ulong &  dataLength);



  private:

    TcpTransport (int     socketFd,
                  ulong   destIpAddress,
                  ushort  destPort,
                  ulong   localIpAddress,
                  ushort  localPort);


    int       socketFd_;
    ulong     destIpAddress_;
    ushort    destPort_;
    ulong     localIpAddress_;
    ushort    localPort_;


    friend class TcpAcceptor;
    friend class TcpConnector;
    friend class TcpAsyncConnector;
};

