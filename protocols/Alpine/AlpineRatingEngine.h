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


class AlpinePeerProfile;


class AlpineRatingEngine
{
  public:

    AlpineRatingEngine () = default;
    ~AlpineRatingEngine () = default;



    // Public types
    //
    enum class t_ResourceRating { Low, Average, High };


    // Peer responded with matches for a query
    //
    static bool  queryResponseEvent (AlpinePeerProfile *  profile,
                                     short &              qualityDelta);


    // Peer sent unknown or invalid packet
    //
    static bool  badPacketEvent (AlpinePeerProfile *  profile,
                                 short &              qualityDelta);


    // A reliable transfer was attempted and failed.
    //
    static bool  transferFailureEvent (AlpinePeerProfile *  profile,
                                       short &              qualityDelta);


    // Peer resource from query is not available
    // (download failed, unreachable, etc)
    //
    static bool  naResourceEvent (AlpinePeerProfile *  profile,
                                  short &              qualityDelta);


    // Peer resource from query misrepresented
    // (spam, deception, malicious intent)
    //
    static bool  deceptiveResourceEvent (AlpinePeerProfile *  profile,
                                         short &              qualityDelta);


    // Peer resource rating by client
    //
    static bool  clientResourceEvaluation (AlpinePeerProfile *  profile,
                                           t_ResourceRating     rating,
                                           short &              qualityDelta);


  private:

};


