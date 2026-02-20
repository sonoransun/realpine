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
#include <ReadWriteSem.h>
#include <list>


class DtcpBaseConnTransport;
class StackLinkInterface;


class DtcpBroadcastStates
{
  public:

    DtcpBroadcastStates ();
    ~DtcpBroadcastStates ();


    using t_TransportList = list<DtcpBaseConnTransport *>;


    bool addState (ulong                 requestId,
                   StackLinkInterface *  packet,
                   t_TransportList *     destinations);

    // Expensive operation, use cautiously.
    //
    bool removeState (ulong  requestId);


    ulong  requestsPending ();



    // Internal types
    //
    struct t_RequestData {
        ulong                 requestId;
        StackLinkInterface *  packet;
        t_TransportList *     remainingDestinations;
    };

    using t_RequestList = list<t_RequestData *>;

    
  private:

    ulong             totalRequests_;
    t_RequestList     requestList_;
    ReadWriteSem      dataLock_;


    bool  getCurrentRequest (StackLinkInterface *&     packet,
                             DtcpBaseConnTransport *&  destination);


    friend class DtcpSendQueue;
};

