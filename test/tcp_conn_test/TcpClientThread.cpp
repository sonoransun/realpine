/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <Log.h>
#include <TcpClientThread.h>
#include <TcpTransport.h>

#include <cstring>


TcpClientThread::TcpClientThread(TcpTransport * transport)
{
#ifdef _VERBOSE
    Log::Debug("TcpClientThread constructor invoked.");
#endif

    transport_ = transport;
}


TcpClientThread::~TcpClientThread()
{
#ifdef _VERBOSE
    Log::Debug("TcpClientThread destructor invoked.");
#endif
}


void
TcpClientThread::threadMain()
{
#ifdef _VERBOSE
    Log::Debug("TcpClientThread::threadMain invoked.");
#endif

    // The sole purpose in the life of this thread is to send and receive as much as possible.
    //
    bool status;
    int fd;

    fd = transport_->getFd();

    status = transport_->blocking();

    if (!status) {
        Log::Error("Could not put transport in blocking mode!");
        delete transport_;
        return;
    }

    byte * buffer;
    ulong bufferSize = 1024;
    ulong dataLength;

    buffer = new byte[bufferSize];
    memset(buffer, '*', bufferSize);


    // Loop forever performing one send, then one receive, ad infinitum
    //
    while (true) {
        status = transport_->send(buffer, bufferSize);

        if (!status) {
            Log::Error("Transport send failed!");
            delete transport_;
            return;
        }

        status = transport_->receive(buffer, bufferSize, dataLength);

        if (!status) {
            Log::Error("Transport receive failed!");
            delete transport_;
            return;
        }
    }
}
