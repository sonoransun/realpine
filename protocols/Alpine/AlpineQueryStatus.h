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


class AlpineQueryStatus
{
  public:

    AlpineQueryStatus () = default;

    AlpineQueryStatus (const AlpineQueryStatus & copy);

    ~AlpineQueryStatus () = default;

    const AlpineQueryStatus & operator = (const AlpineQueryStatus & copy);



    ulong  totalPackets ();

    ulong  numPacketsSent ();

    ulong  numRepliesReceived ();

    double percentComplete ();

    bool   isActive ();



  private:

    ulong        totalPackets_;
    ulong        packetsSent_;
    ulong        repliesReceived_;
    double       percentComplete_;
    bool         isActive_;

  
    void  setTotalPackets (ulong  totalPackets);
 
    void  setPacketsSent (ulong  numSent);

    void  setRepliesReceived (ulong  numReceived);

    void  setPercentComplete (const double &  percentage);

    void  setIsActive (bool  value);


    friend class AlpineQueryMgr;
    friend class AlpineQuery;
};


