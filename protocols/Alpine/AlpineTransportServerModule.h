/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <AppCallback.h>
#include <Common.h>
#include <vector>


class AlpineTransportInterface;


class AlpineTransportServerModule
{
  public:
    AlpineTransportServerModule(){};
    virtual ~AlpineTransportServerModule(){};


    // Control
    //
    virtual bool start() = 0;

    virtual bool isActive() = 0;

    virtual bool stop() = 0;


    // Server operations
    //
    using t_TransportIdList = vector<ulong>;

    virtual bool getTransportIdList(t_TransportIdList & idList) = 0;


    // Transport must be an allocated duplicate!
    //
    virtual bool getTransport(ulong transportId, AlpineTransportInterface *& transport) = 0;


    // Event dispatch registry
    //
    // These methods must be invoked when a transport is destroyed at completion
    // or due to error.
    //
    using t_Callback = AppCallbackBase<ulong>;

    virtual bool registerCompletionHandler(t_Callback & handler) = 0;

    virtual bool registerFailureHandler(t_Callback & handler) = 0;
};
