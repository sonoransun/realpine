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


#include <TcpConnector.h>
#include <TcpTransport.h>
#include <Log.h>
#include <StringUtils.h>



TcpConnector::TcpConnector ()
{
#ifdef _VERBOSE
    Log::Debug ("TcpConnector constructor invoked.");
#endif

    destIpAddress_ = 0;
    destPort_      = 0;
    addressSize_   = sizeof(socketAddress_);
    memset (&socketAddress_, 0, sizeof(socketAddress_));
}



TcpConnector::~TcpConnector ()
{
#ifdef _VERBOSE
    Log::Debug ("TcpConnector destructor invoked.");
#endif
}



bool 
TcpConnector::setDestination (ulong   ipAddress,
                              ushort  port)
{
#ifdef _VERBOSE
    Log::Debug ("TcpConnector::setDestination invoked.");
#endif

    if (ipAddress == 0) {
        Log::Error ("Invalid IP address passed in call to TcpConnector::setDestination!");
        return false;
    }

    destIpAddress_ = ipAddress;
    destPort_      = port;
    addressSize_   = sizeof(socketAddress_);

    socketAddress_.sin_family      = AF_INET;
    socketAddress_.sin_addr.s_addr = destIpAddress_;
    socketAddress_.sin_port        = destPort_;


    return true;
}



bool 
TcpConnector::getDestination (ulong &   ipAddress,
                              ushort &  port)
{
#ifdef _VERBOSE
    Log::Debug ("TcpConnector::getDestination invoked.");
#endif

    ipAddress = destIpAddress_;
    port      = destPort_;

    return true;
}



bool 
TcpConnector::connect (TcpTransport *&  transport)
{
#ifdef _VERBOSE
    Log::Debug ("TcpConnector::connect invoked.");
#endif

    // Create local socket, then connect.
    //
    int socketFd;
    socketFd = socket (AF_INET, SOCK_STREAM, 0);
    if (socketFd < 0) {
        string errorCode;
        NetUtils::socketErrorAsString (alpine_socket_errno(), errorCode);
        Log::Error ("Create socket error: "s + errorCode +
                    " in TcpConnector::connect!");

        return false;
    }

    int retVal;
    retVal = ::connect (socketFd,
                        reinterpret_cast<struct sockaddr *>(&socketAddress_),
                        addressSize_);

    if (retVal < 0) {
        string errorCode;
        NetUtils::socketErrorAsString (alpine_socket_errno(), errorCode);
        Log::Error ("Connect error: "s + errorCode +
                    " in TcpConnector::connect!");

        return false;
    }


    bool  status;
    ulong  localIp;
    ushort localPort;

    status = NetUtils::getLocalEndpoint (socketFd, localIp, localPort);

    if (!status) {
        Log::Error ("Unable to get local endpoint address in call to "
                             "TcpConnector::connect!");
        return false;
    }


    // Allocate new TCP transport with this connected socket
    //
    transport = new TcpTransport (socketFd,
                                  destIpAddress_,
                                  destPort_,
                                  localIp,
                                  localPort);
                          

    return true;
}



