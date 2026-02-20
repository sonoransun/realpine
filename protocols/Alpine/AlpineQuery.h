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
#include <AlpineQueryOptions.h>
#include <AlpineQueryStatus.h>
#include <ReadWriteSem.h>


class AlpinePacket;
class AlpineQueryPacket;
class AlpineBroadcast;
class AlpineGroup;


class AlpineQuery
{
  public:

    ~AlpineQuery ();


    using t_PeerIdList = vector<ulong>;


    bool  startQuery ();

    bool  inProgress ();

    bool  getStatus (AlpineQueryStatus &  queryStatus);

    bool  getPeerIdList (t_PeerIdList &  peerIdList);

    bool  halt ();

    bool  resume ();

    bool  cancel ();


  private:

    AlpineQueryOptions   options_;
    bool                 queryActive_;
    AlpineGroup *        group_;
    AlpineBroadcast *    broadcast_;
    AlpinePacket *       alpinePacket_;
    AlpineQueryPacket *  queryPacket_;
    ReadWriteSem         dataLock_;
    

    // Only the AlpineQueryMgr creates queries
    //
    AlpineQuery (AlpineQueryOptions & options);


    bool  packetSent (ulong  transportId);

    bool  broadcastComplete (ulong             numSent,
                             struct timeval &  duration);


    // Copy consturctor and assignement operator not implemented
    //
    AlpineQuery (const AlpineQuery & copy);
    AlpineQuery & operator = (const AlpineQuery & copy);


    friend class AlpineBroadcast;
    friend class AlpineQueryMgr;
};


