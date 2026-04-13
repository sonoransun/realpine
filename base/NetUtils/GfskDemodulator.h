/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <SdrDemodulator.h>

#include <cstdint>
#include <deque>


/// GFSK (Gaussian Frequency Shift Keying) demodulator for digital radio
/// payload reception.
///
/// Performs FM discrimination, clock recovery (Mueller-Muller), symbol
/// slicing, preamble/sync detection, and packet extraction with CRC-16
/// validation.
///
/// Default parameters target 9600 baud with a deviation of +/- 4800 Hz,
/// which is a common configuration for embedded/IoT digital radio links.
///
class GfskDemodulator : public SdrDemodulator
{
  public:
    GfskDemodulator(uint baudRate = 9600);
    ~GfskDemodulator() override = default;

    bool initialize(uint sampleRate) override;

    bool demodulate(const int8_t * iqSamples, uint numSamples, vector<vector<byte>> & packets) override;

    string modulationName() const override;

  private:
    uint baudRate_;
    uint sampleRate_;
    float samplesPerSymbol_;
    bool initialized_;

    // FM discriminator state
    float prevI_;
    float prevQ_;

    // Clock recovery (Mueller-Muller) state
    float muPhase_;     // fractional symbol phase [0, 1)
    float prevSample_;  // previous interpolated sample
    float prevSymbol_;  // previous decided symbol

    // Bit assembly
    std::deque<byte> bitBuffer_;

    // Packet state machine
    enum class State { HUNTING_PREAMBLE, HUNTING_SYNC, READING_LENGTH, READING_PAYLOAD, READING_CRC };
    State state_;
    uint preambleCount_;
    uint bitsCollected_;
    uint16_t frameLength_;
    vector<byte> frameBytes_;

    float fmDiscriminate(float i, float q);
    void processSymbol(byte bit, vector<vector<byte>> & packets);
    void resetFrameState();
};
