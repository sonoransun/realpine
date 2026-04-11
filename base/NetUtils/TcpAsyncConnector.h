/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <NetUtils.h>
#include <OptHash.h>
#include <Platform.h>
#include <PollSet.h>
#include <list>


class TcpTransport;


class TcpAsyncConnector
{
  public:
    TcpAsyncConnector();
    virtual ~TcpAsyncConnector();


    bool setDestination(ulong ipAddress, ushort port);

    bool getDestination(ulong & ipAddress, ushort & port);

    bool requestConnection(ulong & requestId);

    bool requestConnection(ulong destIpAddress, ushort destPort, ulong & requestId);

    bool pendingRequests();

    bool getFdList(PollSet::t_FileDescList & fdList);

    bool cancelRequest(ulong requestId);


    // This MUST be called upon FD activity to accept connections
    // If doBlock, this acts as a run loop and returns only when all requests are satisfied.
    //
    bool processEvents(bool doBlock = false);


    // User must implement these methods to handle connection sucess for failure events.
    //
    virtual void receiveTransport(ulong requestId, TcpTransport * transport) = 0;

    virtual void handleRequestFailure(ulong requestId) = 0;


    // Internal types
    //
    struct t_PendingRequest
    {
        ulong requestId;
        int socketFd;
        ulong destIp;
        ushort destPort;
    };

    using t_RequestIndex = std::unordered_map<ulong,
                                              void *,  // t_PendingRequest *
                                              OptHash<ulong>,
                                              equal_to<ulong>>;

    using t_RequestIndexPair = std::pair<ulong, void *>;


    using t_FileDescIndex = std::unordered_map<int,
                                               void *,  // t_PendingRequest *
                                               OptHash<ulong>,
                                               equal_to<ulong>>;

    using t_FileDescIndexPair = std::pair<int, void *>;


    using t_RequestList = list<t_PendingRequest *>;


  private:
    ulong currRequestId_;
    ulong destIpAddress_;
    ushort destPort_;
    struct sockaddr_in socketAddress_;
    int addressSize_;
    t_RequestIndex requestIdIndex_;
    t_FileDescIndex socketFdIndex_;
    t_RequestList connectedList_;
    PollSet pollSet_;


    bool removePollFd(int fd);
};
