/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <SdrDemodulator.h>

#include <cstring>


uint16_t
SdrDemodulator::crc16(const byte * data, uint length)
{
    uint16_t crc = 0xFFFF;

    for (uint i = 0; i < length; ++i) {
        crc ^= static_cast<uint16_t>(data[i]) << 8;

        for (int bit = 0; bit < 8; ++bit) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ 0x1021;
            } else {
                crc <<= 1;
            }
        }
    }

    return crc;
}


vector<byte>
SdrDemodulator::buildRadioFrame(const byte * payload, uint payloadLen)
{
    // [Preamble:4][Sync:2][Length:2][Payload:N][CRC16:2]
    uint frameLen = PREAMBLE_LEN + 2 + 2 + payloadLen + 2;
    vector<byte> frame(frameLen);
    uint off = 0;

    // Preamble
    for (uint i = 0; i < PREAMBLE_LEN; ++i) {
        frame[off++] = PREAMBLE_BYTE;
    }

    // Sync word
    frame[off++] = SYNC_WORD[0];
    frame[off++] = SYNC_WORD[1];

    // Length (big-endian)
    frame[off++] = static_cast<byte>((payloadLen >> 8) & 0xFF);
    frame[off++] = static_cast<byte>(payloadLen & 0xFF);

    // Payload
    memcpy(frame.data() + off, payload, payloadLen);
    off += payloadLen;

    // CRC-16 over length + payload
    uint16_t crc = crc16(frame.data() + PREAMBLE_LEN + 2, 2 + payloadLen);
    frame[off++] = static_cast<byte>((crc >> 8) & 0xFF);
    frame[off++] = static_cast<byte>(crc & 0xFF);

    return frame;
}


bool
SdrDemodulator::parseRadioFrame(const byte * frame, uint frameLen, vector<byte> & outPayload)
{
    // Minimum: preamble(4) + sync(2) + length(2) + crc(2) = 10
    if (frameLen < PREAMBLE_LEN + 2 + 2 + 2) {
        return false;
    }

    uint off = 0;

    // Verify preamble
    for (uint i = 0; i < PREAMBLE_LEN; ++i) {
        if (frame[off++] != PREAMBLE_BYTE) {
            return false;
        }
    }

    // Verify sync word
    if (frame[off] != SYNC_WORD[0] || frame[off + 1] != SYNC_WORD[1]) {
        return false;
    }
    off += 2;

    // Extract length (big-endian)
    uint payloadLen = (static_cast<uint>(frame[off]) << 8) | static_cast<uint>(frame[off + 1]);

    if (payloadLen > MAX_FRAME_PAYLOAD) {
        return false;
    }

    // Verify we have enough data: length(2) + payload + crc(2)
    if (off + 2 + payloadLen + 2 > frameLen) {
        return false;
    }

    // Verify CRC-16 over length + payload
    uint16_t expectedCrc = crc16(frame + off, 2 + payloadLen);
    uint crcOff = off + 2 + payloadLen;
    uint16_t receivedCrc = (static_cast<uint16_t>(frame[crcOff]) << 8) | static_cast<uint16_t>(frame[crcOff + 1]);

    if (expectedCrc != receivedCrc) {
        return false;
    }

    // Extract payload (skip length field)
    off += 2;
    outPayload.assign(frame + off, frame + off + payloadLen);

    return true;
}
