/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#include <CovertChannel.h>

#include <cstring>


bool CovertChannel::enabled_s = false;
vector<byte> CovertChannel::key_s;
uint32_t CovertChannel::keySeed_s = 0;


uint32_t
CovertChannel::xorshift32(uint32_t & state)
{
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return state;
}


void
CovertChannel::initialize(const string & key)
{
    if (key.empty()) {
        return;
    }

    key_s.assign(key.begin(), key.end());

    // FNV-1a hash of key to produce seed
    uint32_t hash = 2166136261u;
    for (auto c : key) {
        hash ^= static_cast<uint32_t>(static_cast<unsigned char>(c));
        hash *= 16777619u;
    }
    keySeed_s = hash;

    enabled_s = true;
}


bool
CovertChannel::isEnabled()
{
    return enabled_s;
}


void
CovertChannel::obfuscate(byte * data, uint length)
{
    if (length == 0) {
        return;
    }

    // Stage 1: XOR cipher with rotating key
    uint keyLen = static_cast<uint>(key_s.size());
    for (uint i = 0; i < length; ++i) {
        data[i] ^= key_s[i % keyLen];
    }

    // Stage 2: Fisher-Yates shuffle seeded from key + length
    uint32_t state = keySeed_s + length;
    if (state == 0) {
        state = 1;
    }

    vector<byte> temp(data, data + length);
    for (uint i = length - 1; i > 0; --i) {
        uint32_t r = xorshift32(state);
        uint j = r % (i + 1);
        std::swap(temp[i], temp[j]);
    }
    memcpy(data, temp.data(), length);
}


void
CovertChannel::deobfuscate(byte * data, uint length)
{
    if (length == 0) {
        return;
    }

    // Stage 1: Compute the same shuffle sequence, then reverse it
    uint32_t state = keySeed_s + length;
    if (state == 0) {
        state = 1;
    }

    // Record the swaps so we can reverse them
    vector<pair<uint, uint>> swaps;
    swaps.reserve(length);
    for (uint i = length - 1; i > 0; --i) {
        uint32_t r = xorshift32(state);
        uint j = r % (i + 1);
        swaps.emplace_back(i, j);
    }

    // Apply swaps in reverse order to unshuffle
    for (auto it = swaps.rbegin(); it != swaps.rend(); ++it) {
        std::swap(data[it->first], data[it->second]);
    }

    // Stage 2: XOR cipher (same operation reverses itself)
    uint keyLen = static_cast<uint>(key_s.size());
    for (uint i = 0; i < length; ++i) {
        data[i] ^= key_s[i % keyLen];
    }
}
