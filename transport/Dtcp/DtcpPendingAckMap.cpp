/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <DtcpPendingAckMap.h>
#include <DtcpBaseUdpTransport.h>
#include <DtcpBaseConnTransport.h>
#include <DataBlock.h>
#include <Configuration.h>
#include <Log.h>
#include <StringUtils.h>
#include <Platform.h>
#include <cmath>



DtcpPendingAckMap::DtcpPendingAckMap (DtcpBaseUdpTransport * udpTransport)
    : rng_ (std::random_device{}())
{
#ifdef _VERBOSE
    Log::Debug ("DtcpPendingAckMap constructor invoked.");
#endif

    udpTransport_ = udpTransport;

    recordIndex_ = new t_RecordIndex;
    currId_ = 1;

    // Read configurable max retries
    //
    maxRetries_ = kDefaultMaxRetries;

    string maxRetriesStr;
    if (Configuration::getValue ("dtcp.maxRetries"s, maxRetriesStr) &&
        !maxRetriesStr.empty ()) {
        try {
            auto val = std::stoi (maxRetriesStr);
            if (val > 0 && val <= 255) {
                maxRetries_ = static_cast<ushort>(val);
            }
        }
        catch (...) {
            Log::Error ("Invalid dtcp.maxRetries value, using default.");
        }
    }
}



DtcpPendingAckMap::~DtcpPendingAckMap ()
{
#ifdef _VERBOSE
    Log::Debug ("DtcpPendingAckMap destructor invoked.");
#endif

    // Clean up all remaining pending records
    //
    for (auto & [id, record] : *recordIndex_) {
        delete record->data;
        delete record;
    }

    delete recordIndex_;
}



bool
DtcpPendingAckMap::add (DtcpBaseConnTransport * requestor,
                        DataBlock *             data,
                        ulong                   destIpAddress,
                        ushort                  destPort,
                        ulong &                 id)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpPendingAckMap::add invoked.");
#endif


    // assign this request an ID, not a problem if it
    // fails, as we have plenty to choose from.
    //
    id = currId_++;

    auto now = t_Clock::now ();


    // Compute initial RTO based on peer RTT estimate
    //
    auto rto = computeRto (destIpAddress);
    auto nextRetry = now + applyJitter (rto);


    // Create new record for this reliable transfer
    //
    auto * newRecord = new t_PendingRecord;

    newRecord->id               = id;
    newRecord->requestor        = requestor;
    newRecord->data             = data;
    newRecord->destIpAddress    = destIpAddress;
    newRecord->destPort         = destPort;
    newRecord->sendTime         = now;
    newRecord->nextRetryTime    = nextRetry;
    newRecord->retryCount       = 0;
    newRecord->currentRto       = rto;


    // Place in priority queue and index by ID.
    //
    recordIndex_->emplace (id, newRecord);
    retryQueue_.push ({id, nextRetry});


    return true;
}



bool
DtcpPendingAckMap::remove (ulong  id)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpPendingAckMap::remove invoked.");
#endif

    // Locate record for this ID
    //
    auto iter = recordIndex_->find (id);

    if (iter == recordIndex_->end ()) {
        // not found?
        //
#ifdef _VERBOSE
        Log::Debug ("Error locating record ID: "s + std::to_string (id) +
                    " in DtcpPendingAckMap::remove.");
#endif

        return false;
    }
    auto * record = iter->second;


    // Compute RTT sample from first send time (only valid if not retransmitted,
    // to avoid retransmission ambiguity per Karn's algorithm)
    //
    if (record->retryCount == 0) {
        auto now = t_Clock::now ();
        auto sampleMs = std::chrono::duration<double, std::milli>(now - record->sendTime).count ();
        updatePeerRtt (record->destIpAddress, sampleMs);
    }


    // Free data block and erase from index
    //
    delete record->data;
    delete record;

    recordIndex_->erase (iter);

    // Note: stale entries in retryQueue_ are handled lazily in processTimers()
    // by checking if the ID still exists in the recordIndex_.


    return true;
}



