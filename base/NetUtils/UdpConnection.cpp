/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <Log.h>
#include <NetUtils.h>
#include <UdpConnection.h>
#include <CovertChannel.h>



UdpConnection::UdpConnection ()
{
    connectionFd_ = -1;

    addressSize_ = sizeof(connectionAddress_);
    memset (&connectionAddress_, 0, addressSize_);
    connectionAddress_.sin_family = AF_INET;
}



UdpConnection::~UdpConnection ()
{
    close();
}



bool 
UdpConnection::create (ulong  ipAddress,
                       ushort port)
{
    if (connectionFd_ >= 0) {
        close ();
    }

    connectionFd_ = socket(AF_INET, SOCK_DGRAM, 0);

    if (connectionFd_ < 0) {
        string errorCode;
        NetUtils::socketErrorAsString (alpine_socket_errno(), errorCode);
        Log::Error ("Socket create error: "s + errorCode +
                    " in UdpConnection::create.");

        return false;
    }
    const int receiveSocketBufferSize = 262144; // 256k

    if ( setsockopt(connectionFd_, 
                    SOL_SOCKET,
                    SO_RCVBUF,
                    reinterpret_cast<const byte *>(&receiveSocketBufferSize),
                    sizeof(int)) < 0) {

        string errorCode;
        NetUtils::socketErrorAsString (alpine_socket_errno(), errorCode);
        Log::Error ("Set receive buffer size socket option error: "s + errorCode +
                    " in UdpConnection::create.");

        close();
        return false;
    }
    const int sendSocketBufferSize = 65536; // 64k
 
    if ( setsockopt(connectionFd_,
                    SOL_SOCKET,
                    SO_SNDBUF,
                    reinterpret_cast<const byte *>(&sendSocketBufferSize),
                    sizeof(int)) < 0) {

        string errorCode;
        NetUtils::socketErrorAsString (alpine_socket_errno(), errorCode);
        Log::Error ("Set send buffer size socket option error: "s + errorCode +
                    " in UdpConnection::create.");

        close();
        return false;
    }
    if (port > 0) {
        connectionAddress_.sin_port   = port;
    }

    if (ipAddress > 0) {
        connectionAddress_.sin_addr.s_addr = ipAddress;
    }


    if ( bind(connectionFd_,
              reinterpret_cast<const struct sockaddr *>(&connectionAddress_),
              addressSize_) < 0) {

        string errorCode;
        NetUtils::socketErrorAsString (alpine_socket_errno(), errorCode);
        Log::Error ("Bind error: "s + errorCode +
                    " in UdpConnection::create.");

        close();
        return false;
    }
    // By default we put the socket in non-blocking mode...
    //
    nonBlocking ();


    return true;
}



void 
UdpConnection::close ()
{
    if (connectionFd_ >= 0) {
        alpine_close_socket(connectionFd_);
        connectionFd_ = -1;
    }
}



int
UdpConnection::getFd ()
{
    return connectionFd_;
}



bool 
UdpConnection::nonBlocking ()
{
#ifdef _VERBOSE
    Log::Debug ("UdpConnection::nonBlocking invoked.");
#endif

    bool  result;
    result = NetUtils::nonBlocking (connectionFd_);

    return result;
}



bool 
UdpConnection::blocking ()
{
#ifdef _VERBOSE
    Log::Debug ("UdpConnection::blocking invoked.");
#endif

    bool  result;
    result = NetUtils::blocking (connectionFd_);

    return result;
}



bool 
UdpConnection::sendData (const ulong    destIpAddress,
                         const ushort   destPort,
                         const byte *   dataBuffer,
                         const uint     dataLength)
{
    if (connectionFd_ < 0) {
        return false;
    }
    connectionAddress_.sin_addr.s_addr = destIpAddress;
    connectionAddress_.sin_port = destPort;
    
    const byte* sendBuffer = dataBuffer;
    vector<byte> obfuscatedBuffer;

    if (CovertChannel::isEnabled()) {
        obfuscatedBuffer.assign(dataBuffer, dataBuffer + dataLength);
        CovertChannel::obfuscate(obfuscatedBuffer.data(), dataLength);
        sendBuffer = obfuscatedBuffer.data();
    }

    int result;
    result = sendto(connectionFd_,
                    sendBuffer,
                    dataLength,
                    0,  // No options
                    reinterpret_cast<const struct sockaddr *>(&connectionAddress_),
                    addressSize_);

    if (result < 0) {
        if (alpine_socket_errno() == EAGAIN) {
            // non block and send buffer is full
            //
            sendBufferFull_ = true;
            return false;
        }
        string errorCode;
        NetUtils::socketErrorAsString (alpine_socket_errno(), errorCode);
        Log::Error ("Send error: "s + errorCode +
                    " in UdpConnection::sendData.");

        return false;
    }
    sendBufferFull_ = false;

    if (result != (int)dataLength) {
        return false;
    }
    return true;
}



bool 
UdpConnection::sendBufferFull ()
{
    return sendBufferFull_;
}



bool 
UdpConnection::receiveData (byte *     dataBuffer,
                            const uint bufferLength,
                            ulong &    sourceIpAddress,
                            ushort &   sourcePort,
                            uint &     dataLength)
{
    if (connectionFd_ < 0) {
        return false;
    }
    socklen_t incomingAddrSize = sizeof(connectionAddress_);

    int result;
    result = recvfrom (connectionFd_,
                       dataBuffer,
                       bufferLength,
                       0,  // No options
                       reinterpret_cast<struct sockaddr *>(&connectionAddress_),
                       &incomingAddrSize);

    if (result < 0) {
        if (alpine_socket_errno() == EAGAIN) {
            // non block and no data waiting, bad poll?
            return false;
        }
        string errorCode;
        NetUtils::socketErrorAsString (alpine_socket_errno(), errorCode);
        Log::Error ("Receive error: "s + errorCode +
                    " in UdpConnection::receiveData.");

        return false;    
    }
    dataLength = (uint)result;

    if (CovertChannel::isEnabled()) {
        CovertChannel::deobfuscate(dataBuffer, dataLength);
    }

    sourceIpAddress = static_cast<ulong>(connectionAddress_.sin_addr.s_addr);
    sourcePort = connectionAddress_.sin_port;

    return true;
}



