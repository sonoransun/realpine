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


#include <TcpClientThread.h>
#include <TcpTransport.h>
#include <Log.h>


TcpClientThread::TcpClientThread (TcpTransport *  transport)
{
#ifdef _VERBOSE
    Log::Debug ("TcpClientThread constructor invoked.");
#endif

    transport_ = transport;
}



TcpClientThread::~TcpClientThread ()
{
#ifdef _VERBOSE
    Log::Debug ("TcpClientThread destructor invoked.");
#endif
}



void 
TcpClientThread::threadMain ()
{
#ifdef _VERBOSE
    Log::Debug ("TcpClientThread::threadMain invoked.");
#endif

    // The sole purpose in the life of this thread is to send and receive as much as possible.
    //
    bool  status;
    int   fd;

    fd = transport_->getFd ();

    status = transport_->blocking ();

    if (!status) {
        Log::Error ("Could not put transport in blocking mode!");
        delete transport_;
        return;
    }

    byte * buffer;
    ulong  bufferSize = 1024;
    ulong  dataLength;

    buffer = new byte[bufferSize];
    memset (buffer, '*', bufferSize);


    // Loop forever performing one send, then one receive, ad infinitum
    //
    while (true) {
        status = transport_->send (buffer, bufferSize);

        if (!status) {
            Log::Error ("Transport send failed!");
            delete transport_;
            return;
        }

        status = transport_->receive (buffer, bufferSize, dataLength);

        if (!status) {
            Log::Error ("Transport receive failed!");
            delete transport_;
            return;
        }
    }
}



