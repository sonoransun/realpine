/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <AlpineDtcpConnConnector.h>
#include <AlpineDtcpConnTransport.h>
#include <AlpineStack.h>
#include <Log.h>
#include <MuxInterface.h>
#include <ReadLock.h>
#include <StringUtils.h>
#include <WriteLock.h>


AlpineDtcpConnConnector::t_BackoffIndex AlpineDtcpConnConnector::backoffIndex_s;
ReadWriteSem AlpineDtcpConnConnector::backoffLock_s;
std::mt19937 AlpineDtcpConnConnector::rng_s{std::random_device{}()};


AlpineDtcpConnConnector::AlpineDtcpConnConnector()
{
#ifdef _VERBOSE
    Log::Debug("AlpineDtcpConnConnector constructor invoked.");
#endif
}


AlpineDtcpConnConnector::~AlpineDtcpConnConnector()
{
#ifdef _VERBOSE
    Log::Debug("AlpineDtcpConnConnector destructor invoked.");
#endif
}


bool
AlpineDtcpConnConnector::createTransport(DtcpBaseConnTransport *& transport)
{
#ifdef _VERBOSE
    Log::Debug("AlpineDtcpConnConnector::createTransport invoked.");
#endif


    AlpineDtcpConnTransport * alpineTransport;
    alpineTransport = new AlpineDtcpConnTransport();

    transport = static_cast<DtcpBaseConnTransport *>(alpineTransport);


    return true;
}


bool
AlpineDtcpConnConnector::receiveTransport(DtcpBaseConnTransport * transport)
{
#ifdef _VERBOSE
    Log::Debug("AlpineDtcpConnConnector::receiveTransport invoked.");
#endif

    // Cast back to AlpineDtcpConnTransport type.
    AlpineDtcpConnTransport * alpineTransport;
    alpineTransport = dynamic_cast<AlpineDtcpConnTransport *>(transport);

    if (!alpineTransport) {
        // Invalid transport type?
        Log::Error("Invalid transport type passed to AlpineDtcpConnConnector::receiveTransport.");

        return false;
    }

    bool status;
    ulong peerId;

    transport->getTransportId(peerId);

    status = AlpineStack::registerTransport(peerId, alpineTransport);

    if (!status) {
        Log::Error("Register transport failed in AlpineDtcpConnConnector::receiveTransport!");
        return false;
    }

    // Connection succeeded — reset backoff for this peer
    resetBackoff(peerId);


    return true;
}


bool
AlpineDtcpConnConnector::handleRequestFailure(ulong ipAddress, ushort port)
{
#ifdef _VERBOSE
    Log::Debug("AlpineDtcpConnConnector::handleRequestFailure invoked.");
#endif

    // Record backoff for this peer address
    //
    WriteLock lock(backoffLock_s);

    auto & state = backoffIndex_s[ipAddress];

    double jitteredDelay = applyJitter(state.currentDelaySecs);
    state.nextAllowedAt = Clock::now() + std::chrono::milliseconds(static_cast<long>(jitteredDelay * 1000.0));
    state.active = true;

    Log::Info("AlpineDtcpConnConnector: backoff "s + std::to_string(jitteredDelay) + "s for peer "s +
              std::to_string(ipAddress));

    // Increase backoff for next failure
    state.currentDelaySecs = std::min(state.currentDelaySecs * BACKOFF_MULTIPLIER, MAX_BACKOFF_SECS);

    return true;
}


double
AlpineDtcpConnConnector::applyJitter(double delaySecs)
{
    std::uniform_real_distribution<double> dist(1.0 - JITTER_FACTOR, 1.0 + JITTER_FACTOR);
    return delaySecs * dist(rng_s);
}


bool
AlpineDtcpConnConnector::allowReconnect(ulong peerId)
{
    WriteLock lock(backoffLock_s);

    auto iter = backoffIndex_s.find(peerId);
    if (iter == backoffIndex_s.end()) {
        return true;  // no backoff state — allow
    }

    auto & state = iter->second;
    if (!state.active) {
        return true;
    }

    return Clock::now() >= state.nextAllowedAt;
}


void
AlpineDtcpConnConnector::resetBackoff(ulong peerId)
{
    WriteLock lock(backoffLock_s);

    auto iter = backoffIndex_s.find(peerId);
    if (iter == backoffIndex_s.end()) {
        return;
    }

    iter->second.currentDelaySecs = INITIAL_BACKOFF_SECS;
    iter->second.active = false;
}
