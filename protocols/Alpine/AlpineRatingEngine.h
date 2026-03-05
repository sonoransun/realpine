/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <ReadWriteSem.h>
#include <chrono>
#include <unordered_map>


class AlpinePeerProfile;


class AlpineRatingEngine
{
  public:

    AlpineRatingEngine () = default;
    ~AlpineRatingEngine () = default;



    // Public types
    //
    enum class t_ResourceRating { Low, Average, High };

    struct t_PeerRating
    {
        double                                    score        = 0.5;
        ulong                                     successCount = 0;
        ulong                                     failureCount = 0;
        std::chrono::steady_clock::time_point     lastUpdate   = std::chrono::steady_clock::now();
    };


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


    // Score lookup for external consumers (e.g. weighted peer selection)
    //
    static double  getScore (ulong peerId);


    // Distributed score gossip
    //
    struct t_ScoreEntry {
        ulong   peerId;
        double  score;
        ulong   interactions;  // successCount + failureCount
    };

    static vector<t_ScoreEntry>  getTopScores (size_t maxEntries = 50);

    // Merge remote scores using weighted average.
    // Only applies if remote has >= minInteractions.
    static void  mergeRemoteScores (const vector<t_ScoreEntry> & remoteScores,
                                    ulong minInteractions = 10);


    // Persistence
    //
    static bool  persist ();
    static bool  load ();


  private:

    static constexpr double  kInitialScore   = 0.5;
    static constexpr double  kAlpha          = 0.1;
    static constexpr double  kMinScore       = 0.0;
    static constexpr double  kMaxScore       = 1.0;
    static constexpr double  kDecayTarget    = 0.5;
    static constexpr double  kDecayRatePerHr = 0.01;

    static std::unordered_map<ulong, t_PeerRating>  ratings_s;
    static ReadWriteSem                              lock_s;


    static ulong   getPeerId (AlpinePeerProfile * profile);

    static double  applyDecay (t_PeerRating & rating);

    static double  applyDelta (t_PeerRating & rating, double delta, bool isSuccess);

    static double  clampScore (double score);

};


