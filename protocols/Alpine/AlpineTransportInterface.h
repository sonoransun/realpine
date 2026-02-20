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


////
//
// The TransportInterface is used by the Client and Server Transport Modules
// for individual data transfers which are requested or served.
//
// Persistant or non terminating transfers, such as chat or streaming will have
// no totalDataSize (), but must track all other statistics.
//
////


class AlpineTransportInterface
{
  public:

    AlpineTransportInterface () {};
    virtual ~AlpineTransportInterface () {};


    // Each transport must have a unqiue 4 byte unsigned identifier
    //
    virtual ulong  getId () = 0;


    // Duplication of existing transports (handles)
    //
    virtual AlpineTransportInterface *  duplicate () = 0;


    // Status
    //
    virtual float  averageBandwidth () = 0;

    virtual float  peakBandwidth () = 0;

    virtual ulong  totalDataSize () = 0;

    virtual ulong  xferDataSize () = 0;


    // Control
    //
    virtual bool  isActive () = 0;

    virtual bool  cancel () = 0;

    virtual bool  pause () = 0;

    virtual bool  resume () = 0;

};

