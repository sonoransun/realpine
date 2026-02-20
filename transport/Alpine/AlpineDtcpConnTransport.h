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
#include <DtcpBaseConnTransport.h>
#include <OptHash.h>
#include <list>


class AlpinePacket;
class DataBuffer;
class DataBlock;


class AlpineDtcpConnTransport : public DtcpBaseConnTransport
{
  public:
    AlpineDtcpConnTransport ();
    
    virtual ~AlpineDtcpConnTransport ();



    virtual bool handleData (const byte * data,
                             uint         dataLength);

    virtual bool handleConnectionClose ();

    virtual bool handleSendReceived ();

    virtual bool handleSendFailure ();

    virtual bool handleConnectionDeactivate ();


    // Provide some methods used for reliable transfer of data.
    // The Alpine stack requires a queue for reliable transfers which
    // is implemented at this layer.  The underlying DTCP transports
    // only keep track of a single reliable request.
    //
    virtual bool sendReliableData (const byte * data,
                                   uint         dataLength,
                                   ulong        requestId);

    virtual bool acknowledgeTransfer (ulong  requestId);



    // Types and data structures for management of reliable transfer queue
    //
    struct t_ReliableRequest {
        ulong        requestId;
        DataBlock *  data;
    };

    using t_RequestList = list<t_ReliableRequest *>;

    struct t_RequestStateIndex {
        t_RequestList  pendingRequests;
        ulong          currRequestId;
    };
        


  private:

    bool  processAlpineQueryPacket (AlpinePacket *  packet,
                                    DataBuffer *    buffer);

    bool  processAlpinePeerPacket (AlpinePacket *  packet,
                                    DataBuffer *    buffer);

    bool  processAlpineProxyPacket (AlpinePacket *  packet,
                                    DataBuffer *    buffer);

    void  checkPendingRequests ();

    void  cleanUp ();

    

    ulong                  requestId_;
    t_RequestStateIndex *  pendingIndex_;  // this is usually unused.  Only allocate if needed.
};


