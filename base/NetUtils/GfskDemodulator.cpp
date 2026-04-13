/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <GfskDemodulator.h>
#include <Log.h>

#include <cmath>
#include <cstring>


GfskDemodulator::GfskDemodulator(uint baudRate)
    : baudRate_(baudRate),
      sampleRate_(0),
      samplesPerSymbol_(0.0f),
      initialized_(false),
      prevI_(0.0f),
      prevQ_(0.0f),
      muPhase_(0.0f),
      prevSample_(0.0f),
      prevSymbol_(0.0f),
      state_(State::HUNTING_PREAMBLE),
      preambleCount_(0),
      bitsCollected_(0),
      frameLength_(0)
{}


bool
GfskDemodulator::initialize(uint sampleRate)
{
    if (sampleRate == 0 || baudRate_ == 0) {
        Log::Error("GfskDemodulator: invalid sample rate or baud rate.");
        return false;
    }

    sampleRate_ = sampleRate;
    samplesPerSymbol_ = static_cast<float>(sampleRate_) / static_cast<float>(baudRate_);

    if (samplesPerSymbol_ < 2.0f) {
        Log::Error("GfskDemodulator: sample rate too low for baud rate (need >= 2 samples/symbol).");
        return false;
    }

    prevI_ = 0.0f;
    prevQ_ = 0.0f;
    muPhase_ = 0.0f;
    prevSample_ = 0.0f;
    prevSymbol_ = 0.0f;
    bitBuffer_.clear();
    resetFrameState();

    initialized_ = true;

    Log::Info("GfskDemodulator: initialized (baud="s + std::to_string(baudRate_) + ", sampleRate=" +
              std::to_string(sampleRate_) + ", samplesPerSymbol=" + std::to_string(samplesPerSymbol_) + ")");

    return true;
}


bool
GfskDemodulator::demodulate(const int8_t * iqSamples, uint numSamples, vector<vector<byte>> & packets)
{
    if (!initialized_) {
        return false;
    }

    // IQ samples are interleaved: I0, Q0, I1, Q1, ...
    // numSamples is the total count of int8 values (so numSamples/2 IQ pairs)
    uint numPairs = numSamples / 2;

    for (uint i = 0; i < numPairs; ++i) {
        float iSample = static_cast<float>(iqSamples[i * 2]) / 128.0f;
        float qSample = static_cast<float>(iqSamples[i * 2 + 1]) / 128.0f;

        // FM discriminator: instantaneous frequency from phase difference
        float freq = fmDiscriminate(iSample, qSample);

        // Mueller-Muller clock recovery
        muPhase_ += 1.0f / samplesPerSymbol_;

        if (muPhase_ >= 1.0f) {
            muPhase_ -= 1.0f;

            // Symbol decision: positive frequency -> 1, negative -> 0
            byte bit = (freq > 0.0f) ? 1 : 0;

            // Timing error detector (Mueller-Muller)
            float timingError = (prevSample_ - freq) * prevSymbol_ - (prevSymbol_ - (bit ? 1.0f : -1.0f)) * prevSample_;

            // Adjust phase with bounded correction
            static constexpr float MU_GAIN = 0.01f;
            float correction = timingError * MU_GAIN;

            if (correction > 0.1f) {
                correction = 0.1f;
            }

            if (correction < -0.1f) {
                correction = -0.1f;
            }

            muPhase_ += correction;

            prevSample_ = freq;
            prevSymbol_ = bit ? 1.0f : -1.0f;

            processSymbol(bit, packets);
        }
    }

    return true;
}


string
GfskDemodulator::modulationName() const
{
    return "gfsk";
}


float
GfskDemodulator::fmDiscriminate(float i, float q)
{
    // Instantaneous frequency via conjugate multiply with previous sample:
    // freq = atan2(prev.conj * curr) / pi
    // Simplified: (prevI*q - prevQ*i) gives the imaginary part of the
    // conjugate product, which approximates the phase difference for small
    // angles.  We skip the atan2 for efficiency; for moderate frequency
    // deviations this approximation is adequate.
    float crossProduct = prevI_ * q - prevQ_ * i;
    float dotProduct = prevI_ * i + prevQ_ * q;
    float magnitude = std::sqrt(dotProduct * dotProduct + crossProduct * crossProduct);

    prevI_ = i;
    prevQ_ = q;

    if (magnitude < 1e-10f) {
        return 0.0f;
    }

    return crossProduct / magnitude;
}


