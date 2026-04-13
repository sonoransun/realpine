/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>

#include <cstdint>
#include <vector>


/// Abstract interface for SDR demodulators.
///
/// Implementations consume IQ sample streams and extract framed packets
/// using the Alpine radio-layer packet format:
///
///   [Preamble: 4x 0xAA] [Sync: 0xA1 0xE1] [Length: 2 BE] [Payload: N] [CRC16: 2]
///
class SdrDemodulator
{
  public:
    virtual ~SdrDemodulator() = default;

    virtual bool initialize(uint sampleRate) = 0;

    /// Feed IQ samples (interleaved I,Q int8 pairs) and extract any
    /// complete packets found.  Each inner vector is one complete payload
    /// (after sync word, length, and CRC have been stripped and validated).
    virtual bool demodulate(const int8_t * iqSamples, uint numSamples, vector<vector<byte>> & packets) = 0;

    virtual string modulationName() const = 0;


    // Radio-layer frame constants shared by all demodulators.
    //
    static constexpr byte PREAMBLE_BYTE = 0xAA;
    static constexpr uint PREAMBLE_LEN = 4;
    static constexpr byte SYNC_WORD[2] = {0xA1, 0xE1};
    static constexpr uint MAX_FRAME_PAYLOAD = 4096;

    /// Compute CRC-16/CCITT over a byte buffer.
    static uint16_t crc16(const byte * data, uint length);

    /// Build a complete radio-layer frame around a payload.
    static vector<byte> buildRadioFrame(const byte * payload, uint payloadLen);

    /// Parse a radio-layer frame.  Returns true if sync/length/CRC are
    /// valid, and writes the extracted payload to `outPayload`.
    static bool parseRadioFrame(const byte * frame, uint frameLen, vector<byte> & outPayload);
};
