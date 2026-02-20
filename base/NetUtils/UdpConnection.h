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


class UdpConnection
{
  public:

    UdpConnection ();
    ~UdpConnection ();


    bool create (ulong  ipAddress = 0,
                 ushort port = 0);

    void close ();

    int  getFd ();

    bool nonBlocking ();

    bool blocking ();

    bool sendData (const ulong    destIpAddress,
                   const ushort   destPort,
                   const byte *   dataBuffer,
                   const uint     dataLength);

    // Only valid immediately after failed send
    //
    bool sendBufferFull ();

    bool receiveData (byte *     dataBuffer,
                      const uint bufferLength,
                      ulong &    sourceIpAddress,
                      ushort &   sourcePort,
                      uint &     dataLength);

  private:

    int                 connectionFd_;
    struct sockaddr_in  connectionAddress_;
    int                 addressSize_;
    bool                sendBufferFull_;

};


