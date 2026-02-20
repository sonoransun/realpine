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


#include <TcpAsyncConnector.h>
#include <TcpTransport.h>
#include <NetUtils.h>
#include <Log.h>
#include <StringUtils.h>



TcpAsyncConnector::TcpAsyncConnector ()
{
#ifdef _VERBOSE
    Log::Debug ("TcpAsyncConnector constructor invoked.");
#endif

    currRequestId_  = 0;
    destIpAddress_  = 0;
    destPort_       = 0;
    addressSize_    = sizeof(socketAddress_);
    memset (&socketAddress_, 0, sizeof(socketAddress_));
}



TcpAsyncConnector::~TcpAsyncConnector ()
{
#ifdef _VERBOSE
    Log::Debug ("TcpAsyncConnector destructor invoked.");
#endif

    t_PendingRequest *  request;

    for (const auto& [key, value] : requestIdIndex_) {
        request = reinterpret_cast<t_PendingRequest *>(value);
        delete request;
    }

    for (const auto& item : connectedList_) {
        request = reinterpret_cast<t_PendingRequest *>(item);
        delete request;
    }
}



bool 
TcpAsyncConnector::setDestination (ulong   ipAddress,
                                   ushort  port)
{
#ifdef _VERBOSE
    Log::Debug ("TcpAsyncConnector::setDestination invoked.");
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
TcpAsyncConnector::getDestination (ulong &   ipAddress,
                                   ushort &  port)
{
#ifdef _VERBOSE
    Log::Debug ("TcpAsyncConnector::getDestination invoked.");
#endif

    ipAddress = destIpAddress_;
    port      = destPort_;

    return true;
}



bool 
TcpAsyncConnector::requestConnection (ulong &  requestId)
{
#ifdef _VERBOSE
    Log::Debug ("TcpAsyncConnector::requestConnection invoked.");
#endif

    t_PendingRequest *  newRequest;
    newRequest = new t_PendingRequest;

    requestId = newRequest->requestId = currRequestId_++;
    newRequest->socketFd  = socket (AF_INET, SOCK_STREAM, 0);
    newRequest->destIp    = destIpAddress_;
    newRequest->destPort  = destPort_;

    if (newRequest->socketFd < 0) {
        delete newRequest;
        string errorCode;
        NetUtils::socketErrorAsString (alpine_socket_errno(), errorCode);
        Log::Error ("Create socket error: "s + errorCode +
                    " in TcpAsyncConnector::requestConnection!");

        return false;
    }
    bool  status;
    status = NetUtils::nonBlocking (newRequest->socketFd);

    if (!status) {
        Log::Error ("Error setting socket FD to non blocking in call to "
                             "TcpAsyncConnector::requestConnection!");
        alpine_close_socket(newRequest->socketFd);
        delete newRequest;

        return false;
    }
    int retVal;
    retVal = ::connect (newRequest->socketFd,
                        reinterpret_cast<struct sockaddr *>(&socketAddress_),
                        addressSize_);

    if (retVal >= 0) {
        // This is probably a local destination, so connect was immediate.  Place this
        // request on the connectedList_ and return with sucess.  No need to poll on this
        // socket.
        // 
        Log::Debug ("Socket connected immediately in call to "
                             "TcpAsyncConnector::requestConnection.  Adding to connectedList_.");

        connectedList_.push_back (newRequest);
        return true;
    }
    // Check for errno of anything but EINPROGRESS
    //
    if (alpine_socket_errno() != EINPROGRESS) {
        alpine_close_socket(newRequest->socketFd);
        delete newRequest;

        string errorCode;
        NetUtils::socketErrorAsString (alpine_socket_errno(), errorCode);
        Log::Error ("Connect socket error: "s + errorCode +
                    " in TcpAsyncConnector::requestConnection!");

        return false;
    }
    // Index this new request by request ID and FD.  Add to poll set for use
    // in user driven event loop.
    //
    status = pollSet_.add (newRequest->socketFd);

    if (!status) {
        alpine_close_socket(newRequest->socketFd);
        delete newRequest;

        Log::Error ("Error adding new socket FD to poll set in call to "
                             "TcpAsyncConnector::requestConnection!");

        return false;
    }
    requestIdIndex_.emplace (newRequest->requestId, newRequest);
    socketFdIndex_.emplace (newRequest->socketFd, newRequest);

    Log::Debug ("Socket added to pending indexes in call to "
                         "TcpAsyncConnector::requestConnection!");


    return true;
}



bool 
TcpAsyncConnector::requestConnection (ulong    destIpAddress,
                                      ushort   destPort,
                                      ulong &  requestId)
{
#ifdef _VERBOSE
    Log::Debug ("TcpAsyncConnector::requestConnection (with destination) invoked.");
#endif

    bool status;
    status = setDestination (destIpAddress, destPort);

    if (!status) {
        Log::Error ("Invalid destination address passed in call to "
                             "TcpAsyncConnector::requestConnection!");
        return false;
    }
    status = requestConnection (requestId);

    return status;
}



bool 
TcpAsyncConnector::pendingRequests ()
{
#ifdef _VERBOSE
    Log::Debug ("TcpAsyncConnector::pendingRequests invoked.");
#endif

    return !requestIdIndex_.empty () || !connectedList_.empty ();
}



bool 
TcpAsyncConnector::getFdList (PollSet::t_FileDescList &  fdList)
{
#ifdef _VERBOSE
    Log::Debug ("TcpAsyncConnector::getFdList invoked.");
#endif

    bool  status;
    status = pollSet_.getFdList (fdList);

    return status;
}



bool 
TcpAsyncConnector::cancelRequest (ulong  requestId)
{
#ifdef _VERBOSE
    Log::Debug ("TcpAsyncConnector::cancelRequest invoked.");
#endif

    t_PendingRequest *  request = nullptr;

    auto iter = requestIdIndex_.find (requestId);

    if (iter ==  requestIdIndex_.end ()) {
        Log::Error ("Invalid request ID passed in call to TcpAsyncConnector::cancelRequest!");
        return false;
    }
    request = reinterpret_cast<t_PendingRequest *>((*iter).second);

    // Remove this request from all indexes and poll sets
    //
    alpine_close_socket(request->socketFd);
    requestIdIndex_.erase (requestId);
    socketFdIndex_.erase (request->socketFd);

    removePollFd (request->socketFd);

    delete request;


    return true;
}



bool 
TcpAsyncConnector::processEvents (bool  doBlock)
{
#ifdef _VERBOSE
    Log::Debug ("TcpAsyncConnector::processEvents invoked.");
#endif

    bool     status;
    ulong    localIp;
    ushort   localPort;
    TcpTransport *  transport;


    // First things first, process any completed connect requests pending.
    //
    if (connectedList_.size()) {
        Log::Debug ("Processing connected requests.");
        return true;
    }

    t_PendingRequest *  request;
    for (const auto& listItem : connectedList_) {
        request = listItem;

        // Get endpoint information required to create transport
        //
        status = NetUtils::getLocalEndpoint (request->socketFd, localIp, localPort);

        if (!status) {
            Log::Error ("Unable to get local endpoint address in call to "
                                 "TcpAsyncConnector::processEvents! (connectedList)");
            alpine_close_socket(request->socketFd);
            delete request;

            continue;
        }

        transport = new TcpTransport (request->socketFd,
                                      request->destIp,
                                      request->destPort,
                                      localIp,
                                      localPort);

        // Invoke user derived handler to pass initialized transport up
        //
        receiveTransport (request->requestId, transport);

        // Cleanup
        //
        delete request;
    }

    connectedList_.clear ();


    // If we have no pending requests right now, simply return true.
    //
    if (requestIdIndex_.empty()) {
        Log::Debug ("No pending requests, returning.");
        return true;
    }
    // If user specified block mode, use infinite timeout, otherwise timeout immediately...
    //
    int timeout;
    if (doBlock) {
        timeout = -1;
    }
    else {
        timeout = 0;
    }

    // Update pollSet to poll on any read or write events.
    // (normally only write events will be received, however, failure will provide a read event)
    //
    pollSet_.setEvents (POLLIN | POLLOUT);


    // Perform poll.  If any active FDs, perform second non blocking poll on read events to
    // filter out failed requests.
    //
    Log::Debug ("Polling for activity on pending requests.");

    PollSet::t_FileDescList  badFdList;
    PollSet::t_FileDescList  goodFdList;
    status = pollSet_.poll (timeout, badFdList);

    if (!status) {
        Log::Error ("Poll failed on pollSet_ in call to "
                             "TcpAsyncConnector::processEvents!");
        return false;
    }
    if (badFdList.empty()) {
        return true;
    }
    // Get FD lists for failed and completed requests
    //
    pollSet_.setEvents (POLLIN);
    status = pollSet_.poll (0, badFdList);

    if (!status) {
        Log::Error ("Input/read poll failed on pollSet_ in call to "
                             "TcpAsyncConnector::processEvents!");
        return false;
    }
    pollSet_.setEvents (POLLOUT);
    status = pollSet_.poll (0, goodFdList);

    if (!status) {
        Log::Error ("Output/write poll failed on pollSet_ in call to "
                             "TcpAsyncConnector::processEvents!");
        return false;
    }
    // Process any failed requests
    //
    uint   i;
    int    currFd;
    ulong  currId;

    for (i = 0; i < badFdList.size (); i++) {
        currFd = badFdList[i];
        auto fdIter = socketFdIndex_.find (currFd);

        if (fdIter == socketFdIndex_.end ()) {
            Log::Error ("Active FD not found in socket FD index in call to "
                                 "TcpAsyncConnector::processEvents!");
            return false;
        }
        request = reinterpret_cast<t_PendingRequest *>((*fdIter).second);
        currId = request->requestId;
        requestIdIndex_.erase (currId);
        socketFdIndex_.erase (currFd);
        removePollFd (currFd);
        delete request;

        // Notify user of request failure
        //
        handleRequestFailure (currId);
    }
    

    // Process successful requests
    //
    ulong  destIp;
    ushort destPort;

    for (i = 0; i < goodFdList.size (); i++) {
        currFd = goodFdList[i];
        auto fdIter = socketFdIndex_.find (currFd);

        if (fdIter == socketFdIndex_.end ()) {
            // This was a failed request, removed when processing bad FDs
            continue;
        }

        request  = reinterpret_cast<t_PendingRequest *>((*fdIter).second);
        currId   = request->requestId;
        destIp   = request->destIp;
        destPort = request->destPort;

        requestIdIndex_.erase (currId);
        socketFdIndex_.erase (currFd);
        removePollFd (currFd);
        delete request;

        // Get endpoint information required to create transport
        //
        status = NetUtils::getLocalEndpoint (currFd, localIp, localPort);

        if (!status) {
            Log::Error ("Unable to get local endpoint address in call to "
                                 "TcpAsyncConnector::processEvents! (process goodFdList)");
            alpine_close_socket(currFd);
            continue;
        }

        transport = new TcpTransport (currFd,
                                      destIp,
                                      destPort,
                                      localIp,
                                      localPort);

        // Invoke user derived handler to pass initialized transport up
        //
        receiveTransport (currId, transport);
    }    


    return true;
}



bool  
TcpAsyncConnector::removePollFd (int  fd)
{
#ifdef _VERBOSE
    Log::Debug ("TcpAsyncAcceptor::removePollFd invoked.");
#endif

    bool status;
    PollSet::t_FileDescList  currFds;
    PollSet::t_FileDescList  newFds;

    status = pollSet_.getFdList (currFds);

    if (!status) {
        Log::Error ("Error retrieving current poll FD list from pollSet_ in call to "
                             "TcpAsyncConnector::removePollFd!");
        return false;
    }
    newFds.resize (currFds.size () -1);

    uint i;
    uint currInd = 0;
    for (i = 0; i < currFds.size (); i++) {
        if (currFds[i] != fd) {
            newFds[currInd++] = currFds[i];
        }
    }

    pollSet_.clear ();
    pollSet_.add (newFds);
    

    return true;
}



