/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <OptHash.h>
#include <ReadWriteSem.h>
#include <chrono>
#include <unordered_map>


class CircuitBreaker
{
  public:
    CircuitBreaker() = default;
    ~CircuitBreaker() = default;


    enum class State { Closed, Open, HalfOpen };


    /// Check whether a request to the given peer should be allowed.
    /// Returns true if the circuit is Closed or HalfOpen (probe allowed).
    ///
    static bool allowRequest(ulong peerId);

    /// Record a successful response from a peer.
    /// Transitions Open/HalfOpen → Closed and resets failure count.
    ///
    static void recordSuccess(ulong peerId);

    /// Record a failure for a peer.
    /// After FAILURE_THRESHOLD consecutive failures, transitions to Open.
    ///
    static void recordFailure(ulong peerId);

    /// Return the current state for a peer (Closed if unknown).
    ///
    static State getState(ulong peerId);

    /// Configure thresholds (primarily for testing).
    ///
    static void configure(int failureThreshold, int openDurationSecs);

    /// Reset all circuit state (primarily for testing).
    ///
    static void reset();


  private:
    static int failureThreshold_s;
    static int halfOpenMaxProbes_s;
    static int openDurationSecs_s;

    using Clock = std::chrono::steady_clock;
    using TimePoint = Clock::time_point;

    struct PeerCircuit
    {
        State state = State::Closed;
        int failureCount = 0;
        int halfOpenProbes = 0;
        TimePoint openedAt{};
    };

    using t_CircuitIndex = std::unordered_map<ulong, PeerCircuit, OptHash<ulong>, std::equal_to<ulong>>;

    static t_CircuitIndex circuitIndex_s;
    static ReadWriteSem dataLock_s;

    /// Evaluate time-based state transitions (Open → HalfOpen).
    ///
    static void evaluateState(PeerCircuit & circuit);
};
