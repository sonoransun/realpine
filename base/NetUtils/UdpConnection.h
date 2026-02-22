/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <Platform.h>
#include <NetUtils.h>


class UdpConnection
{
  public:

    UdpConnection ();
    virtual ~UdpConnection ();


    virtual bool create (ulong  ipAddress = 0,
                         ushort port = 0);

    virtual void close ();

    virtual int  getFd ();

    virtual bool nonBlocking ();

    virtual bool blocking ();

    virtual bool sendData (const ulong    destIpAddress,
                           const ushort   destPort,
                           const byte *   dataBuffer,
                           const uint     dataLength);

    // Only valid immediately after failed send
    //
    virtual bool sendBufferFull ();

    virtual bool receiveData (byte *     dataBuffer,
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


