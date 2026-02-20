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
#include <AlpineCorbaClient.h>
#include <CorbaUtils.h>
#include <string>
#include <vector>


class AlpinePeerMgmtIntf
{
  public:


    // Public types
    //
    struct t_AlpinePeerInfo {
        ulong    id;
        string   alias;
        string   description;
        ulong    dateConnected;
        ulong    qualityPosition;
        ulong    relativeQualityRating;
        ulong    totalShared;
    };

    using t_AlpinePeerInfoList = vector<t_AlpinePeerInfo>;

    using t_PeerIdList = vector<ulong>;


    // Supported interface operations
    //
    static bool  getExtendedPeerList (t_PeerIdList & peerList);

    static bool  getPeerInformation (t_AlpinePeerInfo & peerInfo);

    static bool  updatePeerInformation (const t_AlpinePeerInfo & peerInfo);


  private:

};

