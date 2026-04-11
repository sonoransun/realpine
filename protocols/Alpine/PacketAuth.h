/// Copyright (C) 2026 sonoransun — see LICENCE.txt


#pragma once
#include <Common.h>
#include <OptHash.h>
#include <ReadWriteSem.h>
#include <SecureString.h>
#include <unordered_map>


class PacketAuth
{
  public:
    static constexpr uint HMAC_SIZE = 32;  // SHA-256

    static void initialize();

    static bool isEnabled();
    static void setEnabled(bool enabled);

    // Per-peer shared secret management
    static bool setPeerSecret(ulong peerId, const byte * secret, uint secretLen);

    static bool removePeerSecret(ulong peerId);

    static bool hasPeerSecret(ulong peerId);

    // HMAC computation and verification
    static bool computeHmac(ulong peerId, const byte * data, uint dataLen, byte * hmacOut, uint hmacBufSize);

    static bool verifyHmac(ulong peerId, const byte * data, uint dataLen, const byte * hmac, uint hmacLen);

    // Key generation utility
    static bool generateSecret(byte * secretOut, uint secretBufSize);

  private:
    struct t_PeerSecret
    {
        SecureString secret;
    };

    static std::unordered_map<ulong, t_PeerSecret, OptHash<ulong>> peerSecrets_s;
    static ReadWriteSem secretsLock_s;
    static bool enabled_s;
};
