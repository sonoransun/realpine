/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <CircuitBreaker.h>
#include <Log.h>
#include <ReadLock.h>
#include <StringUtils.h>
#include <WriteLock.h>


CircuitBreaker::t_CircuitIndex CircuitBreaker::circuitIndex_s;
ReadWriteSem CircuitBreaker::dataLock_s;
int CircuitBreaker::failureThreshold_s = 5;
int CircuitBreaker::halfOpenMaxProbes_s = 2;
int CircuitBreaker::openDurationSecs_s = 30;


void
CircuitBreaker::evaluateState(PeerCircuit & circuit)
{
    if (circuit.state != State::Open) {
        return;
    }

    auto elapsed = Clock::now() - circuit.openedAt;
    if (elapsed >= std::chrono::seconds(openDurationSecs_s)) {
        circuit.state = State::HalfOpen;
        circuit.halfOpenProbes = 0;
    }
}


bool
CircuitBreaker::allowRequest(ulong peerId)
{
    WriteLock lock(dataLock_s);

    auto iter = circuitIndex_s.find(peerId);
    if (iter == circuitIndex_s.end()) {
        return true;  // no record — Closed by default
    }

    auto & circuit = iter->second;
    evaluateState(circuit);

    if (circuit.state == State::Closed) {
        return true;
    }

    if (circuit.state == State::HalfOpen) {
        if (circuit.halfOpenProbes < halfOpenMaxProbes_s) {
            ++circuit.halfOpenProbes;
            return true;
        }
        return false;
    }

    // State::Open
    return false;
}


void
CircuitBreaker::recordSuccess(ulong peerId)
{
    WriteLock lock(dataLock_s);

    auto iter = circuitIndex_s.find(peerId);
    if (iter == circuitIndex_s.end()) {
        return;  // no record — already Closed
    }

    auto & circuit = iter->second;

    if (circuit.state == State::HalfOpen || circuit.state == State::Open) {
        Log::Info("CircuitBreaker: peer "s + std::to_string(peerId) + " circuit closed after successful probe"s);
    }

    circuit.state = State::Closed;
    circuit.failureCount = 0;
    circuit.halfOpenProbes = 0;
}


void
CircuitBreaker::recordFailure(ulong peerId)
{
    WriteLock lock(dataLock_s);

    auto & circuit = circuitIndex_s[peerId];
    evaluateState(circuit);

    if (circuit.state == State::HalfOpen) {
        // Probe failed — reopen the circuit
        circuit.state = State::Open;
        circuit.openedAt = Clock::now();
        Log::Info("CircuitBreaker: peer "s + std::to_string(peerId) + " circuit re-opened after failed probe"s);
        return;
    }

    ++circuit.failureCount;

    if (circuit.failureCount >= failureThreshold_s && circuit.state == State::Closed) {
        circuit.state = State::Open;
        circuit.openedAt = Clock::now();
        Log::Info("CircuitBreaker: peer "s + std::to_string(peerId) + " circuit opened after "s +
                  std::to_string(failureThreshold_s) + " consecutive failures"s);
    }
}


CircuitBreaker::State
CircuitBreaker::getState(ulong peerId)
{
    WriteLock lock(dataLock_s);

    auto iter = circuitIndex_s.find(peerId);
    if (iter == circuitIndex_s.end()) {
        return State::Closed;
    }

    evaluateState(iter->second);
    return iter->second.state;
}


void
CircuitBreaker::configure(int failureThreshold, int openDurationSecs)
{
    WriteLock lock(dataLock_s);
    failureThreshold_s = failureThreshold;
    openDurationSecs_s = openDurationSecs;
}


void
CircuitBreaker::reset()
{
    WriteLock lock(dataLock_s);
    circuitIndex_s.clear();
    failureThreshold_s = 5;
    halfOpenMaxProbes_s = 2;
    openDurationSecs_s = 30;
}