void
GfskDemodulator::processSymbol(byte bit, vector<vector<byte>> & packets)
{
    switch (state_) {

    case State::HUNTING_PREAMBLE:
        // Preamble is 0xAA = 10101010, so alternating 1/0 bits
        bitBuffer_.push_back(bit);

        if (bitBuffer_.size() > 8) {
            bitBuffer_.pop_front();
        }

        if (bitBuffer_.size() == 8) {
            // Check if we have a preamble byte (0xAA = 10101010)
            byte assembled = 0;

            for (int b = 0; b < 8; ++b) {
                assembled = (assembled << 1) | bitBuffer_[b];
            }

            if (assembled == PREAMBLE_BYTE) {
                preambleCount_++;

                if (preambleCount_ >= PREAMBLE_LEN) {
                    state_ = State::HUNTING_SYNC;
                    bitBuffer_.clear();
                    bitsCollected_ = 0;
                }
            } else {
                preambleCount_ = 0;
            }
        }
        break;

    case State::HUNTING_SYNC:
        bitBuffer_.push_back(bit);
        bitsCollected_++;

        if (bitBuffer_.size() >= 16) {
            // Check last 16 bits for sync word 0xA1E1
            byte hi = 0;
            byte lo = 0;

            auto it = bitBuffer_.end() - 16;

            for (int b = 0; b < 8; ++b) {
                hi = (hi << 1) | *it++;
            }

            for (int b = 0; b < 8; ++b) {
                lo = (lo << 1) | *it++;
            }

            if (hi == SYNC_WORD[0] && lo == SYNC_WORD[1]) {
                state_ = State::READING_LENGTH;
                bitBuffer_.clear();
                bitsCollected_ = 0;
                frameLength_ = 0;
                break;
            }
        }

        // Timeout: if we don't find sync within 64 bits of preamble end
        if (bitsCollected_ > 64) {
            resetFrameState();
        }
        break;

    case State::READING_LENGTH:
        bitBuffer_.push_back(bit);
        bitsCollected_++;

        if (bitsCollected_ == 16) {
            // Assemble 16-bit length (big-endian)
            frameLength_ = 0;

            for (int b = 0; b < 16; ++b) {
                frameLength_ = (frameLength_ << 1) | bitBuffer_[b];
            }

            if (frameLength_ == 0 || frameLength_ > MAX_FRAME_PAYLOAD) {
                resetFrameState();
                break;
            }

            // Store the length bytes for CRC calculation
            frameBytes_.clear();
            frameBytes_.push_back(static_cast<byte>((frameLength_ >> 8) & 0xFF));
            frameBytes_.push_back(static_cast<byte>(frameLength_ & 0xFF));

            state_ = State::READING_PAYLOAD;
            bitBuffer_.clear();
            bitsCollected_ = 0;
        }
        break;

    case State::READING_PAYLOAD:
        bitBuffer_.push_back(bit);
        bitsCollected_++;

        if (bitsCollected_ % 8 == 0) {
            // Assemble byte from last 8 bits
            byte assembled = 0;
            auto it = bitBuffer_.end() - 8;

            for (int b = 0; b < 8; ++b) {
                assembled = (assembled << 1) | *it++;
            }

            frameBytes_.push_back(assembled);
        }

        if (bitsCollected_ == static_cast<uint>(frameLength_) * 8) {
            state_ = State::READING_CRC;
            bitBuffer_.clear();
            bitsCollected_ = 0;
        }
        break;

    case State::READING_CRC:
        bitBuffer_.push_back(bit);
        bitsCollected_++;

        if (bitsCollected_ == 16) {
            // Assemble CRC-16
            uint16_t receivedCrc = 0;

            for (int b = 0; b < 16; ++b) {
                receivedCrc = (receivedCrc << 1) | bitBuffer_[b];
            }

            // Compute expected CRC over length + payload
            uint16_t expectedCrc = crc16(frameBytes_.data(), static_cast<uint>(frameBytes_.size()));

            if (expectedCrc == receivedCrc) {
                // Valid packet — extract payload (skip length bytes)
                vector<byte> payload(frameBytes_.begin() + 2, frameBytes_.end());
                packets.push_back(std::move(payload));
            }

            resetFrameState();
        }
        break;
    }
}


void
GfskDemodulator::resetFrameState()
{
    state_ = State::HUNTING_PREAMBLE;
    preambleCount_ = 0;
    bitsCollected_ = 0;
    frameLength_ = 0;
    frameBytes_.clear();
    bitBuffer_.clear();
}
