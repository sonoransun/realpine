/// Copyright (C) 2026 sonoransun — see LICENCE.txt


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


