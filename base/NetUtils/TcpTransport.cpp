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


#include <TcpTransport.h>
#include <Log.h>
#include <StringUtils.h>
#include <NetUtils.h>



TcpTransport::TcpTransport (int     socketFd,
                            ulong   destIpAddress,
                            ushort  destPort,
                            ulong   localIpAddress,
                            ushort  localPort)
    : socketFd_(socketFd),
      destIpAddress_(destIpAddress),
      destPort_(destPort),
      localIpAddress_(localIpAddress),
      localPort_(localPort)
{
#ifdef _VERBOSE
    Log::Debug ("TcpTransport constructor invoked.");
#endif
}



TcpTransport::~TcpTransport ()
{
#ifdef _VERBOSE
    Log::Debug ("TcpTransport destructor invoked.");
#endif

    if (socketFd_ >= 0) 
        alpine_close_socket(socketFd_);
}



bool 
TcpTransport::getDestination (ulong &   ipAddress,
                              ushort &  port)
{
#ifdef _VERBOSE
    Log::Debug ("TcpTransport::getDestination invoked.");
#endif

    if (socketFd_ < 0) {
        Log::Error ("Call to TcpTransport::getDestination when connection closed!");
        return false;
    }
    ipAddress = destIpAddress_;
    port      = destPort_;

    return true;
}



bool 
TcpTransport::getEndpoint (ulong &   ipAddress,
                           ushort &  port)
{
#ifdef _VERBOSE
    Log::Debug ("TcpTransport::getEndpoint invoked.");
#endif

    if (socketFd_ < 0) {
        Log::Error ("Call to TcpTransport::getEndpoint when connection closed!");
        return false;
    }
    ipAddress = localIpAddress_;
    port      = localPort_;

    return true;
}



void 
TcpTransport::close ()
{
#ifdef _VERBOSE
    Log::Debug ("TcpTransport::close invoked.");
#endif

    if (socketFd_ >= 0) {
        alpine_close_socket(socketFd_);
        socketFd_ = -1;
    }
}



int  
TcpTransport::getFd ()
{
#ifdef _VERBOSE
    Log::Debug ("TcpTransport::getFd invoked.");
#endif

    if (socketFd_ < 0) {
        Log::Error ("Call to TcpTransport::getFd when connection closed!");
        return false;
    }

    return socketFd_;
}



bool 
TcpTransport::nonBlocking ()
{
#ifdef _VERBOSE
    Log::Debug ("TcpTransport::nonBlocking invoked.");
#endif

    if (socketFd_ < 0) {
        Log::Error ("Call to TcpTransport::nonBlocking when connection closed!");
        return false;
    }
    bool status;
    status = NetUtils::nonBlocking (socketFd_);


    return status;
}



bool 
TcpTransport::blocking ()
{
#ifdef _VERBOSE
    Log::Debug ("TcpTransport::blocking invoked.");
#endif

    if (socketFd_ < 0) {
        Log::Error ("Call to TcpTransport::blocking when connection closed!");
        return false;
    }
    bool status;
    status = NetUtils::blocking (socketFd_);


    return status;
}



bool 
TcpTransport::send (const byte *  data,
                    ulong         dataLength)
{
#ifdef _VERBOSE
    Log::Debug ("TcpTransport::send invoked.");
#endif

    if (socketFd_ < 0) {
        Log::Error ("Call to TcpTransport::send when connection closed!");
        return false;
    }
    int retVal;
    retVal = ::send (socketFd_, data, dataLength, 0);

    if (retVal < 0) {
        if (alpine_socket_errno() == EAGAIN) {
            return false;
        }
        string errorCode;
        NetUtils::socketErrorAsString (alpine_socket_errno(), errorCode);
        Log::Error ("Send error: "s + errorCode +
                    " in TcpTransport::send!");

        return false;
    }
    return true;
}



bool 
TcpTransport::receive (byte *   buffer,
                       ulong    bufferSize,
                       ulong &  dataLength)
{
#ifdef _VERBOSE
    Log::Debug ("TcpTransport::receive invoked.");
#endif

    if (socketFd_ < 0) {
        Log::Error ("Call to TcpTransport::receive when connection closed!");
        return false;
    }
    int retVal;
    retVal = ::recv (socketFd_, buffer, bufferSize, 0);

    if (retVal < 0) {
        if (alpine_socket_errno() == EAGAIN) {
            return false;
        }
        string errorCode;
        NetUtils::socketErrorAsString (alpine_socket_errno(), errorCode);
        Log::Error ("Receive error: "s + errorCode +
                    " in TcpTransport::receive!");

        return false;
    }
    dataLength = retVal;


    return true;
}



