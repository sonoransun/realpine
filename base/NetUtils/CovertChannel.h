/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>


class CovertChannel
{
  public:

    static void initialize(const string& key);
    static bool isEnabled();
    static void obfuscate(byte* data, uint length);
    static void deobfuscate(byte* data, uint length);

  private:

    static bool enabled_s;
    static vector<byte> key_s;
    static uint32_t keySeed_s;

    static uint32_t xorshift32(uint32_t& state);
};
