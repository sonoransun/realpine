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
#include <list>
#include <vector>
#include <OptHash.h>


class DtcpBaseConnTransport;


class DtcpBroadcastSet
{
  public:


    // Public types
    // 
    using t_TransportList = list<DtcpBaseConnTransport *>;

    using t_TransportIdList = vector<ulong>;



    // Constructors
    //
    DtcpBroadcastSet ();

    DtcpBroadcastSet (t_TransportList &  transportList);

    DtcpBroadcastSet (t_TransportIdList &  transportIdList);

    ~DtcpBroadcastSet ();



    // Public operations
    //
    ulong  size ();

    // Insert always places new transport(s) at end of set.
    //
    bool  insert (DtcpBaseConnTransport *  transport);

    bool  insert (ulong  transportId);

    bool  exists (DtcpBaseConnTransport *  transport);

    bool  exists (ulong  transportId);

    bool  getTransportList (t_TransportList &  transportList);

    bool  getTransportIdList (t_TransportIdList &  transportIdList);

    // Remove is NOT EFFICIENT.  Use with care
    //
    bool  remove (DtcpBaseConnTransport *  transport);

    bool  remove (ulong  transportId);

    bool  clear ();



    // Internal types
    //
    using t_TransportIndex = std::unordered_map<ulong, // transport ID
                      ulong, // transport index
                      OptHash<ulong>,
                      equal_to<ulong> >;

    using t_TransportIndexPair = std::pair<ulong, ulong>;

    using t_TransportArray = vector<DtcpBaseConnTransport *>;
                 

  private:

    t_TransportArray *  transportArray_;
    t_TransportIndex *  transportIndex_;
    uint                reserve_;


    bool  extendTransportArray (uint  extent);

};

