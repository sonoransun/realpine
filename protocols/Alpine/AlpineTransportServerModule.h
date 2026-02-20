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
#include <AppCallback.h>
#include <vector>


class AlpineTransportInterface;


class AlpineTransportServerModule
{
  public:

    AlpineTransportServerModule () {};
    virtual ~AlpineTransportServerModule () {};


    // Control
    //
    virtual bool  start () = 0;

    virtual bool  isActive () = 0;

    virtual bool  stop () = 0;



    // Server operations
    //
    using t_TransportIdList = vector<ulong>;

    virtual bool  getTransportIdList (t_TransportIdList &  idList) = 0;



    // Transport must be an allocated duplicate!
    //
    virtual bool  getTransport (ulong                        transportId,
                                AlpineTransportInterface *&  transport) = 0;



    // Event dispatch registry
    //
    // These methods must be invoked when a transport is destroyed at completion
    // or due to error.
    //
    using t_Callback = AppCallbackBase<ulong>;

    virtual bool  registerCompletionHandler (t_Callback &  handler) = 0;

    virtual bool  registerFailureHandler (t_Callback &  handler) = 0;


};

