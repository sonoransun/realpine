/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <Log.h>
#include <StringUtils.h>
#include <TcpAcceptor.h>
#include <TcpTransport.h>
#include <cstring>
#include <memory>

#ifndef ALPINE_PLATFORM_WINDOWS
#include <fcntl.h>
#endif


static const int maxPendingAccept_s = 32;


TcpAcceptor::TcpAcceptor()
{
#ifdef _VERBOSE
    Log::Debug("TcpAcceptor constructor invoked.");
#endif

    socketFd_ = -1;
    ipAddress_ = 0;
    port_ = 0;
    addressSize_ = sizeof(socketAddress_);
    memset(&socketAddress_, 0, sizeof(socketAddress_));
}


TcpAcceptor::~TcpAcceptor()
{
#ifdef _VERBOSE
    Log::Debug("TcpAcceptor destructor invoked.");
#endif

    if (socketFd_ >= 0)
        alpine_close_socket(socketFd_);
}


bool
TcpAcceptor::create(ulong ipAddress, ushort port)
{
#ifdef _VERBOSE
    Log::Debug("TcpAcceptor::create invoked.");
#endif

    if (socketFd_ >= 0) {
        alpine_close_socket(socketFd_);
    }

    // Create and bind our socket for this local endpoint address.
    // SOCK_CLOEXEC avoids a small race where a forked child could
    // otherwise inherit the listening socket across exec().
    //
    socketFd_ = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if (socketFd_ < 0) {
        socketFd_ = -1;

        string errorCode;
        NetUtils::socketErrorAsString(alpine_socket_errno(), errorCode);
        Log::Error("Socket create error: "s + errorCode + " in TcpAcceptor::create.");

        return false;
    }
    int retVal;
    socketAddress_.sin_family = AF_INET;
    socketAddress_.sin_addr.s_addr = ipAddress;
    socketAddress_.sin_port = port;

    retVal = bind(socketFd_, reinterpret_cast<const struct sockaddr *>(&socketAddress_), addressSize_);
    if (retVal < 0) {
        alpine_close_socket(socketFd_);
        socketFd_ = -1;

        string errorCode;
        NetUtils::socketErrorAsString(alpine_socket_errno(), errorCode);
        Log::Error("Socket bind error: "s + errorCode + " in TcpAcceptor::create.");

        return false;
    }
    retVal = listen(socketFd_, maxPendingAccept_s);
    if (retVal < 0) {
        alpine_close_socket(socketFd_);
        socketFd_ = -1;

        string errorCode;
        NetUtils::socketErrorAsString(alpine_socket_errno(), errorCode);
        Log::Error("Socket accept error: "s + errorCode + " in TcpAcceptor::create.");

        return false;
    }
    return true;
}


bool
TcpAcceptor::getEndpoint(ulong & ipAddress, ushort & port)
{
#ifdef _VERBOSE
    Log::Debug("TcpAcceptor::getEndpoint invoked.");
#endif

    ipAddress = ipAddress_;
    port = port_;

    return true;
}


void
TcpAcceptor::close()
{
#ifdef _VERBOSE
    Log::Debug("TcpAcceptor::close invoked.");
#endif

    if (socketFd_ >= 0) {
        alpine_close_socket(socketFd_);
        socketFd_ = -1;
    }
}


int
TcpAcceptor::getFd()
{
#ifdef _VERBOSE
    Log::Debug("TcpAcceptor::getFd invoked.");
#endif

    return socketFd_;
}


bool
TcpAcceptor::nonBlocking()
{
#ifdef _VERBOSE
    Log::Debug("TcpAcceptor::nonBlocking invoked.");
#endif

    if (socketFd_ < 0) {
        Log::Error("TcpAcceptor::nonBlocking invoked without created socket!\n");
        return false;
    }
    bool status;
    status = NetUtils::nonBlocking(socketFd_);

    return status;
}


bool
TcpAcceptor::blocking()
{
#ifdef _VERBOSE
    Log::Debug("TcpAcceptor::blocking invoked.");
#endif

    if (socketFd_ < 0) {
        Log::Error("TcpAcceptor::blocking invoked without created socket!\n");
        return false;
    }
    bool status;
    status = NetUtils::blocking(socketFd_);

    return status;
}


bool
TcpAcceptor::accept(std::unique_ptr<TcpTransport> & transport)
{
#ifdef _VERBOSE
    Log::Debug("TcpAcceptor::accept invoked.");
#endif

    int connFd;
    ulong destIp;
    ushort destPort;

    struct sockaddr_in destAddr;
    socklen_t destAddrSize = sizeof(destAddr);

#ifdef ALPINE_HAVE_ACCEPT4
    // Atomic accept + set-CLOEXEC on Linux — no race window.
    //
    connFd = ::accept4(socketFd_, reinterpret_cast<struct sockaddr *>(&destAddr), &destAddrSize, SOCK_CLOEXEC);
#else
    connFd = ::accept(socketFd_, reinterpret_cast<struct sockaddr *>(&destAddr), &destAddrSize);
#ifndef ALPINE_PLATFORM_WINDOWS
    if (connFd >= 0) {
        // Fallback: set FD_CLOEXEC via fcntl.  There is a small race
        // between accept() and fcntl() where a concurrent exec() in
        // another thread could leak this fd, but this only runs on
        // platforms without accept4().
        //
        int flags = ::fcntl(connFd, F_GETFD);
        if (flags >= 0) {
            ::fcntl(connFd, F_SETFD, flags | FD_CLOEXEC);
        }
    }
#endif
#endif

    if (connFd < 0) {
        if (alpine_socket_errno() == EAGAIN) {
            return false;
        }
        string errorCode;
        NetUtils::socketErrorAsString(alpine_socket_errno(), errorCode);
        Log::Error("Accept error: "s + errorCode + " in TcpAcceptor::accept.");

        return false;
    }
    destIp = destAddr.sin_addr.s_addr;
    destPort = destAddr.sin_port;

    transport.reset(new TcpTransport(connFd, destIp, destPort, ipAddress_, port_));


    return true;
}