bool
DtcpPendingAckMap::processTimers ()
{
#ifdef _VERY_VERBOSE
    Log::Debug ("DtcpPendingAckMap::processTimers invoked.");
#endif

    if (retryQueue_.empty ()) {
        return true;
    }

    auto now = t_Clock::now ();

    while (!retryQueue_.empty ()) {

        const auto & top = retryQueue_.top ();

        // Stop when we reach entries that haven't expired yet
        //
        if (top.nextRetryTime > now) {
            break;
        }

        auto entry = retryQueue_.top ();
        retryQueue_.pop ();


        // Check if the record still exists (may have been removed by ack)
        //
        auto iter = recordIndex_->find (entry.id);
        if (iter == recordIndex_->end ()) {
            // Stale entry — record was already acked and removed
            continue;
        }

        auto * record = iter->second;


        // Check if this is a stale queue entry (record was re-enqueued with a newer time)
        //
        if (record->nextRetryTime != entry.nextRetryTime) {
            continue;
        }


        if (record->retryCount >= maxRetries_) {

            // Max retries exceeded — notify requestor of failure
            //
#ifdef _VERBOSE
            Log::Debug ("Max retries ("s + std::to_string (maxRetries_) +
                        ") exceeded for pending packet in DtcpPendingAckMap.");
#endif

            recordIndex_->erase (iter);

            // Notify requestor of send failure (this chains to
            // AlpinePeerMgr::reliableTransferFailed which feeds
            // AlpineRatingEngine::transferFailureEvent)
            //
            if (record->requestor) {
                record->requestor->handleSendFailure (record->id);
            }
            else {
                udpTransport_->handleSendFailure (record->id);
            }

            delete record->data;
            delete record;
        }
        else {
            // Resend packet with exponential backoff
            //
#ifdef _VERBOSE
            Log::Debug ("Retry "s + std::to_string (record->retryCount + 1) +
                        " for pending packet in DtcpPendingAckMap.  Resending...");
#endif

            udpTransport_->sendData (record->destIpAddress,
                                     record->destPort,
                                     record->data->buffer_.get(),
                                     record->data->length_);

            record->retryCount++;

            // Exponential backoff: double the RTO
            //
            record->currentRto = std::min (
                record->currentRto * 2,
                std::chrono::duration_cast<t_Duration>(
                    std::chrono::duration<double, std::milli>(kMaxRtoMs)));

            auto nextRetry = now + applyJitter (record->currentRto);
            record->sendTime      = now;
            record->nextRetryTime = nextRetry;

            retryQueue_.push ({record->id, nextRetry});
        }
    }

#ifdef _VERY_VERBOSE
    Log::Debug ("processTimers complete...");
#endif


    return true;
}



DtcpPendingAckMap::t_Duration
DtcpPendingAckMap::computeRto (ulong  destIpAddress) const
{
    auto iter = peerRttIndex_.find (destIpAddress);

    double rtoMs;

    if (iter != peerRttIndex_.end () && iter->second.initialized) {
        // RTO = SRTT + 4 * RTTVAR
        //
        rtoMs = iter->second.srtt + 4.0 * iter->second.rttvar;

        // Clamp to reasonable range
        //
        rtoMs = std::max (rtoMs, 10.0);
        rtoMs = std::min (rtoMs, kMaxRtoMs);
    }
    else {
        // No RTT data for this peer yet
        //
        rtoMs = kDefaultRtoMs;
    }

    return std::chrono::duration_cast<t_Duration>(
        std::chrono::duration<double, std::milli>(rtoMs));
}



DtcpPendingAckMap::t_Duration
DtcpPendingAckMap::applyJitter (t_Duration  base)
{
    auto baseMs = std::chrono::duration<double, std::milli>(base).count ();

    std::uniform_real_distribution<double> dist (
        baseMs * (1.0 - kJitterFraction),
        baseMs * (1.0 + kJitterFraction));

    auto jitteredMs = dist (rng_);

    return std::chrono::duration_cast<t_Duration>(
        std::chrono::duration<double, std::milli>(jitteredMs));
}



void
DtcpPendingAckMap::updatePeerRtt (ulong  destIpAddress,
                                   double sampleMs)
{
#ifdef _VERBOSE
    Log::Debug ("DtcpPendingAckMap::updatePeerRtt — sample: "s +
                std::to_string (sampleMs) + " ms for peer "s +
                std::to_string (destIpAddress));
#endif

    auto & rtt = peerRttIndex_[destIpAddress];

    if (!rtt.initialized) {
        // First sample: initialize directly per RFC 6298
        //
        rtt.srtt   = sampleMs;
        rtt.rttvar = sampleMs / 2.0;
        rtt.initialized = true;
    }
    else {
        // EWMA update per RFC 6298:
        //   RTTVAR = (1 - beta) * RTTVAR + beta * |SRTT - sample|
        //   SRTT   = (1 - alpha) * SRTT + alpha * sample
        //
        rtt.rttvar = (1.0 - kSrttBeta) * rtt.rttvar +
                     kSrttBeta * std::abs (rtt.srtt - sampleMs);
        rtt.srtt   = (1.0 - kSrttAlpha) * rtt.srtt +
                     kSrttAlpha * sampleMs;
    }
}

